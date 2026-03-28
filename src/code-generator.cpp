#include "code-generator.hpp"
#include "include/utils.hpp"
#include "syntax-tree.hpp"
#include <cstdlib>
#include <print>

CodeGenerator::CodeGenerator(NodeProgram &&prog) noexcept
    : m_os("./gen_asm.s"), m_program(std::move(prog)) {
    if (!m_os.is_open()) {
        std::println(stderr, ERR_FILE);
        exit(EXIT_FAILURE);
    }
    m_os << ".global _main\n.align 4\n_main:\n";
}
CodeGenerator::~CodeGenerator() {
    // Epilogue (kinda)
    m_os << "\tADD sp, sp, " << m_stack_sz << '\n';
    // m_os << "\tLDP x29, x30, [sp], " << m_stack_sz << '\n';
}

const SyntaxNode* CodeGenerator::peek(const size_t offset = 0) const {
    if (m_program.main.empty() ||
        ( m_node_ptr + offset ) >= m_program.main.size()) {
        return nullptr;
    }
    return &m_program.main.at(m_node_ptr + offset);
}

const SyntaxNode* CodeGenerator::next() {
    if (m_program.main.empty() ||
       ( m_node_ptr ) >= m_program.main.size()) {
        return nullptr;
    }
    return &m_program.main.at( m_node_ptr++ );
}

void CodeGenerator::emit_epilogue() {
    m_os << std::format("\tADD sp, sp, {}\n", m_stack_sz);
}

int CodeGenerator::consteval_expr(const NodeBinaryExpr& node) { // use only for compile time eval
    auto lres = 0;
    auto rres = 0;
    const auto n_atom_id { std::get_if<NodeIdentifier>(&node.atom) };
    const auto n_atom_lit { std::get_if<NodeIntLiteral>(&node.atom) };
    // node.print();

    // if (!std::isdigit(node.atom.front()) && !node.atom.empty()) {
    if (n_atom_id) {
        // std::println("IDENT AT EVAL EXPR: {}", node.atom);
        const auto & id_node = std::get<NodeBinaryExpr>(m_program.lookup_node(n_atom_id->name)->m_node);
        const auto lhs = id_node.lhs.get();
        const auto rhs = id_node.rhs.get();

        // id_node.print();
        if (lhs) lres = consteval_expr(*lhs);
        if (rhs) rres = consteval_expr(*rhs);

        switch (id_node.op) {
            case BinOp::ADD: {
                // node.atom = std::to_string(lres + rres);
                return (lres + rres);
            }
            case BinOp::SUB: {
                // node.atom = (lres - rres);
                return (lres - rres);
            }
            case BinOp::MULT: {
                // node.atom = (lres * rres);
                return (lres * rres);
            }
            case BinOp::DIV: {
                // node.atom = (lres / rres);
                return (lres / rres);
            }
            default: {
                assert(false && "Unknown operator!");
            }
        }
    }
    if (!node.lhs && !node.rhs) return n_atom_lit->value;

    if (node.lhs) lres = consteval_expr(*node.lhs);
    if (node.rhs) rres = consteval_expr(*node.rhs);
    
    switch (node.op) {
        case BinOp::ADD:  return lres + rres;
        case BinOp::SUB:  return lres - rres;
        case BinOp::MULT: return lres * rres;
        case BinOp::DIV:  return lres / rres;
        default: assert(false && "Unknown operator!");
    }
}

void CodeGenerator::emit_decl(const NodeVarDeclaration& node) {
    const auto& [kind, ident, value, loc] = node;
    const auto& bin_expr = std::get<NodeBinaryExpr>(value->m_node); // can only be an expr at this moment
    // std::println("Num vars: {}", bin_expr.var_count);
    // bin_expr.print();

    if ( expand_stack ) {
        size_t incr = bin_expr.var_count > 2 ? 16 * bin_expr.var_count : 16;
        m_stack_sz += incr;
        m_os << std::format("\tSUB sp, sp, {}\n", incr);
        m_stack_ptr += m_stack_sz;
        expand_stack = false;
    }

    int stack_loc = emit_expr(bin_expr);
    // std::println("STACK LOC:{} for ID: {}", stack_loc, ident.name);
    m_cached_var.insert({ ident.name, stack_loc + 8 });
    // m_cached_var.insert({ id, stack_loc  });
}

void CodeGenerator::emit_stmt_exit(const NodeStmtExit& node) {
    //exitcode can ONLY be an INTEGER
    const auto n_expr = std::get_if<NodeBinaryExpr>(&node.exit_code->m_node);
    const auto n_int_lit = std::get_if<NodeIntLiteral>(&node.exit_code->m_node);
    if (n_int_lit) {
        m_os << std::format("\tMOV x0, #{}\n", n_int_lit->value);
        m_os << "\tMOV x16, #1\n";
        emit_epilogue();
        m_os << "\tBL  _exit\n";
    }
    if (n_expr) {
        const int64_t stack_pos = emit_expr(*n_expr) + 8;
        m_os << std::format("\tLDR x0, [sp, {}]\n", stack_pos);
        m_os << "\tMOV x16, #1\n";
        emit_epilogue();
        m_os << "\tBL  _exit\n";
    }
}

int32_t CodeGenerator::emit_expr(const NodeBinaryExpr& node) {
    // node.print();
    const auto n_atom_id = std::get_if<NodeIdentifier>(&node.atom);
    const auto n_atom_lit = std::get_if<NodeIntLiteral>(&node.atom);

    if (n_atom_id) {
        int cached_loc = m_cached_var.at(n_atom_id->name) - 8;
        return cached_loc;
    }
    if (n_atom_lit) {
        m_os << "\tMOV x8, " << n_atom_lit->value << '\n';
        m_os << "\tSTR x8, [sp, " << m_stack_ptr << "]" << '\n';
        m_stack_ptr -= 8;
        if (!node.lhs && !node.rhs) return m_stack_ptr;
    }


    // TODO: Rigorously test logic and robustness of curr solution
    int lhs_stk_adr = -1, rhs_stk_adr = -1;
    if (node.lhs) lhs_stk_adr = emit_expr(*node.lhs) + 8; //adr of a = 16
    if (node.rhs) rhs_stk_adr = emit_expr(*node.rhs) + 8; //adr of 3 = 8

    if (lhs_stk_adr == -1 || rhs_stk_adr == -1) {
        std::println("Bad stack address!");
        exit(EXIT_FAILURE);
    }

    switch (node.op) {
        case BinOp::ADD: {
            m_stack_ptr += 8;
            m_os << std::format("\tLDR x8, [sp, {}]\n", lhs_stk_adr);
            m_os << std::format("\tLDR x9, [sp, {}]\n", rhs_stk_adr);
            m_os << "\tADD x8, x8, x9\n";
            m_os << std::format("\tSTR x8, [sp, {}]\n", m_stack_ptr);
            m_stack_ptr -= 8;
            return m_stack_ptr;
        }
        case BinOp::SUB:
        case BinOp::MULT:
        case BinOp::DIV:
        default: assert(false && "Unknown operator!");
    }
}


void CodeGenerator::emit() {
    for (const auto &call : m_program.main) {
        switch (call.get_node_type()) {
            case NodeType::STMT_EXIT: {
                emit_stmt_exit(std::get<NodeStmtExit>(call.m_node));
                break;
            }
            case NodeType::VAR_DECL: {
                emit_decl(std::get<NodeVarDeclaration>(call.m_node));
                break;
            }
        }
    }
}
