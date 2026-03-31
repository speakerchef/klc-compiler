#include "code-generator.hpp"
#include "include/utils.hpp"
#include "syntax-tree.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <print>
#include <mach/memory_object_types.h>

CodeGenerator::CodeGenerator(NodeProgram &&prog, const std::string& exec_name) noexcept
    : m_os(std::format("./{}.s", exec_name)), m_program(std::move(prog)) {
    if (!m_os.is_open()) {
        std::println(stderr, ERR_FILE);
        exit(EXIT_FAILURE);
    }
    m_os << ".global _main\n.align 4\n_main:\n";
    get_count_vars(m_program.main);
    // std::println("Total num vars = {}", m_var_count);
    emit(m_program.main);
    m_os.close();
}

void CodeGenerator::get_count_vars(const NodeScope& node) {
    for (const auto& stmt : node.stmts){
        switch (stmt->get_node_type()) {
        case NodeType::VAR_DECL: {
            if (const auto& expr = std::get_if<NodeBinaryExpr>( &std::get<NodeVarDeclaration>(stmt->m_node).value->m_node )) {
                m_var_count += expr->var_count;
            }
            break;
        }
        case NodeType::STMT_EXIT: {
            if (const auto& expr = std::get_if<NodeBinaryExpr>( &std::get<NodeStmtExit>(stmt->m_node).exit_code->m_node )) {
                m_var_count += expr->var_count;
            }
            else if ( std::get_if<NodeIdentifier>( &std::get<NodeStmtExit>(stmt->m_node).exit_code->m_node )) {
                m_var_count++;
            }
            break;
        }
        case NodeType::STMT_IF: {
            const auto&[cond, scp, loc] = std::get<NodeStmtIf>(stmt->m_node);
            get_count_vars(scp);
            if (const auto& expr = std::get_if<NodeBinaryExpr>(&cond->m_node)) {
                m_var_count += expr->var_count;
            }
            else if ( std::get_if<NodeIdentifier>(&cond->m_node)) {
                m_var_count++;
            }
            break;
        }
        case NodeType::STMT_ELIF: {
            const auto&[cond, scp, loc] = std::get<NodeStmtElif>(stmt->m_node);
            get_count_vars(scp);
            if (const auto& expr = std::get_if<NodeBinaryExpr>(&cond->m_node)) {
                m_var_count += expr->var_count;
            }
            else if ( std::get_if<NodeIdentifier>(&cond->m_node)) {
                m_var_count++;
            }
            break;
        }
        case NodeType::STMT_WHILE: {
            const auto&[cond, scp, loc] = std::get<NodeStmtWhile>(stmt->m_node);
            get_count_vars(scp);
            if (const auto& expr = std::get_if<NodeBinaryExpr>(&cond->m_node)) {
                m_var_count += expr->var_count;
            }
            else if ( std::get_if<NodeIdentifier>(&cond->m_node)) {
                m_var_count++;
            }
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

void CodeGenerator::emit_epilogue() {
    m_os << std::format("\tADD sp, sp, {}\n", m_stack_sz);
}

void CodeGenerator::emit_decl(const NodeVarDeclaration& node) {
    const auto& [kind, ident, value, loc] = node;
    const auto& bin_expr = std::get<NodeBinaryExpr>(value->m_node); // can only be an expr at this moment

    if ( m_expand_stack ) {
        size_t incr = m_var_count * 16;
        m_stack_sz = incr + 16;
        m_os << std::format("\tSUB sp, sp, {}\n", m_stack_sz);
        m_stack_ptr = incr;
        m_expand_stack = false;
    }

    if (m_cached_var.contains(ident.name)) {
        const int prev_adr = m_cached_var.at(ident.name);
        const int res = emit_expr(bin_expr, &prev_adr, false);
        if (prev_adr != res) {
            m_os << std::format("\tLDR x10, [sp, {}]\n", res);
            m_os << std::format("\tSTR x10, [sp, {}]\n", prev_adr);
        }
        return;
    }
    int stack_loc = emit_expr(bin_expr, nullptr, true);
    m_cached_var.insert_or_assign( ident.name, stack_loc );
}

void CodeGenerator::emit_stmt_exit(const NodeStmtExit& node) {
    //exitcode can ONLY be an INTEGER
    if (const auto n_int_lit = std::get_if<NodeIntLiteral>(&node.exit_code->m_node)) {
        m_os << std::format("\tMOV x0, #{}\n", n_int_lit->value);
    }

    int32_t stack_pos = 0;
    if (const auto n_ident = std::get_if<NodeIdentifier>(&node.exit_code->m_node)) {
        stack_pos = m_cached_var.at(n_ident->name);
        m_os << std::format("\tLDR x0, [sp, {}]\n", stack_pos);
    }
    if (const auto n_expr = std::get_if<NodeBinaryExpr>(&node.exit_code->m_node)) {
        stack_pos = emit_expr(*n_expr, nullptr, false);
        m_os << std::format("\tLDR x0, [sp, {}]\n", stack_pos);
    }

    m_os << "\tMOV x16, 1\n";
    emit_epilogue();
    m_os << "\tBL  _exit\n";
}

void CodeGenerator::emit_conditional(const std::variant<const NodeStmtIf*, const NodeStmtElif*> node,
                                     const std::string& lbl_if, const std::string& lbl_else, const std::string& lbl_elif,
                                     const std::string& lbl_chain_end) {

    const auto stmt_if = std::get_if<const NodeStmtIf*>(&node);
    const auto stmt_elif = std::get_if<const NodeStmtElif*>(&node);

    const int32_t stack_loc = stmt_if ? emit_expr(std::get<NodeBinaryExpr>((*stmt_if)->cond->m_node), nullptr  , false)
                            : emit_expr(std::get<NodeBinaryExpr>((*stmt_elif)->cond->m_node), nullptr  , false);

    m_os << std::format("\tLDR x8, [sp, {}]\n", stack_loc); // for now only int values from expr are evaluated
    m_os << "\tCMP x8, 0\n"; // anything nonzero is true (janky bool)
    
    m_os << std::format("\tB.NE {}\n", lbl_if);

    if (stmt_if) {
        m_os << std::format("\tB {}\n", lbl_else);
        m_os << lbl_if << ":\n";
        emit((*stmt_if)->scope); 
    }

    if (stmt_elif) {
        m_os << std::format("\tB {}\n", lbl_elif);
        m_os << lbl_if << ":\n";
        emit((*stmt_elif)->scope); 
    }
    m_os << std::format("\tB {}\n", lbl_chain_end);
}

void CodeGenerator::emit_stmt_else(const NodeStmtElse& node) {
    emit(node.scope);
}

void CodeGenerator::emit_stmt_while(const NodeStmtWhile& node) {
    const std::string lbl_while = std::format("label{}while", m_lbl_count++);
    m_os << lbl_while << ":\n";

    if (const auto cond = std::get_if<NodeBinaryExpr>(&node.cond->m_node)) {
        const int stack_loc = emit_expr(*cond, nullptr, false);
        m_os << std::format("\tLDR x8, [sp, {}]\n", stack_loc);
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

int32_t CodeGenerator::emit_expr(const NodeBinaryExpr& node, const int32_t *cached_adr, const bool fresh_alloc) {
    const auto n_atom_id = std::get_if<NodeIdentifier>(&node.atom);
    const auto n_atom_lit = std::get_if<NodeIntLiteral>(&node.atom);

    if ( n_atom_id ) {
        const int cached_loc = m_cached_var.at(n_atom_id->name);

        if (fresh_alloc) {
            m_os << std::format("\tLDR x10, [sp, {}]\n", cached_loc);
            m_os << std::format("\tSTR x10, [sp, {}]\n", m_stack_ptr);
            const int32_t storage_loc = m_stack_ptr;
            m_stack_ptr -= 8;
            return storage_loc;
        }
        return cached_loc;
    }

    if (n_atom_lit) {
        if (n_atom_lit->value > UINT16_MAX - 1){
            const auto low = static_cast<uint16_t>(n_atom_lit->value & 0xFFFF);
            const auto high = static_cast<uint16_t>(n_atom_lit->value >> 16);

            m_os << std::format("\tMOVZ x8, 0x{:02x}\n", low);
            m_os << std::format("\tMOVK x8, 0x{:02x}, LSL 16\n", high);
            m_os << std::format("\tSTR x8, [sp, {}]\n", m_stack_ptr);
        }
        else {
            m_os << std::format("\tMOV x8, {}\n", n_atom_lit->value);
            m_os << std::format("\tSTR x8, [sp, {}]\n", m_stack_ptr);
        }

        const int32_t storage_loc = m_stack_ptr;
        m_stack_ptr -= 8;

        if (!node.lhs && !node.rhs) return storage_loc;
    }

    int lhs_stk_adr = -1, rhs_stk_adr = -1;
    if (node.lhs) lhs_stk_adr = emit_expr(*node.lhs, nullptr, false);
    if (node.rhs) rhs_stk_adr = emit_expr(*node.rhs, nullptr, false);

    if (lhs_stk_adr == -1 || rhs_stk_adr == -1) {
        std::println(stderr, "Bad stack address!");
        exit(EXIT_FAILURE);
    }

    //==========================================================================

    m_os << std::format("\tLDR x8, [sp, {}]\n", lhs_stk_adr);
    m_os << std::format("\tLDR x9, [sp, {}]\n", rhs_stk_adr);

    switch (node.op) {
        case BinOp::ADD:    { m_os << "\tADD x8, x8, x9\n";  break; }
        case BinOp::SUB:    { m_os << "\tSUB x8, x8, x9\n";  break; }
        case BinOp::MUL:    { m_os << "\tMUL x8, x8, x9\n";  break; }
        case BinOp::DIV:    { m_os << "\tSDIV x8, x8, x9\n"; break; }
        case BinOp::BW_AND: { m_os << "\tAND x8, x8, x9\n"; break; }
        case BinOp::BW_OR:  { m_os << "\tORR x8, x8, x9\n"; break; }
        case BinOp::BW_XOR: { m_os << "\tEOR x8, x8, x9\n"; break; }
        case BinOp::LSL:    { m_os << "\tLSL x8, x8, x9\n"; break; }
        case BinOp::LSR:    { m_os << "\tLSR x8, x8, x9\n"; break; }
        case BinOp::MOD:  {
            m_os << "\tSDIV x10, x8, x9\n";
            m_os << "\tMSUB x8, x10, x9, x8\n";
            break;
        }
        case BinOp::EQUIV:  {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, EQ\n";
            break;
        }
        case BinOp::NEQUIV: {
                m_os << "\tCMP x8, x9\n";
                m_os << "\tCSET x8, NE\n";
                break;
        }
        case BinOp::LT: {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, LT\n";
            break;
        }
        case BinOp::GT: {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, GT\n";
            break;
        }
        case BinOp::LTE: {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, LE\n";
            break;
        }
        case BinOp::GTE: {
            m_os << "\tCMP x8, x9\n";
            m_os << "\tCSET x8, GE\n";
            break;
        }
        case BinOp::LG_AND: {
            m_os << "\tCMP  x8, 0\n";
            m_os << "\tCSET x8, NE\n";
            m_os << "\tCMP x9, 0\n";
            m_os << "\tCSET x9, NE\n";
            m_os << "\tAND x8, x8, x9\n";
            break;
        }
        case BinOp::LG_OR: {
            m_os << "\tCMP  x8, 0\n";
            m_os << "\tCSET x8, NE\n";
            m_os << "\tCMP x9, 0\n";
            m_os << "\tCSET x9, NE\n";
            m_os << "\tORR x8, x8, x9\n";
            break;
        }
        case BinOp::PWR: {
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

            break;
        }
        default: assert(false && "Unknown operator!");
    }

    m_os << std::format("\tSTR x8, [sp, {}]\n", cached_adr ? *cached_adr : m_stack_ptr);

    const int32_t storage_loc =                 cached_adr ? *cached_adr : m_stack_ptr;

    if (!cached_adr) m_stack_ptr -= 8; // only move ptr if we're declaring a new variable
    return storage_loc;

    //==========================================================================
}

void CodeGenerator::emit(const NodeScope& node) {
#pragma clang diagnostic ignored "-Wswitch"

    for (auto it = node.stmts.begin(); it < node.stmts.end(); ++it) {
        switch (it.base()->get()->get_node_type()) {
            case NodeType::VAR_DECL: {
                emit_decl(std::get<NodeVarDeclaration>(it.base()->get()->m_node));
                break;
            }
            case NodeType::STMT_EXIT: {
                emit_stmt_exit(std::get<NodeStmtExit>(it.base()->get()->m_node));
                break;
            }
            case NodeType::STMT_IF: {
                auto& stmt_if = std::get<NodeStmtIf>(it.base()->get()->m_node);
                const std::string lbl_if = std::format("label_if{}", m_lbl_count++);
                const std::string lbl_else = std::format("label_else{}", m_lbl_count++);
                const std::string lbl_end = std::format("label_end{}", m_lbl_count++);

                emit_conditional(std::variant<const NodeStmtIf*, const NodeStmtElif*>(&stmt_if),
                                lbl_if, 
                                (++it).base()->get()->get_node_type() ==
                                NodeType::STMT_ELIF ? std::format("label_elif{}", m_lbl_count)
                                                    : lbl_else,
                                "", lbl_end);

                while ((it).base()->get() && it.base()->get()->get_node_type() == NodeType::STMT_ELIF) {
                    auto& stmt_elif = std::get<NodeStmtElif>(it.base()->get()->m_node);

                    m_os << std::format("label_elif{}", m_lbl_count++) << ":\n";

                    emit_conditional(std::variant<const NodeStmtIf*, 
                                    const NodeStmtElif*>(&stmt_elif), 
                                    std::format("label_branch{}", m_lbl_count++), "", 
                                    (++it).base()->get()->get_node_type() != 
                                    NodeType::STMT_ELIF ? lbl_else 
                                                        : std::format("label_elif{}", m_lbl_count), lbl_end);
                }

                if ((it).base()->get() && it.base()->get()->get_node_type() == NodeType::STMT_ELSE) {
                    m_os << lbl_else << ":\n";

                    emit_stmt_else(std::get<NodeStmtElse>(it.base()->get()->m_node));
                    m_os << std::format("\tB {}\n", lbl_end);

                    m_os << lbl_end << ":\n";
                    break;
                }

                m_os << lbl_else << ":\n";
                m_os << std::format("\tB {}\n", lbl_end);
                m_os << lbl_end << ":\n";

                --it; // reset ptr location
                break;
            }
            case NodeType::STMT_ELIF: {
                const auto&[cond, scp, loc] = std::get<NodeStmtElif>(it.base()->get()->m_node);
                std::println(stderr, "[{}:{}]Error: Expected accompanying `if` statement for `elif`.",
                    loc.line, loc.col);
                exit(EXIT_FAILURE);
            }
            case NodeType::STMT_ELSE: {
                const auto&[scope, loc] = std::get<NodeStmtElse>(it.base()->get()->m_node);
                std::println(stderr, "[{}:{}]Error: Expected accompanying `if` statement for `else`.",
                    loc.line, loc.col);
                exit(EXIT_FAILURE);
            }
            case NodeType::STMT_WHILE: {
                const std::string lbl_while = std::format("label{}while", m_lbl_count++);
                const std::string lbl_end = std::format("label_end{}", m_lbl_count++);
                emit_stmt_while(std::get<NodeStmtWhile>(it.base()->get()->m_node));
                break;
            }
            case NodeType::SCOPE_NODE: {
                emit(std::get<NodeScope>(it.base()->get()->m_node));
                break;
            }
            default: assert(false && "Unknown node type!");
        }
    }
}
