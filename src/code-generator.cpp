#include "code-generator.hpp"
#include "include/utils.hpp"
#include "syntax-tree.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <ostream>
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
    m_var_count += get_count_vars(m_program.main);

    const size_t incr = m_var_count * 16;
    m_stack_sz = incr + 16;
    m_stack_ptr = incr;
    m_os << std::format("\tSUB sp, sp, {}\n", m_stack_sz);
    m_os << std::format("\tSTR x29, [sp, 0]\n");
    m_os << std::format("\tSTR x30, [sp, 8]\n");
    m_os << std::format("\tMOV x29, sp\n");
    m_os << std::format("\tMOV x28, x29\n"); // x28 aliased to global scope

    emit(m_program.main, m_os);
    m_os << m_fn_buf.str();
    m_os.close();
}

size_t CodeGenerator::get_count_vars(const NodeScope& node) {
    size_t var_cnt = 0;
    for (const auto& stmt : node.stmts){
        switch (stmt->get_node_type()) {
            case NodeType::VAR_DECL: {
                const auto& expr = std::get<NodeExpr>((std::get<NodeVarDeclaration>(stmt->m_node).value->m_node));
                var_cnt += expr.var_count;
                break;
            }
            case NodeType::STMT_EXIT: {
                if (const auto& expr = std::get_if<NodeExpr>( 
                    &std::get<NodeStmtExit>(stmt->m_node).exit_code->m_node 
                )) {
                    var_cnt += expr->var_count;
                }
                else if ( std::get_if<NodeIdentifier>( 
                    &std::get<NodeStmtExit>(stmt->m_node).exit_code->m_node 
                )) { var_cnt++; }
                break;
            }
            case NodeType::STMT_IF: {
                const auto&[cond, scp, n_elif, n_else, loc] = std::get<NodeStmtIf>(stmt->m_node);
                var_cnt += get_count_vars(scp);
                if (const auto& expr = std::get_if<NodeExpr>(&cond->m_node)) {
                    var_cnt += expr->var_count;
                }
                else if ( std::get_if<NodeIdentifier>(&cond->m_node)) { var_cnt++; }
                break;
            }
            case NodeType::STMT_ELIF: {
                const auto&[cond, scp, loc] = std::get<NodeStmtElif>(stmt->m_node);
                var_cnt += get_count_vars(scp);
                if (const auto& expr = std::get_if<NodeExpr>(&cond->m_node)) {
                    var_cnt += expr->var_count;
                }
                else if ( std::get_if<NodeIdentifier>(&cond->m_node)) { var_cnt++; }
                break;
            }
            case NodeType::STMT_WHILE: {
                const auto&[cond, scp, loc] = std::get<NodeStmtWhile>(stmt->m_node);
                var_cnt += get_count_vars(scp);
                if (const auto& expr = std::get_if<NodeExpr>(&cond->m_node)) {
                    var_cnt += expr->var_count;
                }
                else if ( std::get_if<NodeIdentifier>(&cond->m_node)) { var_cnt++; }
                break;
            }
            case NodeType::STMT_ELSE: {
                const auto& scp = ((std::get<NodeStmtElse>(stmt->m_node)).scope);
                var_cnt += get_count_vars(scp);
                break;
            }
            case NodeType::SCOPE_NODE: {
                var_cnt += get_count_vars(std::get<NodeScope>(stmt->m_node));
                break;
            }
            case NodeType::STMT_FN: {
                var_cnt += get_count_vars(std::get<NodeFunc>(stmt->m_node).scope);
                break;
            }
            case NodeType::CALL_NODE: {
                var_cnt += get_count_vars(
                    m_program.main.fn_table.at(
                        std::get<NodeCall>(stmt->m_node).ident.name
                    )->scope
                );
                var_cnt += std::get<NodeCall>(stmt->m_node).args.size();
                break;
            }
            default: assert(false && "Unknown node type!");
        }
    }
    return var_cnt;
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

std::string CodeGenerator::get_reg(const std::string& id) const {
    return m_program.main.var_table.contains(id) ? "x28" : "x29";
}

void CodeGenerator::emit_epilogue(std::ostream& buf) {
    buf << std::format("\tLDR x29, [sp, 0]\n");
    buf << std::format("\tLDR x30, [sp, 8]\n");
    buf << std::format("\tADD sp, sp, {}\n", m_stack_sz);
}
void CodeGenerator::emit_decl(const NodeVarDeclaration& node, std::ostream& buf) {
    const auto& [kind, ident, val, loc] = node;
    const auto& expr = std::get<NodeExpr>(val->m_node);
    NodeExpr e2send = expr.op == Op::EQ ? std::move(*expr.rhs) 
                                        : std::move(std::get<NodeExpr>(val->m_node));

    if (m_cached_var.contains(ident.name)) {
        const int prev_adr = m_cached_var.at(ident.name);
        const auto [res, temp] = emit_expr(e2send,
                                            CachedLoc{ prev_adr, get_reg(ident.name) },
                                            false, buf);
        if (prev_adr != res) {
            buf << std::format("\tLDR x8, [{}, {}]\n", get_reg(ident.name), res);
            buf << std::format("\tSTR x8, [{}, {}]\n", get_reg(ident.name), prev_adr);
        }
        return;
    }
    const auto [stack_loc, temp] = emit_expr(e2send, std::nullopt, true, buf);
    m_cached_var.insert_or_assign( ident.name, stack_loc ); // assignment always stores at perm adr
}

void CodeGenerator::emit_stmt_exit(const NodeStmtExit& node, std::ostream& buf) {
    //exitcode can ONLY be an INTEGER
    if (const auto n_int_lit = std::get_if<NodeIntLiteral>( &node.exit_code->m_node )) {
        buf << std::format("\tMOV x0, {}\n", n_int_lit->value);
    }

    int32_t stack_loc = 0, temp = 0;
    if (const auto n_ident = std::get_if<NodeIdentifier>( &node.exit_code->m_node )) {
        stack_loc = m_cached_var.at(n_ident->name);
        buf << std::format("\tLDR x0, [{}, {}]\n", get_reg(n_ident->name), stack_loc);
    }
    if (const auto n_expr = std::get_if<NodeExpr>( &node.exit_code->m_node )) {
        std::tie(stack_loc, temp) = emit_expr(*n_expr, std::nullopt, false, buf);

        if (stack_loc != temp) {
            const auto id = std::get_if<NodeIdentifier>( &n_expr->atom );
            if (id) {
                m_cached_var.insert_or_assign(id->name, stack_loc); // If variable, store unary result
            }
            buf << std::format("\tLDR x0, [{}, {}]\n", id ? get_reg(id->name) : "x29", temp);
        } else {
            buf << std::format("\tLDR x0, [x29, {}]\n", stack_loc); 
        }
    }

    buf << "\tMOV x16, 1\n";
    emit_epilogue(buf);
    buf << "\tBL  _exit\n";
}

void CodeGenerator::emit_stmt_if(const NodeStmtIf& node, const std::string& lbl_if,
    const std::string& lbl_else, const std::string& lbl_end, std::ostream& buf)
{
    const auto [stack_loc, temp] = emit_expr(std::get<NodeExpr>( node.cond->m_node ),
                                            std::nullopt, false, buf);
    if (stack_loc != temp) buf << std::format("\tLDR x8, [x29, {}]\n", temp); 
    else buf << std::format("\tLDR x8, [x29, {}]\n", stack_loc);

    buf << std::format("\tCMP x8, 0\n"); // anything nonzero is truthy
    buf << std::format("\tB.NE {}\n", lbl_if);
    buf << std::format("\tB {}\n", !node.n_elif.empty() ?
                                     std::format( "label_elif{}\n", m_lbl_count )
                                     : node.n_else.has_value() ?
                                     lbl_else
                                     : lbl_end);

    buf << lbl_if << ":\n";
    emit(node.scope, buf);
    buf << std::format("\tB {}\n", lbl_end);

    if (!node.n_elif.empty()) {
        buf << std::format("label_elif{}:\n", m_lbl_count);
        size_t lbl_count = m_lbl_count;

        // Elif
        for (auto it = node.n_elif.begin(); it != node.n_elif.end(); ++it) {
            const auto [stk_loc, temp] = emit_expr(std::get<NodeExpr>( it->cond->m_node ),
                                                std::nullopt, false, buf);

            if (stk_loc != temp) buf << std::format("\tLDR x8, [x29, {}]\n", temp);
            else buf << std::format("\tLDR x8, [x29, {}]\n", stk_loc);

            buf << std::format("\tCMP x8, 0\n"); // anything nonzero is truthy
            buf << std::format("\tB.EQ {}\n", std::next(it) != node.n_elif.end() ?
                                               std::format("label_elif{}\n", ++lbl_count)
                                               : node.n_else.has_value() ?
                                               lbl_else
                                               : lbl_end);

            emit(it->scope, buf);
            buf << std::format("\tB {}\n", lbl_end);
            if (std::next(it) != node.n_elif.end()) {
                buf << std::format("label_elif{}:\n", lbl_count);
            }
        }
    }

    // Else
    if (node.n_else.has_value()) {
        buf << std::format("{}:\n", lbl_else);
        emit(node.n_else.value().scope, buf);
    }
    buf << std::format("{}:\n", lbl_end);
}

void CodeGenerator::emit_stmt_while(const NodeStmtWhile& node, std::ostream& buf) {
    const std::string lbl_while = std::format("label_while{}", m_lbl_count++);
    buf << lbl_while << ":\n";

    if (const auto cond = std::get_if<NodeExpr>( &node.cond->m_node )) {
        const auto[stack_loc, temp] = emit_expr(*cond, std::nullopt, false, buf);
        if (stack_loc != temp) { buf << std::format("\tLDR x8, [x29, {}]\n", temp);
        } else { buf << std::format("\tLDR x8, [x29, {}]\n", stack_loc); }
        buf << "\tCMP x8, 0\n";
    }
    const std::string label_start = std::format("label{}", m_lbl_count++);
    const std::string lbl_end = std::format("label_end{}", m_lbl_count++);

    buf << std::format("\tB.NE {}\n", label_start);
    buf << std::format("\tB {}\n", lbl_end);
    buf << label_start << ":\n";

    emit(node.scope, buf);
    buf << std::format("\tB {}\n", lbl_while);
    buf << lbl_end << ":\n";
}

bool CodeGenerator::emit_op(NodeExpr& node, std::ostream& buf) {
    //NOTE: Must use register x8(lhs) and x9(rhs) if calling this function!
    Op op{};
    switch (node.op) {
        case Op::MUL:    { buf << "\tMUL x8, x8, x9\n";  return false; }
        case Op::DIV:    { buf << "\tSDIV x8, x8, x9\n"; return false; }
        case Op::BW_NOT: { buf << "\tMVN x8, x8\n";      return false; }
        case Op::BW_AND: { buf << "\tAND x8, x8, x9\n";  return false; }
        case Op::BW_OR:  { buf << "\tORR x8, x8, x9\n";  return false; }
        case Op::BW_XOR: { buf << "\tEOR x8, x8, x9\n";  return false; }
        case Op::LSL:    { buf << "\tLSL x8, x8, x9\n";  return false; }
        case Op::LSR:    { buf << "\tLSR x8, x8, x9\n";  return false; }
        case Op::INC:    {
            switch (node.fix) {
                case Fix::PREFIX: {
                    buf << "\tADD x8, x8, 1\n";
                    return false;
                }
                case Fix::POSTFIX: {
                    buf << "\tMOV x10, x8\n";
                    buf << "\tADD x10, x10, 1\n";
                    return true;
                }
            }
            break;
        }
        case Op::DEC:    {
            switch (node.fix) {
                case Fix::PREFIX: {
                    buf << "\tSUB x8, x8, 1\n";
                    return false;
                }
                case Fix::POSTFIX: {
                    buf << "\tMOV x10, x8\n";
                    buf << "\tSUB x10, x10, 1\n";
                    return true;
                }
            }
            break;
        }
        case Op::ADD:    {
            if (node.is_positive)
                { buf  << "\tMOV x8, x8\n"; return false; }
            buf << "\tADD x8, x8, x9\n";  return false;
            break;
        }
        case Op::SUB:    {
            if (node.is_negative)
                { buf  << "\tNEG x8, x8\n"; return false; }
            buf << "\tSUB x8, x8, x9\n";  return false;
            break;
        }
        case Op::MOD:  {
            buf << "\tSDIV x10, x8, x9\n";
            buf << "\tMSUB x8, x10, x9, x8\n";
            return false;
        }
        case Op::EQUIV:  {
            buf << "\tCMP x8, x9\n";
            buf << "\tCSET x8, EQ\n";
            return false;
        }
        case Op::NEQUIV: {
            buf << "\tCMP x8, x9\n";
            buf << "\tCSET x8, NE\n";
            return false;
        }
        case Op::LT: {
            buf << "\tCMP x8, x9\n";
            buf << "\tCSET x8, LT\n";
            return false;
        }
        case Op::GT: {
            buf << "\tCMP x8, x9\n";
            buf << "\tCSET x8, GT\n";
            return false;
        }
        case Op::LTE: {
            buf << "\tCMP x8, x9\n";
            buf << "\tCSET x8, LE\n";
            return false;
        }
        case Op::GTE: {
            buf << "\tCMP x8, x9\n";
            buf << "\tCSET x8, GE\n";
            return false;
        }
        case Op::LG_AND: {
            buf << "\tCMP  x8, 0\n";
            buf << "\tCSET x8, NE\n";
            buf << "\tCMP x9, 0\n";
            buf << "\tCSET x9, NE\n";
            buf << "\tAND x8, x8, x9\n";
            return false;
        }
        case Op::LG_OR: {
            buf << "\tCMP  x8, 0\n";
            buf << "\tCSET x8, NE\n";
            buf << "\tCMP x9, 0\n";
            buf << "\tCSET x9, NE\n";
            buf << "\tORR x8, x8, x9\n";
            return false;
        }
        case Op::PWR: {
            const std::string lb1 = std::format("label{}", m_lbl_count++);
            const std::string lb2 = std::format("label{}", m_lbl_count++);
            const std::string lb3 = std::format("label{}", m_lbl_count++);

            // check edge cases
            buf << "\tCMP x9, 1\n"; // power is 1
            buf << "\tB.EQ " << lb3 << "\n";
            buf << "\tCMP x9, 0\n"; // power is 0
            buf << "\tB.EQ " << lb2 << "\n";
            // else begin loop
            buf << "\tMOV x10, x9\n";
            buf << "\tMOV x9, x8\n";

            buf << lb1 << ":\n";
            buf << "\tMUL x8, x9, x8\n";
            buf << "\tSUB x10, x10, 1\n";
            buf << "\tCMP x10, 1\n";
            buf << "\tB.NE " << lb1 << "\n";
            buf << "\tB " << lb3 << "\n";

            buf << lb2 << ":\n";
            buf << "\tMOV x8, 1\n";
            buf << lb3 << ":\n";
            return false;
        }
        default: assert(false && "Error: Unknown op at codegen");
    }
}

void CodeGenerator::emit_store_literal(const int64_t val, std::ostream& buf) {
    //NOTE: this method always stores in register x8!
    const auto low = static_cast<uint16_t>(val & 0xFFFF);
    const auto low_med = static_cast<uint16_t>(val >> 16);
    const auto high_med = static_cast<uint16_t>(val >> 32);
    const auto high = static_cast<uint16_t>(val >> 48);

    buf << std::format("\tMOV x8, 0x{:02x}\n", low);
    if (low_med)  buf << std::format("\tMOVK x8, 0x{:02x}, LSL 16\n", low_med);
    if (high_med) buf << std::format("\tMOVK x8, 0x{:02x}, LSL 32\n", high_med);
    if (high)     buf << std::format("\tMOVK x8, 0x{:02x}, LSL 48\n", high);
                  buf << std::format("\tSTR x8, [x29, {}]\n", m_stack_ptr);
}

//NOTE: Returns a { permanent adr, temp adr } tuple
std::tuple<int32_t, int32_t> CodeGenerator::emit_expr(NodeExpr& node,
                const std::optional<CachedLoc>& cached_adr, const bool fresh_alloc,
                std::ostream& buf)
{
    const auto n_atom_id = std::get_if<NodeIdentifier>(&node.atom);
    const auto n_atom_lit = std::get_if<NodeIntLiteral>(&node.atom);

    if ( n_atom_id ) {
        const int cached_loc = m_cached_var.at(n_atom_id->name);

        /* Allocate new stack address for variables defined using existing variables;
         * Prevents erroneous overwrites */
        if (fresh_alloc) {
            buf << std::format("\tLDR x10, [{}, {}]\n", get_reg(n_atom_id->name), cached_loc);
            buf << std::format("\tSTR x10, [{}, {}]\n", get_reg(n_atom_id->name), m_stack_ptr);
            const int32_t storage_loc = m_stack_ptr;
            m_stack_ptr -= 8;
            return { storage_loc, storage_loc };
        }
        return { cached_loc, cached_loc };
    }

    if (n_atom_lit) {
        emit_store_literal(n_atom_lit->value, buf);
        const int32_t storage_loc = m_stack_ptr;
        m_stack_ptr -= 8;
        if (!node.lhs && !node.rhs) return { storage_loc, storage_loc };
    }

    int32_t lhs_stk_adr = -1, rhs_stk_adr = -1, temp_lhs = -1, temp_rhs = -1;
    if (node.lhs) std::tie( lhs_stk_adr, temp_lhs ) = emit_expr(*node.lhs, std::nullopt, false, buf);
    if (node.rhs) std::tie( rhs_stk_adr, temp_rhs ) = emit_expr(*node.rhs, std::nullopt, false, buf);
    if ( lhs_stk_adr == -1 && rhs_stk_adr == -1 ) return { -1, -1 };

    //==========================================================================
    if      (rhs_stk_adr == -1)       buf << std::format("\tLDR x8, [x29, {}]\n", lhs_stk_adr);
    else if (lhs_stk_adr == -1)       buf << std::format("\tLDR x8, [x29, {}]\n", rhs_stk_adr);
    else {
        buf << std::format("\tLDR x8, [x29, {}]\n", lhs_stk_adr);
        buf << std::format("\tLDR x9, [x29, {}]\n", rhs_stk_adr);
    }
    if (lhs_stk_adr != temp_lhs) buf << std::format("\tLDR x8, [x29, {}]\n", temp_lhs);
    if (rhs_stk_adr != temp_rhs) buf << std::format("\tLDR x9, [x29, {}]\n", temp_rhs);


    const std::string reg = cached_adr ? get_reg(cached_adr.value().reg) : "x29";
    const int32_t storage_loc = cached_adr ? cached_adr.value().adr : m_stack_ptr;
    const bool emit_post_op = emit_op(node, buf);
    buf << std::format("\tSTR x8, [{}, {}]\n", reg, storage_loc);

    if (!cached_adr) m_stack_ptr -= 8; // only move ptr if we're declaring a new variable

    if (emit_post_op) { // emit side effect (post increment)
        const int32_t storage_loc_perm = cached_adr ? cached_adr.value().adr : m_stack_ptr;
        buf << std::format("\tSTR x10, [{}, {}]\n", reg, storage_loc_perm);
        if (!cached_adr) m_stack_ptr -= 8;
        return {storage_loc_perm, storage_loc};
    }
    return {storage_loc, storage_loc};
    //==========================================================================
}

void CodeGenerator::emit_stmt_fn(const NodeFunc& node, std::ostream& buf) {
    const std::string fn_lbl = std::format("fn_{}:\n", node.ident.name);
    const size_t frame_sz = (get_count_vars(node.scope) +
                            node.args.size()) * 16 + 16;
    int32_t frame_ptr = frame_sz;

    /* Clear stack ptr and var table for func scope.
     * Prevents clashing between variable idents. */
    const int32_t cached_stk_ptr = m_stack_ptr;
    const auto cached_var_table = m_cached_var;
    m_cached_var.clear();

    m_fn_buf << fn_lbl;
    m_fn_buf << std::format("\tSUB sp, sp, {}\n", frame_sz);
    m_fn_buf << std::format("\tSTR x29, [sp, 0]\n");
    m_fn_buf << std::format("\tSTR x30, [sp, 8]\n");
    m_fn_buf << std::format("\tMOV x29, sp\n");
    m_fn_buf << "\tMOV x29, sp\n";

    for (size_t i = 0; i < node.args.size(); i++) {
        m_fn_buf << std::format("\tSTR x{}, [x29, {}]\n", i, frame_ptr);
        if (!m_cached_var.contains(node.args.at(i).name)) {
            m_cached_var.insert_or_assign(node.args.at(i).name, frame_ptr);
        }
        frame_ptr -= 8;
    }
    m_stack_ptr = frame_ptr;
    emit(node.scope, buf);

    m_fn_buf << std::format("\tLDR x29, [sp, 0]\n");
    m_fn_buf << std::format("\tLDR x30, [sp, 8]\n");
    m_fn_buf << std::format("\tADD sp, sp, {}\n", frame_sz);
    m_fn_buf << "\tRET\n";

    m_stack_ptr = cached_stk_ptr; m_cached_var = cached_var_table;
}

void CodeGenerator::emit_call_expr(const NodeCall& node, std::ostream& buf) {
    for (size_t i = 0; i < node.args.size(); i++) {
        const auto [stack_loc, temp] =
            emit_expr(
                std::get<NodeExpr>( node.args.at(i)->m_node )
                , std::nullopt, true, buf
            );
        // Load func args x0-x7
        buf << std::format("\tLDR x{}, [sp, {}]\n",
            i, stack_loc != temp ? temp : stack_loc);
    }
    buf << std::format("\tBL fn_{}\n", node.ident.name);
}

void CodeGenerator::emit(const NodeScope& node, std::ostream& buf) {
    for (const auto &stmt : node.stmts) {
        switch (stmt->get_node_type()) {
            case NodeType::VAR_DECL: {
                emit_decl(std::get<NodeVarDeclaration>(stmt->m_node), buf);
                break;
            }
            case NodeType::STMT_EXIT: {
                emit_stmt_exit(std::get<NodeStmtExit>(stmt->m_node), buf);
                break;
            }
            case NodeType::STMT_IF: {
                const std::string& lbl_if = std::format("label_if{}", m_lbl_count);
                const std::string& lbl_else = std::format("label_else{}", m_lbl_count);
                const std::string& lbl_end = std::format("label_end{}", m_lbl_count++);

                // elif and else handled inside
                emit_stmt_if(std::get<NodeStmtIf>(stmt->m_node),
                             lbl_if, lbl_else, lbl_end, buf);
                break;
            }
            case NodeType::STMT_WHILE: {
                emit_stmt_while(std::get<NodeStmtWhile>(stmt->m_node), buf);
                break;
            }
            case NodeType::SCOPE_NODE: {
                emit(std::get<NodeScope>(stmt->m_node), buf);
                break;
            }
            case NodeType::STMT_FN: {
                emit_stmt_fn(std::get<NodeFunc>(stmt->m_node), m_fn_buf);
                break;
            }
            case NodeType::CALL_NODE: {
                emit_call_expr(std::get<NodeCall>(stmt->m_node), buf);
                break;
            }
            default: assert(false && "Unknown node type!");
        }
    }
}
