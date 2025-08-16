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

namespace lexer {
  char peek();
  char advance();
  bool match(char c);

  std::vector<Token> lex(std::string src);
}
