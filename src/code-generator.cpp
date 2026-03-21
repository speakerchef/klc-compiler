#include "code-generator.hpp"

CodeGenerator::CodeGenerator(std::ofstream &os) : m_os(std::move(os)) {}

void CodeGenerator::emit(const SyntaxNode &node) {
    switch (node.get_node_type()) {
        case TokenType::KW_EXIT: {
            m_os << "\tMOV x0, " << node.get_node_value() << "\n";
            m_os << "\tMOV x16, 1\n";
            m_os << "\tBL _exit  \n";
            break;
        }
        default: {
            break;
        }
    }
}
