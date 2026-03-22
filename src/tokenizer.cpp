#include "tokenizer.hpp"
#include "syntax-tree.hpp"
#include "code-generator.hpp"
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <print>
#include <string>
#include <vector>

/* */
Tokenizer::Tokenizer(std::ofstream os, std::ifstream is)
    : 
    m_ifs(std::move(is)),
    m_ofs(std::move(os))
{
    tokenize();
}
Tokenizer::~Tokenizer() 
{
}

/* */
std::optional<Token> Tokenizer::peek(size_t offset) const {
    if (m_tokens.empty() || 
        (m_token_ptr + offset) >= m_tokens.size()) {
        return std::nullopt;
    }
    
    return m_tokens.at(m_token_ptr + offset);
}

/* */
std::optional<Token> Tokenizer::consume() {
    if (m_tokens.empty() || 
        (m_token_ptr) >= m_tokens.size()) {
        return std::nullopt;
    }

    return m_tokens.at(m_token_ptr++);
}

/* */
void Tokenizer::tokenize() {
    bool require_semi = false;
    char ch;
    std::string buf;
    SyntaxTree ast{};

    // Build tokens list
    while (m_ifs.get(ch)) {
        if (std::isalnum(ch)) {
            buf.push_back(ch);
        } 
        else if (std::isspace(ch)) {
            if (!buf.empty()) {
                m_tokens.emplace_back(this->classify_token(buf));
                buf.clear();
            }
        } 
        else if (std::ispunct(ch)){ // Operators & Symbols
            if (!buf.empty()) {
                m_tokens.emplace_back(this->classify_token(buf));
                buf.clear();
            }
            std::string o{ch};
            m_tokens.emplace_back(this->classify_token(o));
        }
    }
    auto visitor = [](const auto &value) { std::println("Token value: {}", value); };
    for (const Token &tok : m_tokens) {
        std::visit(visitor, tok.value);
    }

    // Build syntax tree
    while (peek(0).has_value()) {
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
                    consume();
                    require_semi = false;
                    break; 
                }
                else if (peek(1).has_value() && require_semi){
                    consume();
                    require_semi = false;
                }
                else if (peek(1).has_value() && !require_semi) { 
                    consume();
                    continue; 
                }
                else if (!peek(1).has_value() && !require_semi) {
                    consume();
                    continue; 
                }
                break;
            }

            case TokenType::KW_EXIT: {
                if (!peek(1).has_value() || 
                    (peek(1).value().type != TokenType::LIT_INT &&
                    peek(1).value().type != TokenType::UNCLASSED_VAR_DEC)) {
                    std::println("Error: Required valid argument after `exit`");

                    std::println("DEBUG: Type Unclassed = {}", peek(1).value().type == TokenType::UNCLASSED_VAR_DEC);
                    exit(EXIT_FAILURE);
                } else {
                    switch (peek(1).value().type) {
                        case TokenType::UNCLASSED_VAR_DEC: {
                            if (!peek(2).has_value() || 
                                peek(2).value().type != TokenType::DELIM_SEMI) {
                                std::println("Error: Missing `;` after declaration `{}`", 
                                             std::get<std::string>(peek(2).value().value));
                                exit(EXIT_FAILURE);
                            }

                            Token peeked = peek(1).value();

                            int ec_int_lit = 0;
                            std::string ident{};

                            auto getter = Overload {
                                [&](const std::string &s){ ident = s; },
                                [&](const int &i){ ec_int_lit = i; },
                            };
                            std::visit(getter, peeked.value);
                            std::println("IDENT: {}", ident);

                            auto tree_val = ast.lookup_node(TokenType::VAR_INT, ident);
                            if (!tree_val.has_value()) {
                                std::println(stderr, "Error: Could not get SyntaxNode with ident: {}", ident);
                                exit(EXIT_FAILURE);
                            }

                            SyntaxNode snode = tree_val.value();
                            ec_int_lit = std::get<NodeIntVar>(tree_val
                                .value()
                                .get_node_value(TokenType::VAR_INT)
                            )   .value;

                            ast.push_node(SyntaxNode( 
                                NodeExit({ 
                                    .exit_code = ec_int_lit 
                                }) 
                            ));

                            consume();
                            require_semi = true;
                            break;
                        }
                        case TokenType::LIT_INT: {
                            if (!peek(2).has_value() || peek(2).value().type != TokenType::DELIM_SEMI) {
                                std::println("Error: Missing `;` after declaration `{}`", 
                                             std::get<int>(peek(1).value().value));
                                exit(EXIT_FAILURE);
                            }

                            ast.push_node( SyntaxNode( 
                                NodeExit({ 
                                    .exit_code = std::get<int>(peek(1).value().value)
                                }) 
                            ));

                            consume();
                            require_semi = true;
                            break;
                        }
                        default: {
                            std::println(stderr, "Something went wrong");
                            exit(EXIT_FAILURE);
                        }
                    }
                    break;
                }
            } 

            case TokenType::KW_LET: {
                if (!peek(1).has_value() ||
                    peek(1).value().type != TokenType::UNCLASSED_VAR_DEC) {
                    std::println("Error: Required type specifier after `let`.");
                    exit(EXIT_FAILURE);
                } 

                if (!peek(2).has_value() || 
                    peek(2).value().type != TokenType::OP_EQUALS) {
                    std::println("Error: Missing `=`.");
                    exit(EXIT_FAILURE);
                }

                if (!peek(3).has_value() || 
                    peek(3).value().type != TokenType::LIT_INT) {
                    std::println("Error: Required valid entry after variable declaration.");
                    exit(EXIT_FAILURE);
                }

                if (!peek(4).has_value() ||
                    peek(4).value().type != TokenType::DELIM_SEMI) {
                    std::println("Error: Missing `;` after `{}`.", std::get<int>(peek(3).value().value));
                    exit(EXIT_FAILURE);
                }

                const auto ident = peek(1).value().value;
                const auto int_val = peek(3).value().value;

                // `let`
                ast.push_node(SyntaxNode{ NodeLet{} });
                // Variable
                ast.push_node(SyntaxNode{ 
                    NodeIntVar{
                        .value = std::get<int>(peek(3).value().value),
                        .ident = std::get<std::string>(peek(1).value().value),
                        .is_mutable = true,
                    } 
                });

                consume();
                require_semi = true;

                break;
            }
            case TokenType::KW_INT: {
                consume();
                break;
            }
            case TokenType::LIT_INT: {
                consume();
                break; 
            }
            case TokenType::VAR_INT: {
                consume();
                break;
            }
            case TokenType::UNCLASSED_VAR_DEC: {
                consume();
                break;
            }
            case TokenType::OP_EQUALS: {
                consume();
                break;
            }
            case TokenType::OP_PLUS: {
                consume();
                break;
            }
            default: {
                std::print(stderr, "Error: Unknown Symbol "); print_variant(peeked.value);
                exit(EXIT_FAILURE);
            }
        }
    }
    
   //boom 
    CodeGenerator generator{m_ofs, ast.get_call_stack(), ast.get_var_table()};
    generator.emit();

    std::println("BOMBO");

}

