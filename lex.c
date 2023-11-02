#include "lex.h"
#include "error.h"
#include "io.h"
#include "unit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void lexer_advance(struct lexer *l) {
  line ln;
  file *f = l->unit->file;

  if (l->pos.ln > f->lines_len ||
      (l->pos.ln == f->lines_len &&
       f->lines[f->lines_len - 1].len < l->pos.col)) {
    l->c = 0;
    return;
  }

  ln = f->lines[l->pos.ln - 1];

  if (l->pos.col > ln.len) {
    /* overflow to next line */
    l->pos.col = 1;
    l->pos.ln++;
    l->c = '\n';
  } else if (l->pos.col == ln.len && ln.splice) {
    /* splicing, return characters after splicing but maintain cursor */
    ln = f->lines[++l->pos.ln - 1];
    l->pos.col = 1;
    l->c = ln.src[l->pos.col++ - 1];
  } else {
    l->c = ln.src[l->pos.col++ - 1];
  }
}

char lexer_peek(struct lexer *l) {
  struct lexer l_fake = {0};
  l_fake.unit = l->unit;
  l_fake.pos = l->pos;
  lexer_advance(&l_fake);
  return l_fake.c;
}

token_t new_token(token_kind_t kind, loc pos, const char *text) {
  token_t tok = {0};

  tok.kind = kind;
  tok.line = pos.ln;
  tok.column = pos.col;
  tok.text = calloc(strlen(text) + 1, 1);
  strcpy(tok.text, text);

  return tok;
}

