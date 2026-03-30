#pragma once

#include "lexer.hpp"
#include "syntax-tree.hpp"
#include "vector"
#include <cstddef>
#include <cstdlib>
#include <optional>

class Parser {
  public:
    explicit Parser(std::vector<Token> &&toks) noexcept;
    NodeProgram&& create_program();

  private:
    std::vector<Token> m_tokens;
    size_t m_tok_ptr = 0;
    NodeProgram m_program;

    std::vector<SyntaxNode> m_ast;
    std::unordered_map<std::string, SyntaxNode*> m_var_table;

    [[nodiscard]]    static std::tuple<float, float> get_binding_power(BinOp bop);
    [[nodiscard]]    static BinOp set_op(const std::string& optype);
    [[nodiscard]]    bool validate_token(size_t offset, TokenType ttype, BinOp bop) const;
    [[nodiscard]]    std::optional<Token> peek(size_t offset) const;
    [[maybe_unused]] std::optional<Token> next();
    [[nodiscard]]    NodeVarDeclaration parse_declaration(TokenType ttype,
                        std::unordered_map<std::string, SyntaxNode*>& loc_scp, const bool is_prog);
    [[nodiscard]]    std::unique_ptr<SyntaxNode> parse_expr();
    [[nodiscard]]    std::unique_ptr<NodeBinaryExpr> parse_expr_impl(float min_rbp);
    [[nodiscard]]    NodeStmtExit parse_stmt_exit(TokenType ttype,
                        const std::unordered_map<std::string, SyntaxNode*>& loc_scp);
    // [[nodiscard]]    NodeStmtIf parse_stmt_if(std::unordered_map<std::string, SyntaxNode*>& loc_scp);
    [[nodiscard]]    NodeStmtIf parse_stmt_if(NodeScope& loc_scp);
    // [[nodiscard]]    NodeStmtElif parse_stmt_elif(std::unordered_map<std::string, SyntaxNode*>& loc_scp);
    [[nodiscard]]    NodeStmtElif parse_stmt_elif(NodeScope& loc_scp);
    // [[nodiscard]]    NodeStmtElse parse_stmt_else(std::unordered_map<std::string, SyntaxNode*>& loc_scp);
    [[nodiscard]]    NodeStmtElse parse_stmt_else(NodeScope& loc_scp);
    // [[nodiscard]]    NodeScope parse_stmt(bool is_prog,
    //     std::unordered_map<std::string, SyntaxNode*>& loc_scp);
    [[nodiscard]]    NodeScope parse_stmt(bool is_prog, NodeScope& loc_scp);
    friend class ParserTests;
};
