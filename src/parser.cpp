#include "parser.hpp"
#include "tokenizer.hpp"
#include "include/utils.hpp"
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <vector>

Parser::Parser(std::vector<Token> toks, std::ofstream &osref) : 
    m_tokens(std::move(toks)), 
    m_osref(osref)
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

std::optional<Token> Parser::take(size_t offset) {
    if (m_tokens.empty() || 
        (m_token_ptr + offset) >= m_tokens.size()) {
        return std::nullopt;
    }

    auto res = m_tokens.at(m_token_ptr + offset);
    m_token_ptr += offset + 1;
    return res;
}

void Parser::parse_tokens() {

    while (peek(0).has_value()){
        // TODO: Fix problem with semicolon detection
        bool require_semi = false;
        const Token peeked = peek(0).value();

        // std::println("Token Pointer: {}, Buffer size: {}", m_token_ptr, m_tokens.size());

        // Semicolon check
        if (peeked.type == TokenType::DELIM_SEMI) {
            if (!peek(1).has_value() && require_semi) { 
                take(0);
                require_semi = false;
                break; 
            }
            else if (peek(1).has_value() && require_semi){
                take(0);
                require_semi = false;
            }
            else if (peek(1).has_value() && !require_semi) { 
                take(0);
                continue; 
            }
            else if (!peek(1).has_value() && !require_semi) {
                take(0);
                continue; 
            }
        } 
        // If semi not seen and no more tokens, we fail
        else if (!peek(1).has_value() && require_semi) {
            std::println("Error: Missing `;`");
            exit(EXIT_FAILURE);
        }

        if (peeked.type == TokenType::KW_EXIT) {
            if (!peek(1).has_value() || 
                peek(1).value().type != TokenType::LIT_INT) {
            // if (peek(1).value().type != TokenType::LIT_INT) {
                std::println("Error: Required integer literal code after `exit`");
                exit(EXIT_FAILURE);
            } else {
                int ecode = 0;
                auto exit_code_v = [&](auto &&ec) {
                    if constexpr (std::is_convertible_v<decltype(ec), int>){
                        ecode = ec;
                    }
                };
                std::visit(exit_code_v, take(1).value().value); // consumes both exit and int tokens

                m_osref << "\nMOV x0, " << ecode << "\nMOV x16, 1";
                require_semi = true;
            }
        } else {
            std::print(stderr, "Error: Unknown Symbol ");
            print_variant(peeked.value);
            std::println("Type: {}", peeked.type == TokenType::DELIM_SEMI);
            exit(EXIT_FAILURE);
        }


    }


}

