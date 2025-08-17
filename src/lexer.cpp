#include "stl.h"
#include "lexer.h"

namespace soft {
  namespace lexer {
    std::string src;
    size_t index;

    static constexpr std::pair<std::string_view, Token::Knd> Puncts[] = {
      {"?",   Token::Knd::Qst},
      {"{",   Token::Knd::OpenCurly},
      {"}",   Token::Knd::CloseCurly},
      {"(",   Token::Knd::OpenParent},
      {")",   Token::Knd::CloseParent},
      {"[",   Token::Knd::OpenBracket},
      {"]",   Token::Knd::CloseBracket},
      {";",   Token::Knd::SemiColon},
      {":",   Token::Knd::Colon},
      {",",   Token::Knd::Comma},

      {"->",  Token::Knd::RightArrow},
      {"--",  Token::Knd::Dec},
      {"-=",  Token::Knd::MinusEq},
      {"-",   Token::Knd::Minus},

      {"++",  Token::Knd::Inc},
      {"+=",  Token::Knd::PlusEq},
      {"+",   Token::Knd::Plus},

      {"*=",  Token::Knd::MulEq},
      {"*",   Token::Knd::Mul},

      {"%=",  Token::Knd::ModEq},
      {"%",   Token::Knd::Mod},

      {"/=",  Token::Knd::DivEq},
      {"/",   Token::Knd::Div},

      {"|=",  Token::Knd::OrEq},
      {"|",   Token::Knd::Or},

      {"&=",  Token::Knd::AndEq},
      {"&",   Token::Knd::And},

      {"==",  Token::Knd::EqEq},
      {"=",   Token::Knd::Eq},

      {"!=",  Token::Knd::NotEq},
      {"!",   Token::Knd::Not},

      {"<<=", Token::Knd::ShlEq},
      {"<<",  Token::Knd::Shl},
      {"<=",  Token::Knd::LessEq},
      {"<",   Token::Knd::Less},

      {">>=", Token::Knd::ShrEq},
      {">>",  Token::Knd::Shr},
      {">=",  Token::Knd::GreaterEq},
      {">",   Token::Knd::Greater},
    };
    static constexpr std::pair<std::string_view, Token::Knd> Keywords[] = {
      {"let",      Token::Knd::Let},
      {"return",   Token::Knd::Return},
      {"fn",       Token::Knd::Fn},
      {"if",       Token::Knd::If},
      {"else",     Token::Knd::Else},
      {"while",    Token::Knd::While},
      {"for",      Token::Knd::For},
    };
    static constexpr std::string_view PrimDataTys[] = {
      // integers
      "i8",
      "i16",
      "i32",
      "i64",

      // unsigned integers
      "u8",
      "u16",
      "u32",
      "u64",

      // floating points
      "f32",
      "f64"
    };

