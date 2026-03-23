#include "code-generator.hpp"
#include "syntax-tree.hpp"
#include "tokenizer.hpp"
#include <unordered_map>

CodeGenerator::CodeGenerator(std::ofstream &&os, std::vector<SyntaxNode> &&cnodes, std::unordered_map<std::string, SyntaxNode> &&var_table)
: m_os(std::move(os)),
 m_called_nodes(std::move(cnodes)),
 m_var_table(std::move(var_table))
{}
CodeGenerator::~CodeGenerator() 
{
    // Epilogue (kinda)
    m_os << "\tADD sp, sp, " << m_stack_sz << '\n';
}

void CodeGenerator::emit() {
    for (const auto &call : m_called_nodes) {
        if (call.get_node_type() == TokenType::KW_EXIT) {
            const int exit_code = 
                std::get<ExprExit>(call .get_node_value()).exit_code;
            m_os << "\tMOV x0, " << exit_code << '\n';
            m_os << "\tMOV x16, 1\n";
            m_os << "\tBL _exit  \n";
        }
        else if(call.get_node_type() == TokenType::VAR_INT) {
            const auto var_int = std::get<ExprIntVariable>(call.get_node_value());
            m_os << "\tSUB sp, sp, 16\n";
            m_os << "\tMOV x20, " << var_int.value;
            m_os << " // ident: " << var_int.ident << '\n'; // dbg comment
            m_os << "\tSTR x20, [sp]\n";

            m_stack_sz += 16;
        }
    }
}
