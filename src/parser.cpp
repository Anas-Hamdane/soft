#include "parser.h"
#include <cassert>
#include <cfloat>
#include <cmath>

namespace soft {
  namespace ast {
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

    uint64_t generate_decimal(const std::string& str)
    {
      uint64_t result = 0;
      for (char c : str) {
        // skip separators
        if (c == '\'')
          continue;

        int digit = c - '0';
        if (result > (UINT64_MAX - digit) / 10)
        {
          std::println(stderr, "decimal literal overflow for {}", str);
          exit(1);
        }

        result = (result * 10) + digit;
      }

      return result;
    }
    uint64_t generate_hex(const std::string& str)
    {
      uint64_t result = 0;
      assert(str.starts_with("0x") || str.starts_with("0X"));

      // start after the prefix
      for (size_t i = 2; i < str.length(); ++i) {
        unsigned char c = str[i];

        // skip separators
        if (c == '\'')
          continue;

        int digit;

        if (c >= '0' && c <= '9')
          digit = c - '0';
        else if (c >= 'a' && c <= 'f')
          digit = c - 'a' + 10;
        else
          digit = c - 'A' + 10;

        if (result > (UINT64_MAX >> 4))
        {
          std::println(stderr, "Hex literal overflow for {}", str);
          exit(1);
        }

        result = (result << 4) | digit;
      }

      return result;
    }
    uint64_t generate_octal(const std::string& str)
    {
      uint64_t result = 0;
      assert(str.starts_with("0o") || str.starts_with("0O"));

      // skip prefix 
      for (size_t i = 2; i < str.length(); ++i) {
        unsigned char c = str[i];

        // skip separators
        if (c == '\'')
          continue;

        int digit = c - '0';
        if (result > (UINT64_MAX >> 3))
        {
          std::println(stderr, "Octal literal overflow {}", str);
          exit(1);
        }

        result = (result << 3) | digit;
      }

      return result;
    }
    uint64_t generate_binary(const std::string& str)
    {
      uint64_t result = 0;
      assert(str.starts_with("0b") || str.starts_with("0B"));

      // skip prefix
      for (size_t i = 2; i < str.length(); ++i) {
        unsigned char c = str[i];

        // skip separators
        if (c == '\'')
          continue;

        if (result > (UINT64_MAX >> 1))
        {
          std::println(stderr, "Binary literal overflow {}", str);
          exit(1);
        }

        result = (result << 1) | (c - '0');
      }

      return result;
    }
    uint64_t generate_integer(const std::string& str)
    {
      size_t base = lexer::number_base(str);

      switch (base)
      {
        case 2:  return generate_binary(str);
        case 8:  return generate_octal(str);
        case 10: return generate_decimal(str);
        case 16: return generate_hex(str);
        default: std::unreachable();
      }
    }
    double generate_fdecimal(const std::string& str)
    {
      auto digit = [](char c) {
        return c - '0';
      };

      enum class Section {
        Integer,
        Fraction,
        Exponent
      } section = Section::Integer;

      double result = 0;
      size_t frac_size = 0;
      bool neg_expo = false;
      uint64_t expo = 0;

      for (char c : str)
      {
        if (c == '\'')
          continue;

        if (c == '.')
        {
          section = Section::Fraction;
          continue;
        }

        if (c == 'e' || c == 'E')
        {
          section = Section::Exponent;
          continue;
        }

        if (c == '-' || c == '+')
        {
          neg_expo = c == '-';
          continue;
        }

        double d = digit(c);
        if (section == Section::Integer)
        {
          if (result > (DBL_MAX - d) / 10.0)
          {
            std::println("Decimal floating point overflow {}", str);
            exit(1);
          }
          result = (result * 10.0) + d;
        }
        else if (section == Section::Fraction)
        {
          double delta = d / pow(10.0, ++frac_size);
          if (result > (DBL_MAX - delta))
          {
            std::println("Decimal floating point overflow {}", str);
            exit(1);
          }
          result += delta;
        }
        else
        {
          if (expo > (UINT64_MAX - (uint64_t) d) / 10)
          {
            std::println("Exponent overflow in decimal floating point {}", str);
            exit(1);
          }
          expo = (expo * 10) + (uint64_t) d;
        }
      }

      result *= pow(10.0, neg_expo ? -expo : expo);
      if (std::isinf(result))
      {
        std::println(stderr, "floating point overflow {}", str);
        exit(1);
      }

      return result;
    }
    double generate_fhex(const std::string& str)
    {
      assert(str.starts_with("0x") || str.starts_with("0X"));
      auto digit = [](char c) {
        if (c >= '0' && c <= '9')
          return c - '0';
        else if (c >= 'a' && c <= 'f')
          return c - 'a' + 10;
        else
          return c - 'A' + 10;
      };

      enum class Section {
        Integer,
        Fraction,
        Exponent
      } section = Section::Integer;

      double result = 0;
      size_t frac_size = 0;
      bool neg_expo = false;
      uint64_t expo = 0;

      // skip prefix
      for (size_t i = 2; i < str.length(); ++i)
      {
        char c = str[i];

        if (c == '\'')
          continue;

        if (c == '.')
        {
          section = Section::Fraction;
          continue;
        }

        if (c == 'p' || c == 'P')
        {
          section = Section::Exponent;
          continue;
        }

        if (c == '-' || c == '+')
        {
          neg_expo = c == '-';
          continue;
        }

        double d = digit(c);
        if (section == Section::Integer)
        {
          if (result > (DBL_MAX - d) / 16.0)
          {
            std::println("Hax floating point overflow {}", str);
            exit(1);
          }
          result = (result * 16.0) + d;
        }
        else if (section == Section::Fraction)
        {
          double delta = d / pow(16.0, ++frac_size);
          if (result > (DBL_MAX - delta))
          {
            std::println("Hex floating point overflow {}", str);
            exit(1);
          }
          result += delta;
        }
        else
        {
          if (expo > (UINT64_MAX - (uint64_t) d) / 10)
          {
            std::println("Exponent overflow in decimal floating point {}", str);
            exit(1);
          }
          expo = (expo * 10) + (uint64_t) d;
        }
      }

      result *= pow(2.0, neg_expo ? -expo : expo);
      if (std::isinf(result))
      {
        std::println(stderr, "floating point overflow {}", str);
        exit(1);
      }

      return result;
    }
    double generate_float(const std::string& str)
    {
      size_t base = lexer::number_base(str);

      switch (base)
      {
        case 10: return generate_fdecimal(str);
        case 16: return generate_fhex(str);
        default: std::unreachable();
      }
    }