token_t lex_next(struct lexer *l) {
  loc pos;

  for (;;) {
    pos = l->pos;
    lexer_advance(l);
    switch (l->c) {
    case '\0': {
      return new_token(Eof, pos, "");
    }
    case '\n': {
      return new_token(Lf, pos, "\\n");
    }
    case '{': {
      return new_token(LBrace, pos, "{");
    }
    case '}': {
      return new_token(RBrace, pos, "}");
    }
    case '[': {
      return new_token(LBrack, pos, "[");
    }
    case ']': {
      return new_token(RBrack, pos, "]");
    }
    case '(': {
      return new_token(LParen, pos, "(");
    }
    case ')': {
      return new_token(RParen, pos, ")");
    }
    case '.': {
      return new_token(Dot, pos, ".");
    }
    case ',': {
      return new_token(Comma, pos, ",");
    }
    case ';': {
      return new_token(Semi, pos, ";");
    }
    case '~': {
      return new_token(Tilde, pos, "~");
    }
    case '?': {
      return new_token(Question, pos, "?");
    }
    case ':': {
      return new_token(Colon, pos, ":");
    }
    }

    if (l->c == '=') {
      if (lexer_peek(l) == '=') {
        lexer_advance(l);
        return new_token(Eq, pos, "==");
      } else {
        return new_token(Assn, pos, "=");
      }
    }

    if (l->c == '+') {
      char c_peek = lexer_peek(l);
      if (c_peek == '+') {
        lexer_advance(l);
        return new_token(PlusPlus, pos, "++");
      } else if (c_peek == '=') {
        lexer_advance(l);
        return new_token(PlusAssn, pos, "+=");
      } else {
        return new_token(Plus, pos, "+");
      }
    }

    if (l->c == '-') {
      char c_peek = lexer_peek(l);
      if (c_peek == '-') {
        lexer_advance(l);
        return new_token(MinusMinus, pos, "--");
      } else if (c_peek == '=') {
        lexer_advance(l);
        return new_token(MinusAssn, pos, "-=");
      } else if (c_peek == '>') {
        lexer_advance(l);
        return new_token(Arrow, pos, "->");
      } else {
        return new_token(Minus, pos, "-");
      }
    }

    if (l->c == '*') {
      char c_peek = lexer_peek(l);
      if (c_peek == '=') {
        lexer_advance(l);
        return new_token(StarAssn, pos, "*=");
      } else {
        return new_token(Star, pos, "*");
      }
    }

    if (l->c == '/') {
      char c_peek = lexer_peek(l);
      if (c_peek == '=') {
        lexer_advance(l);
        return new_token(SlashAssn, pos, "/=");
      } else if (c_peek == '/') {
        /* c++ style comment, skip until lf */
        for (; l->c != '\n';) {
          lexer_advance(l);
        }
      } else if (c_peek == '*') {
        /* block comment, skip until match */
        for (; l->c != '*' || lexer_peek(l) != '/';) {
          lexer_advance(l);
        }
        lexer_advance(l);
      } else {
        return new_token(Slash, pos, "/");
      }
    }

    if (l->c == '%') {
      char c_peek = lexer_peek(l);
      if (c_peek == '=') {
        lexer_advance(l);
        return new_token(PercentAssn, pos, "%=");
      } else {
        return new_token(Percent, pos, "%");
      }
    }

    if (l->c == '&') {
      char c_peek = lexer_peek(l);
      if (c_peek == '=') {
        lexer_advance(l);
        return new_token(AmpAssn, pos, "&=");
      } else if (c_peek == '&') {
        lexer_advance(l);
        return new_token(AmpAmp, pos, "&&");
      } else {
        return new_token(Amp, pos, "&");
      }
    }

    if (l->c == '|') {
      char c_peek = lexer_peek(l);
      if (c_peek == '=') {
        lexer_advance(l);
        return new_token(BarAssn, pos, "|=");
      } else if (c_peek == '|') {
        lexer_advance(l);
        return new_token(BarBar, pos, "||");
      } else {
        return new_token(Bar, pos, "|");
      }
    }

    if (l->c == '^') {
      char c_peek = lexer_peek(l);
      if (c_peek == '=') {
        lexer_advance(l);
        return new_token(CaretAssn, pos, "^=");
      } else {
        return new_token(Caret, pos, "^");
      }
    }

    if (l->c == '<') {
      char c_peek = lexer_peek(l);
      if (c_peek == '<') {
        lexer_advance(l);
        c_peek = lexer_peek(l);
        if (c_peek == '=') {
          lexer_advance(l);
          return new_token(LShftAssn, pos, "<<=");
        } else {
          return new_token(LShft, pos, "<<");
        }
      } else if (c_peek == '=') {
        lexer_advance(l);
        return new_token(Leq, pos, "<=");
      } else {
        return new_token(Lt, pos, "<");
      }
    }

    if (l->c == '>') {
      char c_peek = lexer_peek(l);
      if (c_peek == '>') {
        lexer_advance(l);
        c_peek = lexer_peek(l);
        if (c_peek == '=') {
          lexer_advance(l);
          return new_token(RShftAssn, pos, ">>=");
        } else {
          return new_token(RShft, pos, ">>");
        }
      } else if (c_peek == '=') {
        lexer_advance(l);
        return new_token(Geq, pos, ">=");
      } else {
        return new_token(Gt, pos, ">");
      }
    }

    if (l->c == '!') {
      char c_peek = lexer_peek(l);
      if (c_peek == '=') {
        lexer_advance(l);
        return new_token(Neq, pos, "!=");
      } else {
        return new_token(Exclaim, pos, "!");
      }
    }

    if (l->c == '"') {
      char text[1024] = {0};
      int i;
      text[0] = '"';
      lexer_advance(l);
      for (i = 1; l->c != '"'; i++) {
        if (l->c == '\n') {
          l->unit->err = new_error(LexErr, "malformed string literal", pos);
          return new_token(Nil, pos, "");
        }
        text[i] = l->c;
        lexer_advance(l);
        if (text[i] == '\\' && l->c == '"') {
          /* escaped quote */
          text[++i] = l->c;
          lexer_advance(l);
        }
      }
      text[i] = '"';

      return new_token(String, pos, text);
    }

    if (l->c == '#') {
      char text[32] = {0};
      int i = 0;
      char c_peek = lexer_peek(l);

      if (!l->unit->file->lines[pos.ln - 1].cpp) {
        l->unit->err =
            new_error(LexErr, "cpp directive must be on its own line", pos);
        return new_token(Nil, pos, "");
      }

      text[i++] = l->c;
      for (; 'a' <= c_peek && c_peek <= 'z'; c_peek = lexer_peek(l)) {
        lexer_advance(l);
        text[i++] = l->c;
      }

      return new_token(Directive, pos, text);
    }

    if (l->c == '\'') {
      char text[16] = {0};
      int i = 0;

      text[0] = '\'';
      lexer_advance(l);
      for (i = 1; l->c != '\''; i++) {
        if (l->c == '\n') {
          l->unit->err = new_error(LexErr, "malformed character literal", pos);
          return new_token(Nil, pos, "");
        }
        text[i] = l->c;
        lexer_advance(l);
        if (l->c == '\\' && l->c == '\'') {
          /* escaped quote */
          text[++i] = l->c;
          lexer_advance(l);
        }
      }
      text[i] = '\'';

      return new_token(Character, pos, text);
    }

    if ('0' <= l->c && l->c <= '9') {
      char text[32] = {0};
      int i = 0;
      char c_peek = lexer_peek(l);

      text[i++] = l->c;
      for (; ('0' <= c_peek && c_peek <= '9'); c_peek = lexer_peek(l)) {
        lexer_advance(l);
        text[i++] = l->c;
      }

      return new_token(Number, pos, text);
    }

    if (l->c == '_' || ('a' <= l->c && l->c <= 'z') ||
        ('A' <= l->c && l->c <= 'Z')) {
      char text[32] = {0};
      int i = 0;
      int j = 0;
      char c_peek = lexer_peek(l);

      text[i++] = l->c;
      for (;
           c_peek == '_' || ('a' <= c_peek && c_peek <= 'z') ||
           ('A' <= c_peek && c_peek <= 'Z') || ('0' <= c_peek && c_peek <= '9');
           c_peek = lexer_peek(l)) {
        lexer_advance(l);
        text[i++] = l->c;
      }

      /* check if keyword */
      for (j = 0; j < KEYWORDS; j++) {
        if (strcmp(keywords[j], text) == 0) {
          return new_token(49 + j, pos, keywords[j]);
        }
      }

      return new_token(Id, pos, text);
    }
  }
}

struct lexer new_lexer(struct unit *u) {
  struct lexer l = {0};
  l.unit = u;
  l.pos.col = 1;
  l.pos.ln = 1;

  return l;
}

void lex(struct unit *u) {
  struct lexer l;

  l = new_lexer(u);

  for (; lexer_peek(&l);) {
    token_t tok = lex_next(&l);
    if (u->err != NULL) {
      return;
    }
    unit_append_tok(u, tok);
  }
  if (u->toks[u->toks_len].kind != Eof) {
    token_t tok = new_token(Eof, l.pos, "");
    unit_append_tok(u, tok);
  }
}

void print_token(token_t tok) {
  printf("%s\t%s\t%d:%d\n", tok.text, token_kind_map[tok.kind], tok.line,
         tok.column);
}

const char *token_kind_map[] = {
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
    "Eof",       "Nil"};

const char *keywords[KEYWORDS] = {
    "auto",     "break",    "case",     "char",   "const",   "continue",
    "default",  "do",       "double",   "else",   "enum",    "extern",
    "float",    "for",      "goto",     "if",     "inline",  "int",
    "long",     "register", "restrict", "return", "short",   "signed",
    "sizeof",   "static",   "struct",   "switch", "typedef", "union",
    "unsigned", "void",     "volatile", "while",
};
