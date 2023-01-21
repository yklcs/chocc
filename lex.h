#ifndef CHOCC_LEX_H
#define CHOCC_LEX_H
#pragma once

#include "dynarrsaur.h"

typedef enum {
  // literals
  Number,
  String,

  // delimiters
  LBrace,  // {
  RBrace,  // }
  LSquare, // [
  RSquare, // ]
  LParen,  // (
  RParen,  // )
  Comma,   // ,
  Semi,    // ;

  // assignment
  Assn,        // =
  PlusAssn,    // +=
  MinusAssn,   // -=
  StarAssn,    // *=
  SlashAssn,   // /=
  PercentAssn, // %=
  AmpAssn,     // &=
  BarAssn,     // |=
  CaretAssn,   // ^=
  LShftAssn,   // <<=
  RShftAssn,   // >>=

  // {inc,dec}rement
  PlusPlus,   // ++
  MinusMinus, // --

  // arithmetic
  Plus,     // +
  Minus,    // -
  Star,     // *
  Slash,    // /
  Percent,  // %
  Tilde,    // ~
  Amp,      // &
  Bar,      // |
  Caret,    // ^
  LShft,    // <<
  RShft,    // >>
  Exclaim,  // !
  AmpAmp,   // &&
  BarBar,   // ||
  Question, // ?
  Colon,    // :

  // comparison
  Eq,  // ==
  Neq, // !=
  Lt,  // <
  Gt,  // >
  Leq, // <=
  Geq, // >=

  // access
  Arrow, // ->
  Dot,   //.

  // identifiers
  Id, // /[a-zA-Z_][a-zA-Z0-9_]*/

  // keywords
  Auto, // 48
  Break,
  Case,
  Char,
  Const,
  Continue,
  Default,
  Do,
  Double,
  Else,
  Enum,
  Extern,
  Float,
  For,
  Goto,
  If,
  Inline,
  Int,
  Long,
  Register,
  Restrict,
  Return,
  Short,
  Signed,
  Sizeof,
  Static,
  Struct,
  Switch,
  Typedef,
  Union,
  Unsigned,
  Void,
  Volatile,
  While,
  Bool,      // _Bool
  Complex,   // _Complex
  Imaginary, // _Imaginary

  // preprocessor
  Hash, // #

  // internal
  Eof
} token_kind_t;

static const char *token_kind_map[] = {
    "Number",      "String",   "LBrace",     "RBrace",    "LSquare",
    "RSquare",     "LParen",   "RParen",     "Comma",     "Semi",
    "Assn",        "PlusAssn", "MinusAssn",  "StarAssn",  "SlashAssn",
    "PercentAssn", "AmpAssn",  "BarAssn",    "CaretAssn", "LShftAssn",
    "RShftAssn",   "PlusPlus", "MinusMinus", "Plus",      "Minus",
    "Star",        "Slash",    "Percent",    "Tilde",     "Amp",
    "Bar",         "Caret",    "LShft",      "RShft",     "Exclaim",
    "AmpAmp",      "BarBar",   "Question",   "Colon",     "Eq",
    "Neq",         "Lt",       "Gt",         "Leq",       "Geq",
    "Arrow",       "Dot",      "Id",         "Auto",      "Break",
    "Case",        "Char",     "Const",      "Continue",  "Default",
    "Do",          "Double",   "Else",       "Enum",      "Extern",
    "Float",       "For",      "Goto",       "If",        "Inline",
    "Int",         "Long",     "Register",   "Restrict",  "Return",
    "Short",       "Signed",   "Sizeof",     "Static",    "Struct",
    "Switch",      "Typedef",  "Union",      "Unsigned",  "Void",
    "Volatile",    "While",    "Bool",       "Complex",   "Imaginary",
    "Hash",        "Eof"};

static const char *keywords[37] = {
    "auto",       "break",    "case",     "char",   "const",   "continue",
    "default",    "do",       "double",   "else",   "enum",    "extern",
    "float",      "for",      "goto",     "if",     "inline",  "int",
    "long",       "register", "restrict", "return", "short",   "signed",
    "sizeof",     "static",   "struct",   "switch", "typedef", "union",
    "unsigned",   "void",     "volatile", "while",  "_Bool",   "_Complex",
    "_Imaginary",
};

typedef struct {
  token_kind_t kind;
  unsigned int line;
  unsigned int column;
  char text[32];
} token_t;

typedef dynarr_t(token_t) token_dynarr_t;

void skip(char **pos, unsigned int *column, int n);
void add_token(char **pos, token_dynarr_t *tokens, token_kind_t token_kind,
               const char *token_text, unsigned int *token_line,
               unsigned int *token_column);
char *next_token(char *next, token_dynarr_t *tokens, unsigned int *line,
                 unsigned int *column);
token_dynarr_t lex(char *src);
char *format_token(char *buffer, size_t len, token_t token);

#endif
