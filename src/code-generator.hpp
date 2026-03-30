#include "syntax-tree.hpp"
#include <cstddef>
#include <fstream>
#include <string>

enum class Marker{
    END
};

class CodeGenerator {
  public:
     explicit CodeGenerator(NodeProgram&& prog) noexcept;
    ~CodeGenerator();

    void emit(const NodeScope& node);


  private:
    std::ofstream m_os {};
    size_t m_node_ptr = 0;
    size_t m_stack_sz = 0;
    int32_t m_stack_ptr = 0;
    size_t m_var_count = 0;
    size_t m_lbl_count = 0;
    bool m_expand_stack = true;
    NodeProgram m_program{};
    std::unordered_map<std::string, int32_t> m_cached_var;

    //===================================

    [[nodiscard]]    const SyntaxNode* peek(size_t offset) const;
    [[maybe_unused]] const SyntaxNode* next();
    [[nodiscard]]    int32_t emit_expr(const NodeBinaryExpr& node);
                     void get_count_vars(const NodeScope& node);
                     void emit_stmt_exit(const NodeStmtExit& node);
                     // void emit_conditional(const std::variant<NodeStmtIf, NodeStmtElif>& node);
                     void emit_conditional(std::variant<const NodeStmtIf*, const NodeStmtElif*> node);
                     void emit_stmt_else(const NodeStmtElse& node);
                     void emit_decl(const NodeVarDeclaration& node);
                     void emit_epilogue();
    friend class CodeGenTests;
};
