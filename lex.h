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

extern const char *token_kind_map[Eof + 1];

#define KEYWORDS 34
extern const char *keywords[KEYWORDS];

typedef struct {
  token_kind_t kind;
  unsigned int line;
  unsigned int column;
  char *text;
} token_t;

token_t new_token(token_kind_t kind, loc pos, const char *text);

void print_token(token_t token);

/* unit represents a translation unit (a vec of tokens) */
struct unit {
  token_t *toks;
  int len;
  int cap;
};

struct unit *new_unit(void);
void unit_append(struct unit *u, token_t tok);
token_t *unit_at(struct unit *u, int i);

/*
 * Lexes and returns the next token in the given file.
 */
token_t lex_next(file *f);

/*
 * Lexes the entire file.
 */
struct unit *lex_file(file *f);

#endif
