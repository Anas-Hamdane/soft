#pragma once

#include "stl.h"
#include "lexer.h"
#include "data/Type.h"

// the parser itself
namespace soft {
  namespace ast {
    // forward delcare all the stuff
    struct IntLit;
    struct FloatLit;
    struct CharLit;
    struct StrLit;
    struct ArrLit;
    struct Identifier;
    struct VarDecl;
    struct FnCall;
    struct AssgnOp;
    struct BinOp;
    struct UnOp;

    struct Return;
    struct Expmt;
    struct FnDecl;
    struct FnDef;

    // the main variants
    using Expr = std::variant<std::unique_ptr<IntLit>, std::unique_ptr<FloatLit>,
                              std::unique_ptr<CharLit>, std::unique_ptr<StrLit>,
                              std::unique_ptr<ArrLit>, std::unique_ptr<Identifier>,
                              std::unique_ptr<VarDecl>, std::unique_ptr<FnCall>,
                              std::unique_ptr<AssgnOp>, std::unique_ptr<BinOp>,
                              std::unique_ptr<UnOp>>;

    using Stmt = std::variant<std::unique_ptr<Return>, std::unique_ptr<Expmt>,
                              std::unique_ptr<FnDecl>, std::unique_ptr<FnDef>>;

    // define the rest
    struct IntLit {
      uint64_t v;
    };
    struct FloatLit {
      double v;
    };
    struct CharLit {
      char v;
    };
    struct StrLit {
      std::string v;
    };
    struct ArrLit {
      std::vector<std::unique_ptr<Expr>> elms;
    };
    struct Identifier {
      std::string name;
    };
    struct VarDecl {
      std::string name;
      std::unique_ptr<Type> type;
      std::unique_ptr<Expr> init;
    };
    struct FnCall {
      std::string name;
      std::vector<std::unique_ptr<Expr>> args;
    };
    struct AssgnOp {
      std::unique_ptr<Expr> var;
      std::unique_ptr<Expr> val;
    };
    struct BinOp {
      std::unique_ptr<Expr> lhs;
      std::unique_ptr<Expr> rhs;
      Token::Knd op;
    };
    struct UnOp {
      std::unique_ptr<Expr> oprand;
      Token::Knd op;
    };

    struct Return {
      std::unique_ptr<Expr> expr;
    };
    struct Expmt {
      std::unique_ptr<Expr> expr;
    };
    struct FnDecl {
      std::string name;
      std::unique_ptr<Type> type;
      std::vector<std::unique_ptr<VarDecl>> params;
    };
    struct FnDef {
      std::unique_ptr<FnDecl> dec;
      std::vector<std::unique_ptr<Stmt>> body;
    };

    // functions
    Token peek();
    Token advance();
    bool match(Token::Knd knd);
    Token expect(Token::Knd knd);

    std::unique_ptr<Type> generate_type();
    std::unique_ptr<Expr> generate_primary();
    std::unique_ptr<Expr> generate_expression(const int min_prec = 0);
    std::unique_ptr<Stmt> generate_function();
    std::unique_ptr<Stmt> generate_return();
    std::unique_ptr<Stmt> generate_expmt();
    std::unique_ptr<Stmt> generate_stmt();

    std::vector<std::unique_ptr<Stmt>> generate(const std::vector<Token>& tokens);
  }
}
