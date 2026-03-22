#include "tokenizer.hpp"
#include "include/utils.hpp"
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <print>
#include <string>
#include <vector>

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
    if (buf == "exit") {
        tok.type = TokenType::KW_EXIT;
        tok.value.emplace<std::string>(buf);
        return tok;
    } else if (buf == "return") {
        tok.type = TokenType::KW_RETURN;
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
    } else {
        tok.type = TokenType::UNCLASSED_VAR_DEC;
        std::println("Unclassed: {}", buf);
        tok.value.emplace<std::string>(buf);
        return tok;
    }


    std::println(stderr, "Unknown Symbol: `{}`", buf);
    exit(EXIT_FAILURE);
}

[[nodiscard]] std::vector<Token> Tokenizer::tokenize(std::ifstream file) {
    char ch;
    std::string buf;
    std::vector<Token> tokens{};

    while (file.get(ch)) {
        if (std::isalnum(ch)) {
            buf.push_back(ch);

        } 
        else if (std::isspace(ch)) {
            if (!buf.empty()) {
                tokens.emplace_back(this->classify_token(buf));
                buf.clear();
            }
        } 
        else if (ch == ';') {
            if (!buf.empty()) {
                tokens.emplace_back(this->classify_token(buf));
                buf.clear();
            }

            // Given buf had a token and we parsed it,
            // we add the semi token separately
            tokens.emplace_back(
                Token({ .type = TokenType::DELIM_SEMI, .value = ";"}));
        }
        else if (ch == '=') {
            if (!buf.empty()) {
                tokens.emplace_back(this->classify_token(buf));
                buf.clear();
            }

            tokens.emplace_back(
                Token({ .type = TokenType::OP_EQUALS, .value = "="}));
        }
    }
    return tokens;
}
