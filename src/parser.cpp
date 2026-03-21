#include "parser.hpp"
#include "tokenizer.hpp"
#include "include/utils.hpp"
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <vector>


// TODO: Make some sort of AST 
// TODO: Helper function to check token validity for kw/op
// TODO: Make queue for code generator from syntax tree

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
    bool require_semi = false;

    while (peek(0).has_value()){
        const Token peeked = peek(0).value();

        // If semi not seen and no more tokens, we fail
        if (peeked.type != TokenType::DELIM_SEMI &&
            !peek(1).has_value()) {
            std::println("Error: Missing `;`");
            exit(EXIT_FAILURE);
        }
        
        switch (peeked.type) {
            case TokenType::DELIM_SEMI: {
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
                break;
            }
            case TokenType::KW_EXIT: {
                if (!peek(1).has_value() || 
                    peek(1).value().type != TokenType::LIT_INT) {
                    std::println("Error: Required integer literal code after `exit`");
                    exit(EXIT_FAILURE);
                } else {
                    if (!peek(2).has_value()) {
                        std::println("Error: Missing `;`");
                        exit(EXIT_FAILURE);
                    }
                    int ecode = 0;
                    auto exit_code_v = [&](auto &&ec) {
                        if constexpr (std::is_convertible_v<decltype(ec), int>){
                            ecode = ec;
                        }
                    };
                    std::visit(exit_code_v, peek(1).value().value); // consumes both exit and int tokens

                    take(0);
                    m_osref << "\tMOV x0, " << ecode << "\n";
                    m_osref << "\tMOV x16, 1\n";
                    m_osref << "\tBL _exit  \n";
                    require_semi = true;
                }
                break;
            } 
            case TokenType::KW_RETURN: {
                take(0);
                break;
            } 
            case TokenType::LIT_INT: {
                take(0);
                break; 
            }
            case TokenType::LIT_STR:  {
                take(0);
                break; 
            }
            default: {
                std::print(stderr, "Error: Unknown Symbol "); print_variant(peeked.value);
                exit(EXIT_FAILURE);
            }
        }
    }


}

