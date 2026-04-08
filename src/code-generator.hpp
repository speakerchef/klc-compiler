#pragma once
#include "syntax-tree.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

struct CachedLoc {
    int32_t adr;
    std::string reg;
};

class CodeGenerator {
  public:
     explicit CodeGenerator(NodeProgram&& prog, const std::string& exec_name) noexcept;
    ~CodeGenerator() = default;

    void emit(const NodeScope& node, std::ostream& buf);


  private:
    std::ofstream m_os {};
    size_t m_node_ptr = 0;
    size_t m_stack_sz = 0;
    int32_t m_stack_ptr = 0;
    size_t m_var_count = 0;
    size_t m_lbl_count = 0;
    NodeProgram m_program{};
    std::ostringstream m_fn_buf{};
    std::unordered_map<std::string, int32_t> m_cached_var;

    //===============================================================================

    [[nodiscard]]            const SyntaxNode* peek(size_t offset) const;
    [[maybe_unused]]         const SyntaxNode* next();
    [[nodiscard]]            std::tuple<int32_t, int32_t>
                                emit_expr(NodeExpr& node,
                                          const std::optional<CachedLoc>& cached_adr,
                                          bool fresh_alloc,
                                          std::ostream& buf);
    [[nodiscard]]            std::string get_reg(const std::string& id) const;
                             bool emit_op(NodeExpr& node, std::ostream& buf);
                             void emit_store_literal(int64_t val, std::ostream& buf);
    [[nodiscard]]            size_t get_count_vars(const NodeScope& node);
                             void emit_stmt_exit(const NodeStmtExit& node, std::ostream& buf);
                             void emit_stmt_if(const NodeStmtIf& node, const std::string& lbl_if,
                                                                     const std::string& lbl_else,
                                                                     const std::string& lbl_end, 
                                                                    std::ostream& buf);
                             void emit_stmt_else(const NodeStmtElse& node, std::ostream& buf);
                             void emit_stmt_while(const NodeStmtWhile& node, std::ostream& buf);
                             void emit_decl(const NodeVarDeclaration& node, std::ostream& buf);
                             void emit_call_expr(const NodeCall& node, std::ostream& buf);
                             void emit_stmt_fn(const NodeFunc& node, std::ostream& buf);
                             void emit_epilogue(std::ostream& buf);

    //===============================================================================
    friend class CodeGenTests;
};
