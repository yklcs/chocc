#ifndef CHOCC_LEX_H
#define CHOCC_LEX_H
#pragma once

#include "chocc.h"
#include "io.h"

typedef enum {
  /* literals */
  Number,
  String,
  Character,

  /* delimiters */
  LBrace, /* { */
  RBrace, /* } */
  LBrack, /* [ */
  RBrack, /* ] */
  LParen, /* ( */
  RParen, /* ) */
  Comma,  /* , */
  Semi,   /* ; */

  /* assignment */
  Assn,        /* =*/
  PlusAssn,    /* += */
  MinusAssn,   /* -= */
  StarAssn,    /* *= */
  SlashAssn,   /* /= */
  PercentAssn, /* %= */
  AmpAssn,     /* &= */
  BarAssn,     /* |= */
  CaretAssn,   /* ^= */
  LShftAssn,   /* <<= */
  RShftAssn,   /* >>= */

  /* {inc,dec}rement */
  PlusPlus,   /* ++ */
  MinusMinus, /* -- */

  /* arithmetic */
  Plus,     /* + */
  Minus,    /* - */
  Star,     /* * */
  Slash,    /* / */
  Percent,  /* % */
  Tilde,    /* ~ */
  Amp,      /* & */
  Bar,      /* | */
  Caret,    /* ^ */
  LShft,    /* << */
  RShft,    /* >> */
  Exclaim,  /* ! */
  AmpAmp,   /* && */
  BarBar,   /* || */
  Question, /* ? */
  Colon,    /* : */

  /* comparison */
  Eq,  /* == */
  Neq, /* != */
  Lt,  /* < */
  Gt,  /* > */
  Leq, /* <= */
  Geq, /* >= */

  /* access */
  Arrow, /* -> */
  Dot,   /* . */

  /* identifiers */
  Id, /* [a-zA-Z_][a-zA-Z0-9_]* */

  /* keywords */
  Auto,
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

  /* preprocessor */
  Directive,

  /* internal */
  Lf,
  Eof
} token_kind_t;

static const char *token_kind_map[] = {
    "Number",    "String",      "Character", "LBrace",     "RBrace",
    "LBrack",    "RBrack",      "LParen",    "RParen",     "Comma",
    "Semi",      "Assn",        "PlusAssn",  "MinusAssn",  "StarAssn",
    "SlashAssn", "PercentAssn", "AmpAssn",   "BarAssn",    "CaretAssn",
    "LShftAssn", "RShftAssn",   "PlusPlus",  "MinusMinus", "Plus",
    "Minus",     "Star",        "Slash",     "Percent",    "Tilde",
    "Amp",       "Bar",         "Caret",     "LShft",      "RShft",
    "Exclaim",   "AmpAmp",      "BarBar",    "Question",   "Colon",
    "Eq",        "Neq",         "Lt",        "Gt",         "Leq",
    "Geq",       "Arrow",       "Dot",       "Id",         "Auto",
    "Break",     "Case",        "Char",      "Const",      "Continue",
    "Default",   "Do",          "Double",    "Else",       "Enum",
    "Extern",    "Float",       "For",       "Goto",       "If",
    "Inline",    "Int",         "Long",      "Register",   "Restrict",
    "Return",    "Short",       "Signed",    "Sizeof",     "Static",
    "Struct",    "Switch",      "Typedef",   "Union",      "Unsigned",
    "Void",      "Volatile",    "While",     "Directive",  "Lf",
    "Eof"};

#define KEYWORDS 34
static const char *keywords[KEYWORDS] = {
    "auto",     "break",    "case",     "char",   "const",   "continue",
    "default",  "do",       "double",   "else",   "enum",    "extern",
    "float",    "for",      "goto",     "if",     "inline",  "int",
    "long",     "register", "restrict", "return", "short",   "signed",
    "sizeof",   "static",   "struct",   "switch", "typedef", "union",
    "unsigned", "void",     "volatile", "while",
};

typedef struct {
  token_kind_t kind;
  unsigned int line;
  unsigned int column;
  char text[32];
} token_t;

void print_token(token_t token);

/*
 * Lexes and returns the next token in the given file.
 */
token_t lex_next(file *f);

/*
 * Lexes the entire file.
 */
int lex_file(file *f, token_t **toks);

#endif
