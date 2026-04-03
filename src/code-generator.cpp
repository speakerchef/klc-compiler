#include "code-generator.hpp"
#include "include/utils.hpp"
#include "syntax-tree.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <print>
#include <mach/memory_object_types.h>
#include <variant>

CodeGenerator::CodeGenerator(NodeProgram &&prog, const std::string& exec_name) noexcept
    : m_os(std::format("./{}.s", exec_name)), m_program(std::move(prog)) {

    if (!m_os.is_open()) {
        std::println(stderr, ERR_FILE);
        exit(EXIT_FAILURE);
    }
    m_os << ".global _main\n.align 4\n_main:\n";
    get_count_vars(m_program.main);
    emit(m_program.main);
    m_os.close();
}

void CodeGenerator::get_count_vars(const NodeScope& node) {
    for (const auto& stmt : node.stmts){
        switch (stmt->get_node_type()) {
        case NodeType::VAR_DECL: {
            const auto& expr = std::get<NodeExpr>((std::get<NodeVarDeclaration>(stmt->m_node).value->m_node));
            m_var_count += expr.var_count;
            break;
        }
        case NodeType::STMT_EXIT: {
            if (const auto& expr = std::get_if<NodeExpr>( &std::get<NodeStmtExit>(stmt->m_node).exit_code->m_node )) {
                m_var_count += expr->var_count;
            }
            else if ( std::get_if<NodeIdentifier>( &std::get<NodeStmtExit>(stmt->m_node).exit_code->m_node )) { m_var_count++; }
            break;
        }
        case NodeType::STMT_IF: {
            const auto&[cond, scp, n_elif, n_else, loc] = std::get<NodeStmtIf>(stmt->m_node);
            get_count_vars(scp);
            if (const auto& expr = std::get_if<NodeExpr>(&cond->m_node)) {
                m_var_count += expr->var_count;
            }
            else if ( std::get_if<NodeIdentifier>(&cond->m_node)) { m_var_count++; }
            break;
        }
        case NodeType::STMT_ELIF: {
            const auto&[cond, scp, loc] = std::get<NodeStmtElif>(stmt->m_node);
            get_count_vars(scp);
            if (const auto& expr = std::get_if<NodeExpr>(&cond->m_node)) {
                m_var_count += expr->var_count;
            }
            else if ( std::get_if<NodeIdentifier>(&cond->m_node)) { m_var_count++; }
            break;
        }
        case NodeType::STMT_WHILE: {
            const auto&[cond, scp, loc] = std::get<NodeStmtWhile>(stmt->m_node);
            get_count_vars(scp);
            if (const auto& expr = std::get_if<NodeExpr>(&cond->m_node)) {
                m_var_count += expr->var_count;
            }
            else if ( std::get_if<NodeIdentifier>(&cond->m_node)) { m_var_count++; }
            break;
        }
        case NodeType::STMT_ELSE: {
            const auto& scp = ((std::get<NodeStmtElse>(stmt->m_node)).scope);
            get_count_vars(scp);
            break;
        }
        case NodeType::SCOPE_NODE: {
            get_count_vars(std::get<NodeScope>(stmt->m_node));
            break;
        }
        default: assert(false && "Unknown node type!");
        }
    }
}

const SyntaxNode* CodeGenerator::peek(const size_t offset = 0) const {
    if (m_program.main.stmts.empty() ||
        ( m_node_ptr + offset ) >= m_program.main.stmts.size()) {
        return nullptr;
    }
    return m_program.main.stmts.at(m_node_ptr + offset).get();
}

const SyntaxNode* CodeGenerator::next() {
    if (m_program.main.stmts.empty() ||
       ( m_node_ptr ) >= m_program.main.stmts.size()) {
        return nullptr;
    }
    return m_program.main.stmts.at( m_node_ptr++ ).get();
}

