#pragma once

#include "stl.h"

struct Token {
  enum class Knd {
    DataType,
    Identifier,

    // keywords
    Return,
    Let,
    Fn,
    If,
    Else,
    While,
    For,

    // ponctuation
    Colon,
    SemiColon,
    Comma,
    OpenCurly,
    CloseCurly,
    OpenParent,
    CloseParent,
    OpenBracket,
    CloseBracket,
    Not,
    Mul,
    Div,
    Mod,
    And,
    Or,
    Plus,
    Minus,
    Inc,
    Dec,
    Less,
    LessEq,
    Greater,
    GreaterEq,
    Eq,
    EqEq,
    NotEq,
    Shl,
    ShlEq,
    Shr,
    ShrEq,
    ModEq,
    OrEq,
    AndEq,
    PlusEq,
    MinusEq,
    MulEq,
    DivEq,
    Qst,
    RightArrow,

    // literals
    IntLit,
    FloatLit,
    CharLit,
    StrLit,
    ArrLit,

    // enf of file flag
    EndOfFile,

    // Invalid Tokens
    Invalid,
  } knd;
  std::string form;
};

namespace soft {
  namespace lexer {
    char peek(off_t offset = 0);
    char advance(off_t offset = 1);
    bool match(char c, off_t offset = 0);

    size_t number_base(const std::string& str);
    std::vector<Token> lex(std::string src);
    const char* kndts(Token::Knd knd);
    void print_tokens(std::vector<Token> tkns);
  }
}
