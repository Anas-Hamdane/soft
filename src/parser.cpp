#include "parser.h"

namespace soft {
  namespace parser {
    std::vector<Token> tkns;
    size_t index;

    Token peek()
    {
      if (index >= tkns.size())
        return tkns.back();

      return tkns[index];
    }
    Token advance()
    {
      if (index >= tkns.size())
        return tkns.back();

      return tkns[index++];
    }
    bool match(Token::Knd knd)
    {
      if (index >= tkns.size())
        return false;

      return (tkns[index].knd == knd);
    }
    Token expect(Token::Knd knd)
    {
      if (match(knd))
        return advance();

      std::println(stderr, "expected token '{}' but got '{}'", lexer::kndts(knd), lexer::kndts(peek().knd));
      exit(1);
    }
    int precedence(Token::Knd knd)
    {
      switch (knd)
      {
        case Token::Knd::Eq:
          return 5;
        case Token::Knd::Plus:
        case Token::Knd::Minus:
          return 10;
        case Token::Knd::Mul:
        case Token::Knd::Div:
          return 20;
        default:
          return 0;
      }
    }
    bool right_associative(Token::Knd knd)
    {
      return (knd == Token::Knd::Eq);
    }

    std::unique_ptr<Type> parse_type()
    {
      Token token = expect(Token::Knd::DataType);

      Type type;
      if (token.form.starts_with('i'))
        type.knd = Type::Knd::Int;
      else if (token.form.starts_with('u'))
        type.knd = Type::Knd::UInt;
      else
        type.knd = Type::Knd::Float;

      // TODO: use parse decimal here too
      type.byte = 4;

      return std::make_unique<Type>(type);
    }
    std::unique_ptr<Expr> parse_primary()
    {
      switch (peek().knd)
      {
        case Token::Knd::Identifier:
        {
          std::string name = advance().form;

          // function call
          if (match(Token::Knd::OpenParent)) 
          {
            advance();
            auto call = std::make_unique<FnCall>();
            call->name = name;

            do {
              if (match(Token::Knd::CloseParent))
                break;

              if (match(Token::Knd::Comma))
                advance();

              call->args.push_back(parse_expression());
            } while (match(Token::Knd::Comma));

            expect(Token::Knd::CloseParent);
            return std::make_unique<Expr>(std::move(call));
          }

          auto ide = std::make_unique<Identifier>();
          ide->name = name;
          return std::make_unique<Expr>(std::move(ide));
        }
        case Token::Knd::IntLit:
        {
          std::string form = advance().form;

          auto integer = std::make_unique<IntLit>();
          integer->v = 0; // TODO: we need the numerical parsers

          return std::make_unique<Expr>(std::move(integer));
        }
        case Token::Knd::FloatLit:
        {
          std::string form = advance().form;

          auto fp = std::make_unique<FloatLit>();
          fp->v = 0.0; // TODO: same here

          return std::make_unique<Expr>(std::move(fp));
        }
        case Token::Knd::Let:
        {
          advance();

          auto decl = std::make_unique<VarDecl>();
          decl->name = expect(Token::Knd::Identifier).form;

          if (match(Token::Knd::Colon))
          {
            advance();
            decl->type = parse_type();
          }

          if (match(Token::Knd::Eq))
          {
            advance();
            decl->init = parse_expression();
          }

          return std::make_unique<Expr>(std::move(decl));
        }
        case Token::Knd::OpenParent: 
        {
          advance(); // (
          auto expr = parse_expression();
          expect(Token::Knd::CloseParent); // )
          return expr;
        }
        default:
        std::println("Implement support for expressions that starts with `{}`\n", lexer::kndts(peek().knd));
        return nullptr;
      }
    }
    std::unique_ptr<Expr> parse_expression(const int min_prec)
    {
      std::unique_ptr<Expr> left = parse_primary();

      while (true) {
        Token::Knd op = peek().knd;
        const int prec = precedence(op);

        if (prec <= min_prec) break;
        advance();

        int next_min = right_associative(op) ? prec : (prec + 1);
        std::unique_ptr<Expr> right = parse_expression(next_min);

        if (op == Token::Knd::Eq)
        {
          auto assign = std::make_unique<AssgnOp>();
          assign->var = std::move(left);
          assign->val = std::move(right);
          left = std::make_unique<Expr>(std::move(assign));
        }
        else
        {
          auto binop = std::make_unique<BinOp>();
          binop->lhs = std::move(left);
          binop->rhs = std::move(right);
          binop->op = op;
          left = std::make_unique<Expr>(std::move(binop));
        }
      }

      return left;
    }
    std::unique_ptr<Stmt> parse_function()
    {
      expect(Token::Knd::Fn);
      auto decl = std::make_unique<FnDecl>();
      decl->name = expect(Token::Knd::Identifier).form;
      
      expect(Token::Knd::OpenParent);
      do {
        if (match(Token::Knd::CloseParent)) 
          break;

        if (match(Token::Knd::Comma))
          advance();

        std::unique_ptr<VarDecl> param;
        param->name = expect(Token::Knd::Identifier).form;
        expect(Token::Knd::Colon);
        param->type = parse_type();
        decl->params.push_back(std::move(param));
      } while(match(Token::Knd::Comma));
      expect(Token::Knd::CloseParent);

      if (match(Token::Knd::RightArrow))
      {
        advance();
        decl->type = parse_type();
      }

      if (match(Token::Knd::SemiColon))
      {
        advance();
        return std::make_unique<Stmt>(std::move(decl));
      }

      expect(Token::Knd::OpenCurly);
      auto def = std::make_unique<FnDef>();
      def->decl = std::move(decl);
      while (!match(Token::Knd::CloseCurly))
        def->body.push_back(parse_stmt());

      return std::make_unique<Stmt>(std::move(def));
    }
    std::unique_ptr<Stmt> parse_return()
    {
      expect(Token::Knd::Return);
      auto expr = parse_expression();
      return std::make_unique<Stmt>(std::make_unique<Return>(std::move(expr)));
    }
    std::unique_ptr<Stmt> parse_expmt()
    {
      auto expr = parse_expression();
      expect(Token::Knd::SemiColon);
      return std::make_unique<Stmt>(std::make_unique<Expmt>(std::move(expr)));
    }
    std::unique_ptr<Stmt> parse_stmt()
    {
      switch (peek().knd)
      {
        case Token::Knd::Fn:     return parse_function();
        case Token::Knd::Return: return parse_return();
        default:                 return parse_expmt();
      }
    }

    std::vector<std::unique_ptr<Stmt>> parse(std::vector<Token> tokens)
    {
      std::vector<std::unique_ptr<Stmt>> ast;
      tkns = tokens;
      index = 0;

      while (!match(Token::Knd::EndOfFile))
      {
        auto stmt = parse_stmt();
        ast.push_back(std::move(stmt));
      }

      return ast;
    }
  }
}
