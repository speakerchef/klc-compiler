#include "code-generator.hpp"
#include "include/utils.hpp"
#include "syntax-tree.hpp"
#include <cctype>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <variant>

using std::get;

CodeGenerator::CodeGenerator(NodeProgram &&prog) noexcept
    : m_os("./gen_asm.s"), m_program(std::move(prog)) {
    if (!m_os.is_open()) {
        std::println(stderr, ERR_FILE);
        exit(EXIT_FAILURE);
    }
    m_os << ".global _start\n.align 4\n_start:\n";
}
CodeGenerator::~CodeGenerator() {
    // Epilogue (kinda)
    m_os << "\tADD sp, sp, " << m_stack_sz << '\n';
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


// void CodeGenerator::emit() {
//     for (const auto &call : m_called_nodes) {
//         if (call.get_node_type() == TokenType::KW_EXIT) {
//             const int exit_code =
//                 std::get<ExprExit>(call .get_node_value()).exit_code;
//             m_os << "\tMOV x0, " << exit_code << '\n';
//             m_os << "\tMOV x16, 1\n";
//             m_os << "\tBL _exit  \n";
//         }
//         else if(call.get_node_type() == TokenType::VAR_INT) {
//             const auto var_int =
//             std::get<ExprIntVariable>(call.get_node_value()); m_os << "\tSUB
//             sp, sp, 16\n"; m_os << "\tMOV x20, " << var_int.value; m_os << "
//             // ident: " << var_int.ident << '\n'; // dbg comment m_os <<
//             "\tSTR x20, [sp]\n";
//
//             m_stack_sz += 16;
//         }
//     }
// }
