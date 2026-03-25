#pragma once

#include "lexer.hpp"
#include "syntax-tree.hpp"
#include "vector"
#include <cstddef>
#include <cstdlib>
#include <optional>

class Parser {
  public:
    Parser(std::vector<Token> &&toks) noexcept;

    NodeProgram &&create_program();

  private:
    std::vector<Token> m_tokens;
    size_t m_tok_ptr = 0;
    NodeProgram m_program;

    std::vector<SyntaxNode> m_ast;
    std::unordered_map<std::string, SyntaxNode*> m_var_table;

    std::tuple<float, float> get_binding_power(BinOp bop) const;
    bool validate_token(const size_t offset, const TokenType ttype) const;
    [[nodiscard]]    std::optional<Token> peek(size_t offset) const;
    [[maybe_unused]] std::optional<Token> next();
    [[nodiscard]]    NodeVarDeclaration parse_declaration(TokenType ttype);
    [[nodiscard]]    std::unique_ptr<NodeBinaryExpr> parse_expr(float min_rbp);
    [[nodiscard]]    BinOp set_op(const std::string& optype) const;
    [[nodiscard]]    NodeStmtExit parse_stmt_exit(TokenType ttype);
};