    std::unique_ptr<Type> generate_type()
    {
      Token token = expect(Token::Knd::DataType);

      Type type;
      if (token.form.starts_with('i'))
        type.knd = Type::Knd::Int;
      else if (token.form.starts_with('u'))
        type.knd = Type::Knd::UInt;
      else
        type.knd = Type::Knd::Float;

      type.byte = generate_integer(token.form.substr(1));
      return std::make_unique<Type>(type);
    }
    std::unique_ptr<Expr> generate_primary()
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

              call->args.push_back(generate_expression());
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
          integer->v = generate_integer(form);

          return std::make_unique<Expr>(std::move(integer));
        }
        case Token::Knd::FloatLit:
        {
          std::string form = advance().form;

          auto fp = std::make_unique<FloatLit>();
          fp->v = generate_float(form);

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
            decl->type = generate_type();
          }

          if (match(Token::Knd::Eq))
          {
            advance();
            decl->init = generate_expression();
          }

          return std::make_unique<Expr>(std::move(decl));
        }
        case Token::Knd::OpenParent: 
        {
          advance(); // (
          auto expr = generate_expression();
          expect(Token::Knd::CloseParent); // )
          return expr;
        }
        default:
        std::println("Implement support for expressions that starts with `{}`", lexer::kndts(peek().knd));
        return nullptr;
      }
    }
    std::unique_ptr<Expr> generate_expression(const int min_prec)
    {
      std::unique_ptr<Expr> left = generate_primary();

      while (true) {
        Token::Knd op = peek().knd;
        const int prec = precedence(op);

        if (prec <= min_prec) break;
        advance();

        int next_min = right_associative(op) ? prec : (prec + 1);
        std::unique_ptr<Expr> right = generate_expression(next_min);

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
    std::unique_ptr<Stmt> generate_function()
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
        param->type = generate_type();
        decl->params.push_back(std::move(param));
      } while(match(Token::Knd::Comma));
      expect(Token::Knd::CloseParent);

      if (match(Token::Knd::RightArrow))
      {
        advance();
        decl->type = generate_type();
      }

      if (match(Token::Knd::SemiColon))
      {
        advance();
        return std::make_unique<Stmt>(std::move(decl));
      }

      expect(Token::Knd::OpenCurly);

      auto def = std::make_unique<FnDef>();
      def->dec = std::move(decl);
      while (!match(Token::Knd::CloseCurly))
        def->body.push_back(generate_stmt());

      expect(Token::Knd::CloseCurly);
      return std::make_unique<Stmt>(std::move(def));
    }
    std::unique_ptr<Stmt> generate_return()
    {
      expect(Token::Knd::Return);
      auto expr = generate_expression();
      expect(Token::Knd::SemiColon);
      return std::make_unique<Stmt>(std::make_unique<Return>(std::move(expr)));
    }
    std::unique_ptr<Stmt> generate_expmt()
    {
      auto expr = generate_expression();
      expect(Token::Knd::SemiColon);
      return std::make_unique<Stmt>(std::make_unique<Expmt>(std::move(expr)));
    }
    std::unique_ptr<Stmt> generate_stmt()
    {
      switch (peek().knd)
      {
        case Token::Knd::Fn:     return generate_function();
        case Token::Knd::Return: return generate_return();
        default:                 return generate_expmt();
      }
    }

    std::vector<std::unique_ptr<Stmt>> generate(const std::vector<Token>& tokens)
    {
      std::vector<std::unique_ptr<Stmt>> ast;
      tkns = tokens;
      index = 0;

      while (!match(Token::Knd::EndOfFile))
      {
        auto stmt = generate_stmt();
        ast.push_back(std::move(stmt));
      }

      return ast;
    }
  }
}
