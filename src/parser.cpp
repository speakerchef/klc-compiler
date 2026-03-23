#include "parser.hpp"
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <utility>
#include <vector>

// TODO: Make some sort of AST 
// TODO: Helper function to check token validity for kw/op

Parser::Parser(std::vector<Token> toks) : 
    m_tokens(std::move(toks))
{
    parse_tokens();
};
Parser::~Parser()
{
}

std::optional<Token> Parser::peek(size_t offset) const {
    if (m_tokens.empty() || 
        (m_token_ptr + offset) >= m_tokens.size()) {
        return std::nullopt;
    }
    return m_tokens.at(m_token_ptr + offset);
}

std::optional<Token> Parser::consume() {
    if (m_tokens.empty() || 
        (m_token_ptr) >= m_tokens.size()) {
        return std::nullopt;
    }
    return m_tokens.at(m_token_ptr++);
}

void Parser::parse_tokens() {
}