void CodeGenerator::emit_epilogue() { m_os << std::format("\tADD sp, sp, {}\n", m_stack_sz); }
void CodeGenerator::emit_decl(const NodeVarDeclaration& node) {
    const auto& [kind, ident, val, loc] = node;
    const auto& expr = std::get<NodeExpr>(val->m_node);
    NodeExpr e2send = expr.op == Op::EQ ? std::move(*expr.rhs) 
                                        : std::move(std::get<NodeExpr>(val->m_node));

    if ( m_expand_stack ) {
        size_t incr = m_var_count * 16;
        m_stack_sz = incr + 16;
        m_os << std::format("\tSUB sp, sp, {}\n", m_stack_sz);
        m_stack_ptr = incr;
        m_expand_stack = false;
    }

    if (m_cached_var.contains(ident.name)) {
        const int prev_adr = m_cached_var.at(ident.name);
        const auto [res, temp] = emit_expr(e2send, &prev_adr, false);
        if (prev_adr != res) {
            m_os << std::format("\tLDR x10, [sp, {}]\n", res);
            m_os << std::format("\tSTR x10, [sp, {}]\n", prev_adr);
        }
        return;
    }
    const auto [stack_loc, temp] = emit_expr(e2send, nullptr, true);
    std::println("STACK LOC AT DECL: {}, TEMP AT DECL: {}", stack_loc, temp);
    m_cached_var.insert_or_assign( ident.name, stack_loc ); // assignment always stores at perm adr
}

void CodeGenerator::emit_stmt_exit(const NodeStmtExit& node) {
    //exitcode can ONLY be an INTEGER
    if (const auto n_int_lit = std::get_if<NodeIntLiteral>( &node.exit_code->m_node )) {
        m_os << std::format("\tMOV x0, #{}\n", n_int_lit->value);
    }

    int32_t stack_loc = 0, temp = 0;
    if (const auto n_ident = std::get_if<NodeIdentifier>( &node.exit_code->m_node )) {
        stack_loc = m_cached_var.at(n_ident->name);
        m_os << std::format("\tLDR x0, [sp, {}]\n", stack_loc);
    }
    if (const auto n_expr = std::get_if<NodeExpr>( &node.exit_code->m_node )) {
        std::tie(stack_loc, temp) = emit_expr(*n_expr, nullptr, false);

        if (stack_loc != temp) {
            if (const auto id = std::get_if<NodeIdentifier>( &n_expr->atom )) {
                m_cached_var.insert_or_assign(id->name, stack_loc); // If variable, store unary result
            }
            m_os << std::format("\tLDR x0, [sp, {}]\n", temp); 
        } else {
            std::println("TEMP: {}, PERM: {}", temp, stack_loc);
            m_os << std::format("\tLDR x0, [sp, {}]\n", stack_loc); 
        }
    }

    m_os << "\tMOV x16, 1\n";
    emit_epilogue();
    m_os << "\tBL  _exit\n";
}

void CodeGenerator::emit_stmt_if(const NodeStmtIf& node, const std::string& lbl_if,
    const std::string& lbl_else, const std::string& lbl_end) 
{
    const auto [stack_loc, temp] = emit_expr(std::get<NodeExpr>( node.cond->m_node ),
                                            nullptr, false);
    if (stack_loc != temp) m_os << std::format("\tLDR x8, [sp, {}]\n", temp); 
    else m_os << std::format("\tLDR x8, [sp, {}]\n", stack_loc);

    m_os << std::format("\tCMP x8, 0\n"); // anything nonzero is truthy
    m_os << std::format("\tB.NE {}\n", lbl_if);
    m_os << std::format("\tB {}\n", !node.n_elif.empty() ?
                                     std::format( "label_elif{}\n", m_lbl_count )
                                     : node.n_else.has_value() ?
                                     lbl_else
                                     : lbl_end);

    m_os << lbl_if << ":\n";
    emit(node.scope);
    m_os << std::format("\tB {}\n", lbl_end);

    if (!node.n_elif.empty()) {
        m_os << std::format("label_elif{}:\n", m_lbl_count);
        size_t lbl_count = m_lbl_count;

        // Elif
        for (auto it = node.n_elif.begin(); it != node.n_elif.end(); ++it) {
            const auto [stk_loc, temp] = emit_expr(std::get<NodeExpr>( it->cond->m_node ),
                                                nullptr, false);

            if (stk_loc != temp) m_os << std::format("\tLDR x8, [sp, {}]\n", temp);
            else m_os << std::format("\tLDR x8, [sp, {}]\n", stk_loc);

            m_os << std::format("\tCMP x8, 0\n"); // anything nonzero is truthy
            m_os << std::format("\tB.EQ {}\n", std::next(it) != node.n_elif.end() ?
                                               std::format("label_elif{}\n", ++lbl_count)
                                               : node.n_else.has_value() ?
                                               lbl_else
                                               : lbl_end);

            emit(it->scope);
            m_os << std::format("\tB {}\n", lbl_end);
            if (std::next(it) != node.n_elif.end()) {
                m_os << std::format("label_elif{}:\n", lbl_count);
            }
        }
    }

    // Else
    if (node.n_else.has_value()) {
        m_os << std::format("{}:\n", lbl_else);
        emit(node.n_else.value().scope);
    }
    m_os << std::format("{}:\n", lbl_end);
}

