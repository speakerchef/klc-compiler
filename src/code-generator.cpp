#include "code-generator.hpp"
#include "tokenizer.hpp"

CodeGenerator::CodeGenerator(std::ofstream &os) : m_os(std::move(os)) {}
CodeGenerator::~CodeGenerator() 
{
    // Epilogue (kinda)
    m_os << "\tADD sp, sp, " << m_stack_sz << '\n';
}

void CodeGenerator::emit(const SyntaxNode &node) {
    switch (node.get_node_type()) {
        case TokenType::KW_EXIT: {
            const auto exit_node = std::get<NodeExit>(node.get_node_value(TokenType::KW_EXIT));
            m_os << "\tMOV x0, " << exit_node.exit_code << '\n';
            m_os << "\tMOV x16, 1\n";
            m_os << "\tBL _exit  \n";
            break;
        }
        case TokenType::VAR_INT: {
            const auto var_int = std::get<NodeIntVar>(node.get_node_value(TokenType::VAR_INT));
            m_os << "\tSUB sp, sp, 16\n";
            m_os << "\tMOV x20, " << var_int.value;
            m_os << " // ident: " << var_int.ident << '\n'; // dbg comment
            m_os << "\tSTR x20, [sp]\n";

            m_stack_sz += 16;
            break;
        }
        default: {
            break;
        }
    }
}
