#pragma once

#include "parser.h"
namespace soft {
  namespace ir {
    struct Constant {
      Type type;
      std::variant<uint64_t, int64_t, double> v;
    };
    struct Register {
      Type type;
      size_t id;
    };
    using Value = std::variant<Constant, Register>;

    struct Return {
      Value v;
    };
    using Terminator = std::variant<Return>;

    struct Alloca {
      Type type;
      Register reg;
    };
    struct Store {
      Value src;
      Register dst;
    };
    struct BinOp {
      enum class Op { Add, Sub, Mul, Div } op;
      Value lhs, rhs;
      Register dst;
    };
    struct UnOp {
      enum class Op { Neg, Not } op;
      Value operand;
      Register dst;
    };
    struct Conv {
      Value src;
      Register dst;
    };
    using Instruction = std::variant<Alloca, Store, BinOp, UnOp, Conv>;

    struct Function {
      std::string name;
      std::optional<Type> type;
      std::optional<Terminator> terminator;
      std::vector<Register> params;
      std::vector<Instruction> instrs;
      enum class Mod { Definition, Declaration } mod;
    };

    struct Global {
      std::string name;
      Type type;
      std::optional<Constant> init;
      enum class Mod { Definition, Declaration } mod;
    };

    struct Program {
      std::vector<Function> fns;
      std::vector<Global> globals;
    };

    // main functions
    Value generate_expr(const std::unique_ptr<ast::Expr>& expr);

    void generate_fndef(const std::unique_ptr<ast::FnDef>& def);
    void generate_fndec(const std::unique_ptr<ast::FnDecl>& dec);
    void generate_return(const std::unique_ptr<ast::Return>& rt);
    void generate_stmt(const std::unique_ptr<ast::Stmt>& stmt);

    Program generate(const std::vector<std::unique_ptr<ast::Stmt>>& ast);
  }
}
