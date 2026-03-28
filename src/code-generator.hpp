#include "syntax-tree.hpp"
#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

enum class Marker{
    END
};

class CodeGenerator {
  public:
     explicit CodeGenerator(NodeProgram&& prog) noexcept;
    ~CodeGenerator();

    void emit();

    [[nodiscard]] int consteval_expr(const NodeBinaryExpr& node); // only for compile time eval
    [[nodiscard]] int32_t emit_expr(const NodeBinaryExpr& node);
                  void emit_stmt_exit(const NodeStmtExit& node);
                  void emit_epilogue();
    void emit_decl(const NodeVarDeclaration& node);

  private:
    std::ofstream m_os;
    size_t m_node_ptr = 0;
    size_t m_stack_sz = 0;
    int32_t m_stack_ptr = 0;
    bool expand_stack = true;
    NodeProgram m_program{};
    std::unordered_map<std::string, int32_t> m_cached_var;

    //===================================

    [[nodiscard]] const SyntaxNode* peek(size_t offset) const;
    [[maybe_unused]] const SyntaxNode* next();
    [[nodiscard]] std::string gen_var_declaration();
};