void CodeGenerator::emit_stmt_while(const NodeStmtWhile& node) {
    const std::string lbl_while = std::format("label_while{}", m_lbl_count++);
    m_os << lbl_while << ":\n";

    if (const auto cond = std::get_if<NodeExpr>( &node.cond->m_node )) {
        const auto[stack_loc, temp] = emit_expr(*cond, nullptr, false);
        if (stack_loc != temp) {
            m_os << std::format("\tLDR x8, [sp, {}]\n", temp); 
        } else {
            m_os << std::format("\tLDR x8, [sp, {}]\n", stack_loc); 
        }
        m_os << "\tCMP x8, 0\n";
    }
    const std::string label_start = std::format("label{}", m_lbl_count++);
    const std::string lbl_end = std::format("label_end{}", m_lbl_count++);

    m_os << std::format("\tB.NE {}\n", label_start);
    m_os << std::format("\tB {}\n", lbl_end);
    m_os << label_start << ":\n";

    emit(node.scope);
    m_os << std::format("\tB {}\n", lbl_while);
    m_os << lbl_end << ":\n";
}

bool CodeGenerator::emit_op(NodeExpr& node) {
    //NOTE: Must use register x8(lhs) and x9(rhs) if calling this function!
    Op op{};
    switch (node.op) {
        case Op::ADD:    { m_os << "\tADD x8, x8, x9\n";  return false; }
        case Op::SUB:    { m_os << "\tSUB x8, x8, x9\n";  return false; }
        case Op::MUL:    { m_os << "\tMUL x8, x8, x9\n";  return false; }
        case Op::DIV:    { m_os << "\tSDIV x8, x8, x9\n"; return false; }
        case Op::BW_NOT: { m_os << "\tMVN x8, x8\n";      return false; }
        case Op::BW_AND: { m_os << "\tAND x8, x8, x9\n";  return false; }
        case Op::BW_OR:  { m_os << "\tORR x8, x8, x9\n";  return false; }
        case Op::BW_XOR: { m_os << "\tEOR x8, x8, x9\n";  return false; }
        case Op::LSL:    { m_os << "\tLSL x8, x8, x9\n";  return false; }
        case Op::LSR:    { m_os << "\tLSR x8, x8, x9\n";  return false; }
        case Op::INC:    {
            switch (node.fix) {
                case Fix::PREFIX: {
                    m_os << "\tADD x8, x8, 1\n";
                    return false;
                }
                case Fix::POSTFIX: {
                    m_os << "\tMOV x10, x8\n";
                    m_os << "\tADD x10, x10, 1\n";
                    return true;
                }
            }
            break;
        }
        case Op::DEC:    {
            switch (node.fix) {
                case Fix::PREFIX: {
                    m_os << "\tSUB x8, x8, 1\n";
                    return false;
                }
                case Fix::POSTFIX: {
                    m_os << "\tMOV x10, x8\n";
                    m_os << "\tSUB x10, x10, 1\n";
                    return true;
                }
            }
            break;
        }
        case Op::MOD:  {
            m_os << "\tSDIV x10, x8, x9\n";
            m_os << "\tMSUB x8, x10, x9, x8\n";
            return false;
        }
        case Op::EQUIV:  {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, EQ\n";
            return false;
        }
        case Op::NEQUIV: {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, NE\n";
            return false;
        }
        case Op::LT: {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, LT\n";
            return false;
        }
        case Op::GT: {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, GT\n";
            return false;
        }
        case Op::LTE: {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, LE\n";
            return false;
        }
        case Op::GTE: {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, GE\n";
            return false;
        }
        case Op::LG_AND: {
            m_os << "\tCMP  x8, 0\n";
            m_os << "\tCSET x8, NE\n";
            m_os << "\tCMP x9, 0\n";
            m_os << "\tCSET x9, NE\n";
            m_os << "\tAND x8, x8, x9\n";
            return false;
        }
        case Op::LG_OR: {
            m_os << "\tCMP  x8, 0\n";
            m_os << "\tCSET x8, NE\n";
            m_os << "\tCMP x9, 0\n";
            m_os << "\tCSET x9, NE\n";
            m_os << "\tORR x8, x8, x9\n";
            return false;
        }
        case Op::PWR: {
            const std::string lb1 = std::format("label{}", m_lbl_count++);
            const std::string lb2 = std::format("label{}", m_lbl_count++);
            const std::string lb3 = std::format("label{}", m_lbl_count++);

            // check edge cases
            m_os << "\tCMP x9, 1\n"; // power is 1
            m_os << "\tB.EQ " << lb3 << "\n";
            m_os << "\tCMP x9, 0\n"; // power is 0
            m_os << "\tB.EQ " << lb2 << "\n";
            // else begin loop
            m_os << "\tMOV x10, x9\n";
            m_os << "\tMOV x9, x8\n";

            m_os << lb1 << ":\n";
            m_os << "\tMUL x8, x9, x8\n";
            m_os << "\tSUB x10, x10, 1\n";
            m_os << "\tCMP x10, 1\n";
            m_os << "\tB.NE " << lb1 << "\n";
            m_os << "\tB " << lb3 << "\n";

            m_os << lb2 << ":\n";
            m_os << "\tMOV x8, 1\n";
            m_os << lb3 << ":\n";
            return false;
        }
        default: assert(false && "Error: Unknown op at codegen");
    }
}

