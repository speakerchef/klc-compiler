#include "code-generator.hpp"
#include "include/utils.hpp"
#include "syntax-tree.hpp"
#include <cstdlib>
#include <print>
#include <mach/memory_object_types.h>

CodeGenerator::CodeGenerator(NodeProgram &&prog) noexcept
    : m_os("./gen_asm.s"), m_program(std::move(prog)) {
    if (!m_os.is_open()) {
        std::println(stderr, ERR_FILE);
        exit(EXIT_FAILURE);
    }
    m_os << ".global _main\n.align 4\n_main:\n";
    get_count_vars(m_program.main);
    std::println("Total num vars = {}", m_var_count);

    emit(m_program.main);
}
CodeGenerator::~CodeGenerator() = default;

void CodeGenerator::get_count_vars(const NodeScope& node) {
    for (const auto& stmt : node.stmts){
        switch (stmt.get_node_type()) {
        case NodeType::VAR_DECL: {
            if (const auto& expr = std::get_if<NodeBinaryExpr>( &std::get<NodeVarDeclaration>(stmt.m_node).value->m_node )) {
                m_var_count += expr->var_count;
            }
            break;
        }
        case NodeType::STMT_EXIT: {
            if (const auto& expr = std::get_if<NodeBinaryExpr>( &std::get<NodeStmtExit>(stmt.m_node).exit_code->m_node )) {
                m_var_count += expr->var_count;
            }
            else if ( std::get_if<NodeIdentifier>( &std::get<NodeStmtExit>(stmt.m_node).exit_code->m_node )) {
                m_var_count++;
            }
            break;
        }
        case NodeType::STMT_IF: {
            const auto&[cond, scp] = std::get<NodeStmtIf>(stmt.m_node);
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
            const auto&[cond, scp] = std::get<NodeStmtElif>(stmt.m_node);
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
            const auto& scp = ((std::get<NodeStmtElse>(stmt.m_node)).scope);
            get_count_vars(scp);
            break;
        }
        case NodeType::SCOPE_NODE: {
            get_count_vars(std::get<NodeScope>(stmt.m_node));
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
    return &m_program.main.stmts.at(m_node_ptr + offset);
}

const SyntaxNode* CodeGenerator::next() {
    if (m_program.main.stmts.empty() ||
       ( m_node_ptr ) >= m_program.main.stmts.size()) {
        return nullptr;
    }
    return &m_program.main.stmts.at( m_node_ptr++ );
}

void CodeGenerator::emit_epilogue() {
    m_os << std::format("\tADD sp, sp, {}\n", m_stack_sz);
}

void CodeGenerator::emit_decl(const NodeVarDeclaration& node) {
    const auto& [kind, ident, value, loc] = node;
    const auto& bin_expr = std::get<NodeBinaryExpr>(value->m_node); // can only be an expr at this moment
    // bin_expr.print();

    if ( m_expand_stack ) {
        size_t incr = m_var_count * 16;
        m_stack_sz = incr + 16;
        m_os << std::format("\tSUB sp, sp, {}\n", m_stack_sz);
        m_stack_ptr = incr;
        m_expand_stack = false;
    }

    int stack_loc = emit_expr(bin_expr);
    m_cached_var.insert({ ident.name, stack_loc });
}

void CodeGenerator::emit_stmt_exit(const NodeStmtExit& node) {
    //exitcode can ONLY be an INTEGER
    if (const auto n_int_lit = std::get_if<NodeIntLiteral>(&node.exit_code->m_node)) {
        m_os << std::format("\tMOV x0, #{}\n", n_int_lit->value);
        m_os << "\tMOV x16, #1\n";
        emit_epilogue();
        m_os << "\tBL  _exit\n";
    }
    if (const auto n_ident = std::get_if<NodeIdentifier>(&node.exit_code->m_node)) {
        const int32_t stack_loc = m_cached_var.at(n_ident->name);
        m_os << std::format("\tLDR x0, [sp, {}]\n", stack_loc);
        m_os << "\tMOV x16, 1\n";
        emit_epilogue();
        m_os << "\tBL  _exit\n";
    }
    if (const auto n_expr = std::get_if<NodeBinaryExpr>(&node.exit_code->m_node)) {
        const int64_t stack_pos = emit_expr(*n_expr);
        m_os << std::format("\tLDR x0, [sp, {}]\n", stack_pos);
        m_os << "\tMOV x16, 1\n";
        emit_epilogue();
        m_os << "\tBL  _exit\n";
    }

}

void CodeGenerator::emit_conditional(const std::variant<const NodeStmtIf*, const NodeStmtElif*> node){
    const auto stmt_if = std::get_if<const NodeStmtIf*>(&node);
    const auto stmt_elif = std::get_if<const NodeStmtElif*>(&node);

    int stack_loc = stmt_if ? emit_expr(std::get<NodeBinaryExpr>((*stmt_if)->cond->m_node))
                            : emit_expr(std::get<NodeBinaryExpr>((*stmt_elif)->cond->m_node));

    m_os << std::format("\tLDR x8, [sp, {}]\n", stack_loc); // for now only int values from expr are evaluated
    m_os << "\tCMP x8, 0\n"; // anything nonzero is true (janky bool)

    const std::string label_branch = std::format("label{}", m_lbl_count++);
    const std::string label_else = std::format("label{}", m_lbl_count++);
    m_os << std::format("\tB.NE {}\n", label_branch);
    m_os << std::format("\tB {}\n", label_else);
    m_os << label_branch << ":\n";

    stmt_if ? emit((*stmt_if)->scope) : emit((*stmt_elif)->scope);
    m_os << label_else << ":\n";
}

void CodeGenerator::emit_stmt_else(const NodeStmtElse& node) {
    emit(node.scope);
}

int32_t CodeGenerator::emit_expr(const NodeBinaryExpr& node) {
    // node.print();
    const auto n_atom_id = std::get_if<NodeIdentifier>(&node.atom);
    const auto n_atom_lit = std::get_if<NodeIntLiteral>(&node.atom);

    if (n_atom_id) {
        const int cached_loc = m_cached_var.at(n_atom_id->name);
        return cached_loc;
    }
    if (n_atom_lit) {
        m_os << "\tMOV x8, " << n_atom_lit->value << '\n';
        m_os << "\tSTR x8, [sp, " << m_stack_ptr << "]" << '\n';

        const int32_t storage_loc = m_stack_ptr;
        m_stack_ptr -= 8;
        if (!node.lhs && !node.rhs) return storage_loc;
    }


    int lhs_stk_adr = -1, rhs_stk_adr = -1;
    if (node.lhs) lhs_stk_adr = emit_expr(*node.lhs);
    if (node.rhs) rhs_stk_adr = emit_expr(*node.rhs);

    if (lhs_stk_adr == -1 || rhs_stk_adr == -1) {
        std::println("Bad stack address!");
        exit(EXIT_FAILURE);
    }

    //==========================================================================
    // m_stack_ptr += 8;
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
        m_os << "\tCMP x9, 1\n";
        m_os << "\tB.EQ " << lb3 << "\n";
        m_os << "\tCMP x9, 0\n";
        m_os << "\tB.EQ " << lb2 << "\n";
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
    m_os << std::format("\tSTR x8, [sp, {}]\n", m_stack_ptr);
    const int32_t storage_loc = m_stack_ptr;
    m_stack_ptr -= 8;
    return storage_loc;
    //==========================================================================
}

void CodeGenerator::emit(const NodeScope& node) {
#pragma clang diagnostic ignored "-Wswitch"
    for (const auto &stmt : node.stmts) {
        switch (stmt.get_node_type()) {
        case NodeType::VAR_DECL: {
            emit_decl(std::get<NodeVarDeclaration>(stmt.m_node));
            break;
        }
        case NodeType::STMT_EXIT: {
            emit_stmt_exit(std::get<NodeStmtExit>(stmt.m_node));
            break;
        }
        case NodeType::STMT_IF: {
            auto& stmt_if = std::get<NodeStmtIf>(stmt.m_node);
            emit_conditional(std::variant<const NodeStmtIf*, const NodeStmtElif*>(&stmt_if));
            break;
        }
        case NodeType::STMT_ELIF: {
            auto& stmt_elif = std::get<NodeStmtElif>(stmt.m_node);
            emit_conditional(std::variant<const NodeStmtIf*, const NodeStmtElif*>(&stmt_elif));
            break;
        }
        case NodeType::STMT_ELSE: {
            emit_stmt_else(std::get<NodeStmtElse>(stmt.m_node));
            break;
        }
        case NodeType::SCOPE_NODE: {
            emit(std::get<NodeScope>(stmt.m_node));
            break;
        }
        default: assert(false && "Unknown node type!");
        }
    }
}
