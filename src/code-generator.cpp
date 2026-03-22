#include "code-generator.hpp"
#include "tokenizer.hpp"

CodeGenerator::CodeGenerator(std::ofstream &os, std::vector<std::pair<TokenType, std::string>> cstack, SyntaxTree stree) 
: m_os(std::move(os)),
 m_call_stack(std::move(cstack)),
 m_stree(std::move(stree))
{}
CodeGenerator::~CodeGenerator() 
{
    // Epilogue (kinda)
    m_os << "\tADD sp, sp, " << m_stack_sz << '\n';
}

void CodeGenerator::emit() {
    for (const auto &call : m_call_stack) {
        if (call.first == TokenType::KW_EXIT) {
            const int exit_code = std::get<NodeExit>(m_stree
                                                     .get_node(call.second)
                                                     .value()
                                                     .get_node_value(TokenType::KW_EXIT)
                                                     ).exit_code;
            m_os << "\tMOV x0, " << exit_code << '\n';
            m_os << "\tMOV x16, 1\n";
            m_os << "\tBL _exit  \n";
        }
        else if(call.first == TokenType::VAR_INT) {
            const auto var_int = std::get<NodeIntVar>(m_stree.get_node(call.second)
                                                      .value()
                                                      .get_node_value(TokenType::VAR_INT));
            m_os << "\tSUB sp, sp, 16\n";
            m_os << "\tMOV x20, " << var_int.value;
            m_os << " // ident: " << var_int.ident << '\n'; // dbg comment
            m_os << "\tSTR x20, [sp]\n";

            m_stack_sz += 16;
        }
    }
}