void CodeGenerator::emit_store_literal(const int64_t val) {
    //NOTE: this method always stores in register x8!
    const auto low = static_cast<uint16_t>(val & 0xFFFF);
    const auto low_med = static_cast<uint16_t>(val >> 16);
    const auto high_med = static_cast<uint16_t>(val >> 32);
    const auto high = static_cast<uint16_t>(val >> 48);

    m_os << std::format("\tMOV x8, #0x{:02x}\n", low);
    if (low_med) m_os << std::format("\tMOVK x8, #0x{:02x}, LSL 16\n", low_med);
    if (high_med) m_os << std::format("\tMOVK x8, #0x{:02x}, LSL 32\n", high_med);
    if (high) m_os << std::format("\tMOVK x8, #0x{:02x}, LSL 48\n", high);
    m_os << std::format("\tSTR x8, [sp, #{}]\n", m_stack_ptr);
}

//NOTE: Returns a { permanent adr, temp adr } tuple
std::tuple<int32_t, int32_t> CodeGenerator::emit_expr(NodeExpr& node, const int32_t *cached_adr, const bool fresh_alloc) {
    const auto n_atom_id = std::get_if<NodeIdentifier>(&node.atom);
    const auto n_atom_lit = std::get_if<NodeIntLiteral>(&node.atom);

    if ( n_atom_id ) {
        const int cached_loc = m_cached_var.at(n_atom_id->name);

        /* Allocate new stack address for variables defined using existing variables;
         * Prevents erroneous overwrites */
        if (fresh_alloc) {
            m_os << std::format("\tLDR x10, [sp, {}]\n", cached_loc);
            m_os << std::format("\tSTR x10, [sp, {}]\n", m_stack_ptr);
            const int32_t storage_loc = m_stack_ptr;
            m_stack_ptr -= 8;
            return { storage_loc, storage_loc };
        }
        return { cached_loc, cached_loc };
    }

    if (n_atom_lit) {
        emit_store_literal(n_atom_lit->value);
        const int32_t storage_loc = m_stack_ptr;
        m_stack_ptr -= 8;
        if (!node.lhs && !node.rhs) return { storage_loc, storage_loc };
    }

    int32_t lhs_stk_adr = -1, rhs_stk_adr = -1, temp_lhs = -1, temp_rhs = -1;
    if (node.lhs) std::tie( lhs_stk_adr, temp_lhs ) = emit_expr(*node.lhs, nullptr, false);
    if (node.rhs) std::tie( rhs_stk_adr, temp_rhs ) = emit_expr(*node.rhs, nullptr, false);
    if ( lhs_stk_adr == -1 && rhs_stk_adr == -1 ) return { -1, -1 };

    //==========================================================================
    if (rhs_stk_adr == -1)       m_os << std::format("\tLDR x8, [sp, {}]\n", lhs_stk_adr);
    else if (lhs_stk_adr == -1)       m_os << std::format("\tLDR x8, [sp, {}]\n", rhs_stk_adr);
    else {
        m_os << std::format("\tLDR x8, [sp, {}]\n", lhs_stk_adr);
        m_os << std::format("\tLDR x9, [sp, {}]\n", rhs_stk_adr);
    }
    if (lhs_stk_adr != temp_lhs) m_os << std::format("\tLDR x8, [sp, {}]\n", temp_lhs);
    if (rhs_stk_adr != temp_rhs) m_os << std::format("\tLDR x9, [sp, {}]\n", temp_rhs);

    const bool emit_post_op = emit_op(node);
    m_os << std::format("\tSTR x8, [sp, {}]\n", cached_adr ? *cached_adr : m_stack_ptr);
    const int32_t storage_loc =                 cached_adr ? *cached_adr : m_stack_ptr;
    if (!cached_adr) m_stack_ptr -= 8; // only move ptr if we're declaring a new variable

    if (emit_post_op) {
        m_os << std::format("\tSTR x10, [sp, {}]\n", cached_adr ? *cached_adr : m_stack_ptr);
        const int32_t storage_loc_perm =             cached_adr ? *cached_adr : m_stack_ptr;
        if (!cached_adr) m_stack_ptr -= 8;
        return {storage_loc_perm, storage_loc};
    }
    return {storage_loc, storage_loc};
    //==========================================================================
}

void CodeGenerator::emit(const NodeScope& node) {
    for (const auto &stmt : node.stmts) {
        switch (stmt->get_node_type()) {
            case NodeType::VAR_DECL: {
                emit_decl(std::get<NodeVarDeclaration>(stmt->m_node));
                break;
            }
            case NodeType::STMT_EXIT: {
                emit_stmt_exit(std::get<NodeStmtExit>(stmt->m_node));
                break;
            }
            case NodeType::STMT_IF: {
                const std::string& lbl_if = std::format("label_if{}", m_lbl_count);
                const std::string& lbl_else = std::format("label_else{}", m_lbl_count);
                const std::string& lbl_end = std::format("label_end{}", m_lbl_count++);

                // elif and else handled inside
                emit_stmt_if(std::get<NodeStmtIf>(stmt->m_node), lbl_if, lbl_else, lbl_end);
                break;
            }
            case NodeType::STMT_WHILE: {
                emit_stmt_while(std::get<NodeStmtWhile>(stmt->m_node));
                break;
            }
            case NodeType::SCOPE_NODE: {
                emit(std::get<NodeScope>(stmt->m_node));
                break;
            }
            default: assert(false && "Unknown node type!");
        }
    }
}
