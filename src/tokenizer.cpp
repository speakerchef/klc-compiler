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
    if (buf == "exit") {
        tok = {.type = TokenType::KW_EXIT, .value = buf};
        return tok;
    } else if (buf == "return") {
        tok = {.type = TokenType::KW_RETURN, .value = buf};
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
            .type = TokenType::LIT_INT,
            .value = std::stoi(int_value),
        };

        return tok;
    }

    return {.type = TokenType::CLASS_ERROR, .value = buf};
}
[[nodiscard]] std::vector<Token> Tokenizer::tokenize(std::ifstream file) {
    char ch;
    std::string buf;
    std::vector<Token> tokens{};

    while (file.get(ch)) {
        if (std::isalnum(ch)) {
            buf.push_back(ch);
        } else if (std::isspace(ch) && ch != '\n') {
            Token tok = this->classify_token(buf);
            if (tok.type == TokenType::CLASS_ERROR) {
                std::print(stderr, "Unknown Symbol: ");
                print_variant(tok.value);
                exit(EXIT_FAILURE);
            }
            tokens.emplace_back(tok);
            buf.clear();
        } else if (ch == ';') {
            if (!buf.empty()) {
                assert(this->classify_token(buf).type !=
                       TokenType::CLASS_ERROR);
                tokens.emplace_back(this->classify_token(buf));
                buf.clear();
            }

            // Given buf had a token and we parsed it,
            // we add the semi token separately
            tokens.emplace_back(
                Token({.type = TokenType::DELIM_SEMI, .value = ch}));
        }
    }
    return tokens;
}
