#include "parser.hpp"
#include "code-generator.hpp"
#include "syntax-tree.hpp"
#include "tokenizer.hpp"
#include "nodes.hpp"
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>


// TODO: Make some sort of AST 
// TODO: Helper function to check token validity for kw/op

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

std::optional<Token> Parser::pop() {
    if (m_tokens.empty() || 
        (m_token_ptr) >= m_tokens.size()) {
        return std::nullopt;
    }

    return m_tokens.at(m_token_ptr++);
}

void Parser::parse_tokens() {
    bool require_semi = false;
    CodeGenerator generator(m_osref);
    SyntaxTree ast{};

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
                    pop();
                    require_semi = false;
                    break; 
                }
                else if (peek(1).has_value() && require_semi){
                    pop();
                    require_semi = false;
                }
                else if (peek(1).has_value() && !require_semi) { 
                    pop();
                    continue; 
                }
                else if (!peek(1).has_value() && !require_semi) {
                    pop();
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

                            auto tree_val = ast.get_node(ident);
                            if (!tree_val.has_value()) {
                                std::println(stderr, "Error: Could not get SyntaxNode with ident: {}", ident);
                                exit(EXIT_FAILURE);
                            }

                            SyntaxNode snode = tree_val.value();
                            ec_int_lit = std::get<NodeIntVar>(tree_val
                                .value()
                                .get_node_value(TokenType::VAR_INT)
                            )   .value;

                            generator.emit(SyntaxNode( NodeExit({ .exit_code = ec_int_lit }) ));

                            pop();
                            require_semi = true;
                            break;
                        }
                        case TokenType::LIT_INT: {
                            if (!peek(2).has_value() || peek(2).value().type != TokenType::DELIM_SEMI) {
                                std::println("Error: Missing `;` after declaration `{}`", 
                                             std::get<int>(peek(1).value().value));
                                exit(EXIT_FAILURE);
                            }

                            int ec_int_lit = 0;
                            auto getter = Overload {
                                [&](const int &i) { ec_int_lit = i; },
                                [&](std::string) {},
                            };
                            std::visit(getter, peek(1).value().value);

                            generator.emit(SyntaxNode( NodeExit({ .exit_code = ec_int_lit }) ));

                            pop();
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

                // create new node
                NodeIntVar inode{};
                auto paramgetter = [&](const auto &v) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>) {
                        inode.ident = v;
                        // std::println("IDENT: {}", node.ident);
                    }
                    else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, int>) {
                        inode.value = v;
                        // std::println("VALUE: {}", node.value);
                    };
                } ;

                inode.is_mutable = true;
                std::visit(paramgetter, ident);
                std::visit(paramgetter, int_val);
                SyntaxNode snode{inode};

                // Store variable node
                ast.push_node(snode);

                // Generate variable storage code
                generator.emit(snode);

                pop();
                require_semi = true;

                break;
            }
            case TokenType::KW_RETURN: {
                pop();
                break;
            } 
            case TokenType::LIT_INT: {
                pop();
                break; 
            }
            case TokenType::KW_INT: {
                pop();
                break;
            }
            case TokenType::LIT_STR:  {
                pop();
                break; 
            }
            case TokenType::UNCLASSED_VAR_DEC: {
                pop();
                break;
            }
            case TokenType::OP_EQUALS: {
                pop();
                break;
            }
            case TokenType::VAR_INT: {
                pop();
                break;
            }
            default: {

                std::print(stderr, "Error: Unknown Symbol "); print_variant(peeked.value);
                exit(EXIT_FAILURE);
            }
        }
    }


}
