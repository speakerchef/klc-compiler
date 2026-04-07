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
    std::unordered_map<std::string, NodeVarDeclaration*> m_var_table;
    std::unordered_map<std::string, NodeFunc*> m_fn_table;

//===================================================================================================
 
    [[nodiscard]]       static std::tuple<float, float> get_infix_bpower(Op op);
    [[nodiscard]]       float get_prefix_bpower(Op op);
    [[nodiscard]]       float get_postfix_bpower(Op op);
    [[nodiscard]]       static bool is_assign_or_unary_op(Op op);
    [[nodiscard]]       static Op set_op(const std::string& optype);
    [[nodiscard]]       bool validate_token(size_t offset, TokenType ttype, Op bop) const;
    [[nodiscard]]       std::optional<Token> peek(size_t offset) const;
    [[maybe_unused]]    std::optional<Token> next();
    [[nodiscard]]       NodeVarDeclaration parse_declaration(TokenType ttype, 
                                   NodeScope& loc_scp, bool is_reassign);
    [[nodiscard]]       Fix get_fix();
    [[nodiscard]]       std::unique_ptr<SyntaxNode> parse_expr(bool chk_for_paren);
    [[nodiscard]]       std::unique_ptr<NodeExpr> parse_expr_impl(float min_rbp);
    [[nodiscard]]       NodeStmtExit parse_stmt_exit(TokenType ttype,
                        const std::unordered_map<std::string, NodeVarDeclaration*>& loc_scp);
    [[nodiscard]]       NodeStmtIf parse_stmt_if(NodeScope& loc_scp);
    [[nodiscard]]       std::vector<NodeStmtElif> parse_stmt_elif(NodeScope& loc_scp);
    [[nodiscard]]       std::optional<NodeStmtElse> parse_stmt_else(NodeScope& loc_scp);
    [[nodiscard]]       NodeStmtWhile parse_stmt_while(NodeScope& loc_scp);
    [[nodiscard]]       NodeFunc parse_stmt_fn(NodeScope& loc_scp);
    [[nodiscard]]       NodeScope parse_stmt(bool is_prog, NodeScope& loc_scp);
                        void check_semi() const;
                        void prnt_tok_seq(size_t range);

//===================================================================================================
                        friend class ParserTests;
};