    bool digit(char c) 
    {
      return (c >= '0' && c <= '9');
    }
    bool xdigit(char c)
    {
      return (c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'F') ||
        (c >= 'a' && c <= 'f');
    }
    bool alpha(char c) 
    {
      return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
    }
    bool alnum(char c) 
    {
      return digit(c) || alpha(c);
    }
    bool space(char c) 
    {
      // note that '\n' is exclude
      return (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f');
    }

    char peek(off_t offset)
    {
      if (index + offset >= src.length())
        return '\0';

      return src[index + offset];
    }
    char advance(off_t offset)
    {
      if (index >= src.length())
        return '\0';

      char c = src[index];
      index += offset;
      return c;
    }
    bool match(char c)
    {
      if (index >= src.length())
        return false;

      return (src[index] == c);
    }

    size_t number_base(std::string str)
    {
      if (str.starts_with("0b") || str.starts_with("0B"))
        return 2;

      if (str.starts_with("0o") || str.starts_with("0O"))
        return 8;

      if (str.starts_with("0x") || str.starts_with("OX"))
        return 16;

      return 10;
    }
    Token::Knd scan_binary(std::string lexeme)
    {
      if (lexeme.length() == 2)
      {
        std::println(stderr, "Expected at least one digit after octal prefix");
        return Token::Knd::Invalid;
      }

      size_t last_sep = 0;
      for (size_t i = 2; i < lexeme.length(); ++i)
      {
        char c = lexeme[i];

        if (c == '\'')
        {
          if (last_sep == i - 1)
          {
            std::println(stderr, "One separator allowed in numerical literals");
            return Token::Knd::Invalid;
          }
          last_sep = i;
          continue;
        }

        if (c < '0' || c > '7')
        {
          std::println(stderr, "Invalid digit in octal literal: {}", c);
          return Token::Knd::Invalid;
        }
      }

      return Token::Knd::IntLit;
    }
    Token::Knd scan_octal(std::string lexeme)
    {
      if (lexeme.length() == 2)
      {
        std::println(stderr, "Expected at least one digit after binary prefix");
        return Token::Knd::Invalid;
      }

      size_t last_sep = 0;
      for (size_t i = 2; i < lexeme.length(); ++i)
      {
        char c = lexeme[i];

        if (c == '\'')
        {
          if (last_sep == i - 1)
          {
            std::println(stderr, "One separator allowed in numerical literals");
            return Token::Knd::Invalid;
          }
          last_sep = i;
          continue;
        }

        if (c != '0' && c != '1')
        {
          std::println(stderr, "Invalid digit in binary literal: {}", c);
          return Token::Knd::Invalid;
        }
      }

      return Token::Knd::IntLit;
    }
    Token::Knd scan_hex(std::string lexeme)
    {
      enum class Section {
        Integer,
        Fraction,
        Exponent
      } section = Section::Integer;

      size_t section_size = 0;
      size_t last_sep = 0;
      Token::Knd knd = Token::Knd::IntLit;

      for (size_t i = 0; i < lexeme.length(); ++i)
      {
        char c = lexeme[i];
        if (c == '\'')
        {
          if (section_size == 0)
          {
            std::println(stderr, "Separators can't appear at the start of a section");
            return Token::Knd::Invalid;
          }

          if (last_sep == i - 1)
          {
            std::println(stderr, "Double Separators are not allowed");
            return Token::Knd::Invalid;
          }

          last_sep = i;
          continue;
        }

        if (c == '.')
        {
          if (section != Section::Integer)
          {
            std::println(stderr, "Fraction must be after a valid integer section");
            return Token::Knd::Invalid;
          }

          if (section_size == 0)
          {
            std::println(stderr, "Empty sections are not allowed");
            return Token::Knd::Invalid;
          }

          knd = Token::Knd::FloatLit;
          section = Section::Fraction;
          section_size = 0;
          continue;
        }

        if (c == 'p' || c == 'P')
        {
          if (section == Section::Exponent)
          {
            std::println(stderr, "Multiple exponents are not allowed");
            return Token::Knd::Invalid;
          }

          knd = Token::Knd::FloatLit;
          section = Section::Exponent;
          section_size = 0;
          continue;
        }

        if (c == '+' || c == '-')
        {
          if (lexeme[i - 1] != 'p' && lexeme[i - 1] != 'P')
          {
            std::println(stderr, "+/- are allowed only after an exponent indicator");
            return Token::Knd::Invalid;
          }

          continue;
        }

        if ((section == Section::Exponent) ? digit(c) : xdigit(c))
        {
          std::println(stderr, "Invalid digit in hex literal: {}", c);
          return Token::Knd::Invalid;
        }

        section_size++;
      }

      if (section_size == 0)
      {
        std::println(stderr, "Empty sections are not allowed");
        return Token::Knd::Invalid;
      }

      return knd;
    }
    Token::Knd scan_decimal(std::string lexeme)
    {
      enum class Section {
        Integer,
        Fraction,
        Exponent
      } section = Section::Integer;

      size_t section_size = 0;
      size_t last_sep = 0;
      Token::Knd knd = Token::Knd::IntLit;

      for (size_t i = 0; i < lexeme.length(); ++i)
      {
        char c = lexeme[i];
        if (c == '\'')
        {
          if (section_size == 0)
          {
            std::println(stderr, "Separators can't appear at the start of a section");
            return Token::Knd::Invalid;
          }

          if (last_sep == i - 1)
          {
            std::println(stderr, "Double Separators are not allowed");
            return Token::Knd::Invalid;
          }

          last_sep = i;
          continue;
        }

        if (c == '.')
        {
          if (section != Section::Integer)
          {
            std::println(stderr, "Fraction must be after a valid integer section");
            return Token::Knd::Invalid;
          }

          if (section_size == 0)
          {
            std::println(stderr, "Empty sections are not allowed");
            return Token::Knd::Invalid;
          }

          knd = Token::Knd::FloatLit;
          section = Section::Fraction;
          section_size = 0;
          continue;
        }

        if (c == 'e' || c == 'E')
        {
          if (section == Section::Exponent)
          {
            std::println(stderr, "Multiple exponents are not allowed");
            return Token::Knd::Invalid;
          }

          knd = Token::Knd::FloatLit;
          section = Section::Exponent;
          section_size = 0;
          continue;
        }

        if (c == '+' || c == '-')
        {
          if (lexeme[i - 1] != 'e' && lexeme[i - 1] != 'E')
          {
            std::println(stderr, "+/- are allowed only after an exponent indicator");
            return Token::Knd::Invalid;
          }

          continue;
        }

        if (c < '0' && c > '9')
        {
          std::println(stderr, "Invalid digit in decimal literal: {}", c);
          return Token::Knd::Invalid;
        }

        section_size++;
      }

      if (section_size == 0)
      {
        std::println(stderr, "Empty sections are not allowed");
        return Token::Knd::Invalid;
      }

      return knd;
    }
    Token::Knd scan_number(std::string lexeme)
    {
      size_t base = number_base(lexeme);

      switch (base) {
        case 2:  return scan_binary(lexeme);
        case 8:  return scan_octal(lexeme);
        case 16: return scan_hex(lexeme);
        case 10: return scan_decimal(lexeme);
      }

      std::unreachable();
    }

    std::vector<Token> lex(std::string source)
    {
      std::vector<Token> tkns;
      src = source;
      index = 0;

      while (peek() != '\0')
      {
        if (peek() == '\0')
        {
          tkns.push_back({ Token::Knd::EndOfFile, ""});
          break;
        }

        // TODO: use the new line separeted check to track the current
        // line and the location of each token
        if (space(peek()) || peek() == '\n')
        {
          advance(); continue;
        }

        if (peek() == '/' && peek(1) == '/')
        {
          while (peek() != '\n')
            advance();

          advance(); continue;
        }

        if (peek() == '/' && peek(1) == '*')
        {
          while (peek() != '*' || peek(1) != '/')
            advance();

          advance(2); continue;
        }

        // identifier/keyword/primitive datatype start
        if (peek() == '_' || alpha(peek()))
        {
          std::string lexeme;

          while (peek() == '_' || alnum(peek()))
            lexeme += advance();

          Token tkn = { Token::Knd::Invalid, "" };

          // primitive datatype
          for (auto elm : PrimDataTys)
          {
            if (elm == lexeme)
            {
              tkn = { Token::Knd::DataType, lexeme };
              break;
            }
          }

          if (tkn.knd != Token::Knd::Invalid)
          {
            tkns.push_back(tkn);
            continue;
          }

          // keywords.
          for (auto [keyword, knd] : Keywords)
          {
            if (keyword == lexeme)
            {
              // we don't need a lexeme
              tkn = { knd, "" };
              break;
            }
          }

          if (tkn.knd != Token::Knd::Invalid)
          {
            tkns.push_back(tkn);
            continue;
          }

          tkns.push_back({Token::Knd::Identifier, lexeme});
          continue;
        }

        if (digit(peek()))
        {
          std::string lexeme;

          while (alnum(peek()) || peek() == '.' || peek() == '+' || peek() == '-')
            lexeme += advance();

          Token tkn;
          tkn.form = lexeme;
          tkn.knd = scan_number(lexeme);

          tkns.push_back(tkn);
          continue;
        }

        Token tkn = { Token::Knd::Invalid, "" };
        for (auto [punct, knd] : Puncts)
        {
          if (src.substr(index).starts_with(punct))
          {
            // we don't need a lexeme here too
            tkn = { knd, "" };
            advance(punct.size());
            break;
          }
        }

        if (tkn.knd != Token::Knd::Invalid)
        {
          tkns.push_back(tkn);
          continue;
        }

        // idk
        std::println(stderr, "What is that {}?\n", peek());
      }

      // ensure that it ends with a flag
      if (tkns.back().knd != Token::Knd::EndOfFile)
        tkns.push_back({ Token::Knd::EndOfFile, ""});

      return tkns;
    }
    const char* kndts(Token::Knd knd)
    {
      switch (knd) {
        case Token::Knd::DataType:     return "DataType";
        case Token::Knd::Identifier:   return "Identifier";
        case Token::Knd::Return:       return "Return";
        case Token::Knd::Let:          return "Let";
        case Token::Knd::Fn:           return "Fn";
        case Token::Knd::If:           return "If";
        case Token::Knd::Else:         return "Else";
        case Token::Knd::While:        return "While";
        case Token::Knd::For:          return "For";
        case Token::Knd::Colon:        return "Colon";
        case Token::Knd::SemiColon:    return "SemiColon";
        case Token::Knd::Comma:        return "Comma";
        case Token::Knd::OpenCurly:    return "OpenCurly";
        case Token::Knd::CloseCurly:   return "CloseCurly";
        case Token::Knd::OpenParent:   return "OpenParent";
        case Token::Knd::CloseParent:  return "CloseParent";
        case Token::Knd::OpenBracket:  return "OpenBracket";
        case Token::Knd::CloseBracket: return "CloseBracket";
        case Token::Knd::Not:          return "Not";
        case Token::Knd::Mul:          return "Mul";
        case Token::Knd::Div:          return "Div";
        case Token::Knd::Mod:          return "Mod";
        case Token::Knd::And:          return "And";
        case Token::Knd::Or:           return "Or";
        case Token::Knd::Plus:         return "Plus";
        case Token::Knd::Minus:        return "Minus";
        case Token::Knd::Inc:          return "Inc";
        case Token::Knd::Dec:          return "Dec";
        case Token::Knd::Less:         return "Less";
        case Token::Knd::LessEq:       return "LessEq";
        case Token::Knd::Greater:      return "Greater";
        case Token::Knd::GreaterEq:    return "GreaterEq";
        case Token::Knd::Eq:           return "Eq";
        case Token::Knd::EqEq:         return "EqEq";
        case Token::Knd::NotEq:        return "NotEq";
        case Token::Knd::Shl:          return "Shl";
        case Token::Knd::ShlEq:        return "ShlEq";
        case Token::Knd::Shr:          return "Shr";
        case Token::Knd::ShrEq:        return "ShrEq";
        case Token::Knd::ModEq:        return "ModEq";
        case Token::Knd::OrEq:         return "OrEq";
        case Token::Knd::AndEq:        return "AndEq";
        case Token::Knd::PlusEq:       return "PlusEq";
        case Token::Knd::MinusEq:      return "MinusEq";
        case Token::Knd::MulEq:        return "MulEq";
        case Token::Knd::DivEq:        return "DivEq";
        case Token::Knd::Qst:          return "Qst";
        case Token::Knd::RightArrow:   return "RightArrow";
        case Token::Knd::IntLit:       return "IntLit";
        case Token::Knd::FloatLit:     return "FloatLit";
        case Token::Knd::CharLit:      return "CharLit";
        case Token::Knd::StrLit:       return "StrLit";
        case Token::Knd::ArrLit:       return "ArrLit";
        case Token::Knd::EndOfFile:    return "EndOfFile";
        case Token::Knd::Invalid:      return "Invalid";
        default:                       return "Unknown";
      }
    }
    void print_tokens(std::vector<Token> tkns)
    { 
      for (auto tkn : tkns)
      {
        std::print("[Token: {}", kndts(tkn.knd));

        if (!tkn.form.empty())
          std::print(", Form: {}", tkn.form);

        std::println("]");
      }
    }
  }
}
