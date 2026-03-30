#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sys/wait.h>

#include "code-generator.hpp"
#include "lexer.hpp"
#include "parser.hpp"

static std::string write_temp_file(const std::string &content) {
  static int counter = 0;
  const auto path = std::filesystem::temp_directory_path() /
              std::format("klc_test_{}.knv", counter++);
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path.string();
}

static std::vector<Token> tokenize_string(const std::string &source) {
  const auto path = write_temp_file(source);
  Lexer lex(path);
  auto tokens = lex.tokenize();
  std::filesystem::remove(path);
  return tokens;
}

static NodeProgram parse_string(const std::string &source) {
  auto tokens = tokenize_string(source);
  Parser parser(std::move(tokens));
  return std::move(parser.create_program());
}

static int run_program(const std::string &source) {
  auto src_path = write_temp_file(source);

  Lexer lex(src_path);
  {
    Parser parser(lex.tokenize());
    CodeGenerator gen(std::move(parser.create_program()));
  }

  int asm_result = std::system(
      "xcrun clang -arch arm64 -o /tmp/klc_test_bin ./gen_asm.s 2>/dev/null");
  std::filesystem::remove(src_path);
  if (asm_result != 0)
    return -1;

  int run_result = std::system("/tmp/klc_test_bin");
  int exit_code = WEXITSTATUS(run_result);
  std::filesystem::remove("/tmp/klc_test_bin");
  return exit_code;
}

static const NodeBinaryExpr &get_decl_expr(const NodeProgram &prog,
                                           size_t idx) {
  const auto &decl = std::get<NodeVarDeclaration>(prog.main.stmts[idx]->m_node);
  return std::get<NodeBinaryExpr>(decl.value->m_node);
}

static bool atom_is_int(const NodeBinaryExpr &e, int val) {
  const auto *lit = std::get_if<NodeIntLiteral>(&e.atom);
  return lit && lit->value == val;
}

TEST(Parser, OperatorPrecedence) {
  auto prog = parse_string("let x = 2 + 3 * 4;");
  const auto &expr = get_decl_expr(prog, 0);

  EXPECT_EQ(expr.op, BinOp::ADD);
  ASSERT_NE(expr.lhs, nullptr);
  ASSERT_NE(expr.rhs, nullptr);
  EXPECT_TRUE(atom_is_int(*expr.lhs, 2));
  EXPECT_EQ(expr.rhs->op, BinOp::MUL);
  EXPECT_TRUE(atom_is_int(*expr.rhs->lhs, 3));
  EXPECT_TRUE(atom_is_int(*expr.rhs->rhs, 4));
}

TEST(Parser, Parenthesization) {
  auto prog = parse_string("let x = (2 + 3) * 4;");
  const auto &expr = get_decl_expr(prog, 0);

  EXPECT_EQ(expr.op, BinOp::MUL);
  ASSERT_NE(expr.lhs, nullptr);
  EXPECT_EQ(expr.lhs->op, BinOp::ADD);
  EXPECT_TRUE(atom_is_int(*expr.rhs, 4));
}

TEST(Parser, LeftAssociativity) {
  auto prog = parse_string("let x = 1 + 2 + 3;");
  const auto &expr = get_decl_expr(prog, 0);

  EXPECT_EQ(expr.op, BinOp::ADD);
  EXPECT_TRUE(atom_is_int(*expr.rhs, 3));
  ASSERT_NE(expr.lhs, nullptr);
  EXPECT_EQ(expr.lhs->op, BinOp::ADD);
  EXPECT_TRUE(atom_is_int(*expr.lhs->lhs, 1));
  EXPECT_TRUE(atom_is_int(*expr.lhs->rhs, 2));
}

TEST(Parser, MixedPrecedenceChain) {
  auto prog = parse_string("let x = 1 * 2 + 3 * 4;");
  const auto &expr = get_decl_expr(prog, 0);

  EXPECT_EQ(expr.op, BinOp::ADD);
  ASSERT_NE(expr.lhs, nullptr);
  ASSERT_NE(expr.rhs, nullptr);
  EXPECT_EQ(expr.lhs->op, BinOp::MUL);
  EXPECT_EQ(expr.rhs->op, BinOp::MUL);
}


TEST(E2E, LiteralExit)    { EXPECT_EQ(run_program("exit 42;"), 42); }
TEST(E2E, ZeroExit)       { EXPECT_EQ(run_program("exit 0;"), 0); }
                          
TEST(E2E, Addition)       { EXPECT_EQ(run_program("exit 2 + 3;"), 5); }
TEST(E2E, Subtraction)    { EXPECT_EQ(run_program("exit 10 - 3;"), 7); }
TEST(E2E, Multiplication) { EXPECT_EQ(run_program("exit 3 * 4;"), 12); }
TEST(E2E, Division)       { EXPECT_EQ(run_program("exit 10 / 2;"), 5); }

TEST(E2E, Precedence)     { EXPECT_EQ(run_program("exit 2 + 3 * 4;"), 14); }
TEST(E2E, Parenthesized)  { EXPECT_EQ(run_program("exit (2 + 3) * 4;"), 20); }

TEST(E2E, VariableExit)   { EXPECT_EQ(run_program("let x = 10;\nexit x;"), 10); }

TEST(E2E, VariableArithmetic) {
  EXPECT_EQ(run_program("let x = 5;\nlet y = 3;\nexit x + y;"), 8);
}

TEST(E2E, VariableChain) {
  EXPECT_EQ(run_program("let a = 5;\nlet b = a + 3;\nexit b;"), 8);
}

TEST(E2E, ComplexExpression) {
  EXPECT_EQ(run_program("let x = 5 * 6 / 2 - 5;\n"
                        "let y = (1 + 2) * (3 + 4);\n"
                        "exit (y - x) * (y - x);"),
            121);
}
