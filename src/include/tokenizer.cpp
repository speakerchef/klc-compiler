#include "tokenizer.hpp"
#include <cassert>
#include <cctype>
#include <fstream>
#include <string>
#include <vector>

[[nodiscard]] Token Tokenizer::parse_token(std::string &buf) noexcept {
    Token tok{};
    if (buf == "exit") {
        tok = {.type = TokenType::T_KW_EXIT, .value = buf};

        return tok;
    }

    size_t dg_cnt = 0;
    std::string int_value{};
    while (dg_cnt < buf.length() && std::isdigit(buf.at(dg_cnt))) {
        int_value.push_back(buf.at(dg_cnt));
        ++dg_cnt;
    }

    if (!int_value.empty()) {
        tok = {
            .type = TokenType::T_TYPE_INT,
            .value = std::stoi(int_value),
        };

        return tok;
    }

    return {.type = TokenType::PARSE_ERROR, .value = -1};
}
[[nodiscard]] std::vector<Token> Tokenizer::tokenize(std::ifstream file) {
    char ch;
    std::string buf;
    std::vector<Token> tokens{};
    while (file.get(ch)) {
        if (std::isalnum(ch)) {
            buf.push_back(ch);
        } else if (std::isspace(ch) && ch != '\n') {
            println("Buffer at '<space>' check: {}", buf);
            assert(this->parse_token(buf).type != TokenType::PARSE_ERROR);
            tokens.emplace_back(this->parse_token(buf));
            buf.clear();
        } else if (ch == ';') {
            if (!buf.empty()) {
                println("Buffer at ';' check: {}", buf);
                assert(this->parse_token(buf).type != TokenType::PARSE_ERROR);
                tokens.emplace_back(this->parse_token(buf));
                buf.clear();
            }

            // Given buf had a token and we parsed it,
            // we add the semi token separately
            tokens.emplace_back(
                Token({.type = TokenType::T_DELIM_SEMI, .value = ch}));
        }
    }
    return tokens;
}