/* */
[[nodiscard]] Token Tokenizer::classify_token(std::string &buf) noexcept {
    Token tok{};

    size_t dg_cnt = 0;
    std::string int_value{};
    while (dg_cnt < buf.length() && std::isdigit(buf.at(dg_cnt))) {
        int_value.push_back(buf.at(dg_cnt));
        ++dg_cnt;
    }

    if (!int_value.empty()) {
        tok.type = TokenType::LIT_INT;
        tok.value.emplace<int>(std::stoi(int_value));
        return tok;
    }

    if (buf == ";") {
        tok.type = TokenType::DELIM_SEMI;
        tok.value.emplace<std::string>(buf);
        return tok;
    } else if (buf == "exit") {
        tok.type = TokenType::KW_EXIT;
        tok.value.emplace<std::string>(buf);
        return tok;
    } else if (buf == "let") {
        tok.type = TokenType::KW_LET;
        tok.value.emplace<std::string>(buf);
        return tok;
    } else if (buf == "=") {
        tok.type = TokenType::OP_EQUALS;
        tok.value.emplace<std::string>(buf);
        return tok;
    } else if (buf == "+") {
        tok.type = TokenType::OP_PLUS;
        tok.value.emplace<std::string>(buf);
        return tok;
    } else { // Not reserved by language
        tok.type = TokenType::UNCLASSED_VAR_DEC;
        std::println("Unclassed: {}", buf);
        tok.value.emplace<std::string>(buf);
        return tok;
    }
}
