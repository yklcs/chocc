#ifndef CHOCC_TOKEN_H
#define CHOCC_TOKEN_H
#pragma once

#include <stddef.h>

typedef enum {
  /* literals */
  Number,
  String,

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
  Hash, /* # */

  /* internal */
  Eof
} token_kind_t;

static const char *token_kind_map[] = {[Number] = "Number",
                                       [String] = "String",
                                       [LBrace] = "LBrace",
                                       [RBrace] = "RBrace",
                                       [LBrack] = "LBrack",
                                       [RBrack] = "RBrack",
                                       [LParen] = "LParen",
                                       [RParen] = "RParen",
                                       [Comma] = "Comma",
                                       [Semi] = "Semi",
                                       [Assn] = "Assn",
                                       [PlusAssn] = "PlusAssn",
                                       [MinusAssn] = "MinusAssn",
                                       [StarAssn] = "StarAssn",
                                       [SlashAssn] = "SlashAssn",
                                       [PercentAssn] = "PercentAssn",
                                       [AmpAssn] = "AmpAssn",
                                       [BarAssn] = "BarAssn",
                                       [CaretAssn] = "CaretAssn",
                                       [LShftAssn] = "LShftAssn",
                                       [RShftAssn] = "RShftAssn",
                                       [PlusPlus] = "PlusPlus",
                                       [MinusMinus] = "MinusMinus",
                                       [Plus] = "Plus",
                                       [Minus] = "Minus",
                                       [Star] = "Star",
                                       [Slash] = "Slash",
                                       [Percent] = "Percent",
                                       [Tilde] = "Tilde",
                                       [Amp] = "Amp",
                                       [Bar] = "Bar",
                                       [Caret] = "Caret",
                                       [LShft] = "LShft",
                                       [RShft] = "RShft",
                                       [Exclaim] = "Exclaim",
                                       [AmpAmp] = "AmpAmp",
                                       [BarBar] = "BarBar",
                                       [Question] = "Question",
                                       [Colon] = "Colon",
                                       [Eq] = "Eq",
                                       [Neq] = "Neq",
                                       [Lt] = "Lt",
                                       [Gt] = "Gt",
                                       [Leq] = "Leq",
                                       [Geq] = "Geq",
                                       [Arrow] = "Arrow",
                                       [Dot] = "Dot",
                                       [Id] = "Id",
                                       [Auto] = "Auto",
                                       [Break] = "Break",
                                       [Case] = "Case",
                                       [Char] = "Char",
                                       [Const] = "Const",
                                       [Continue] = "Continue",
                                       [Default] = "Default",
                                       [Do] = "Do",
                                       [Double] = "Double",
                                       [Else] = "Else",
                                       [Enum] = "Enum",
                                       [Extern] = "Extern",
                                       [Float] = "Float",
                                       [For] = "For",
                                       [Goto] = "Goto",
                                       [If] = "If",
                                       [Inline] = "Inline",
                                       [Int] = "Int",
                                       [Long] = "Long",
                                       [Register] = "Register",
                                       [Restrict] = "Restrict",
                                       [Return] = "Return",
                                       [Short] = "Short",
                                       [Signed] = "Signed",
                                       [Sizeof] = "Sizeof",
                                       [Static] = "Static",
                                       [Struct] = "Struct",
                                       [Switch] = "Switch",
                                       [Typedef] = "Typedef",
                                       [Union] = "Union",
                                       [Unsigned] = "Unsigned",
                                       [Void] = "Void",
                                       [Volatile] = "Volatile",
                                       [While] = "While",
                                       [Hash] = "Hash",
                                       [Eof] = "Eof"};

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

typedef struct {
  token_t *tokens;
  int len;
  int cap;
} tokens_t;

void skip(char **pos, unsigned int *column, int n);
void add_token(char **pos, tokens_t *tokens, token_kind_t token_kind,
               const char *token_text, unsigned int *token_line,
               unsigned int *token_column);
char *next_token(char *next, tokens_t *tokens, unsigned int *line,
                 unsigned int *column);
tokens_t lex(char *src);
char *format_token(char *buffer, size_t len, token_t token);

#endif
