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

    std::unique_ptr<Type> parse_type();
    std::unique_ptr<Expr> parse_primary();
    std::unique_ptr<Expr> parse_expression();
    std::unique_ptr<Stmt> parse_function();
    std::unique_ptr<Stmt> parse_return();
    std::unique_ptr<Stmt> parse_expmt();

    std::vector<std::unique_ptr<Stmt>> parse(std::vector<Token> tokens)
    {
      std::vector<std::unique_ptr<Stmt>> ast;
      tkns = tokens;
      index = 0;

      while (!match(Token::Knd::EndOfFile))
      {}
    }
  }
}
