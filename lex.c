#include "lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

token_t new_token(token_kind_t kind, loc pos, const char *text) {
  token_t tok = {0};

  tok.kind = kind;
  tok.line = pos.ln;
  tok.column = pos.col;
  tok.text = calloc(strlen(text) + 1, 1);
  strcpy(tok.text, text);

  return tok;
}

token_t lex_next(file *f) {
  loc pos;
  char c = next_char(f, &pos);

  for (;; c = next_char(f, &pos)) {
    switch (c) {
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

    if (c == '=') {
      if (peek_char(f) == '=') {
        next_char(f, NULL);
        return new_token(Eq, pos, "==");
      } else {
        return new_token(Assn, pos, "=");
      }
    }

    if (c == '+') {
      char c_peek = peek_char(f);
      if (c_peek == '+') {
        next_char(f, NULL);
        return new_token(PlusPlus, pos, "++");
      } else if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(PlusAssn, pos, "+=");
      } else {
        return new_token(Plus, pos, "+");
      }
    }

    if (c == '-') {
      char c_peek = peek_char(f);
      if (c_peek == '-') {
        next_char(f, NULL);
        return new_token(MinusMinus, pos, "--");
      } else if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(MinusAssn, pos, "-=");
      } else if (c_peek == '>') {
        next_char(f, NULL);
        return new_token(Arrow, pos, "->");
      } else {
        return new_token(Minus, pos, "-");
      }
    }

    if (c == '*') {
      char c_peek = peek_char(f);
      if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(StarAssn, pos, "*=");
      } else {
        return new_token(Star, pos, "*");
      }
    }

    if (c == '/') {
      char c_peek = peek_char(f);
      if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(SlashAssn, pos, "/=");
      } else if (c_peek == '/') {
        /* comment, skip until lf */
        c = next_char(f, &pos);
        for (; c != '\n';) {
          c = next_char(f, &pos);
        }
      } else if (c_peek == '*') {
        /* block comment, skip until match */
        c = next_char(f, &pos);
        for (; c != '*' || peek_char(f) != '/';) {
          c = next_char(f, &pos);
        }
        next_char(f, &pos);
      } else {
        return new_token(Slash, pos, "/");
      }
    }

    if (c == '%') {
      char c_peek = peek_char(f);
      if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(PercentAssn, pos, "%=");
      } else {
        return new_token(Percent, pos, "%");
      }
    }

    if (c == '&') {
      char c_peek = peek_char(f);
      if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(AmpAssn, pos, "&=");
      } else if (c_peek == '&') {
        next_char(f, NULL);
        return new_token(AmpAmp, pos, "&&");
      } else {
        return new_token(Amp, pos, "&");
      }
    }

    if (c == '|') {
      char c_peek = peek_char(f);
      if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(BarAssn, pos, "|=");
      } else if (c_peek == '|') {
        next_char(f, NULL);
        return new_token(BarBar, pos, "||");
      } else {
        return new_token(Bar, pos, "|");
      }
    }

    if (c == '^') {
      char c_peek = peek_char(f);
      if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(CaretAssn, pos, "^=");
      } else {
        return new_token(Caret, pos, "^");
      }
    }

    if (c == '<') {
      char c_peek = peek_char(f);
      if (c_peek == '<') {
        c = next_char(f, NULL);
        c_peek = peek_char(f);
        if (c_peek == '=') {
          next_char(f, NULL);
          return new_token(LShftAssn, pos, "<<=");
        } else {
          return new_token(LShft, pos, "<<");
        }
      } else if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(Leq, pos, "<=");
      } else {
        return new_token(Lt, pos, "<");
      }
    }

    if (c == '>') {
      char c_peek = peek_char(f);
      if (c_peek == '>') {
        c = next_char(f, NULL);
        c_peek = peek_char(f);
        if (c_peek == '=') {
          next_char(f, NULL);
          return new_token(RShftAssn, pos, ">>=");
        } else {
          return new_token(RShft, pos, ">>");
        }
      } else if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(Geq, pos, ">=");
      } else {
        return new_token(Gt, pos, ">");
      }
    }

    if (c == '!') {
      char c_peek = peek_char(f);
      if (c_peek == '=') {
        next_char(f, NULL);
        return new_token(Neq, pos, "!=");
      } else {
        return new_token(Exclaim, pos, "!");
      }
    }

    if (c == '"') {
      char text[1024] = {0};
      int i;
      text[0] = '"';
      c = next_char(f, NULL);
      for (i = 1; c != '"'; i++) {
        if (c == '\n') {
          printf("malformed string literal at %d:%d", pos.ln, pos.col);
          exit(1);
        }
        text[i] = c;
        c = next_char(f, NULL);
        if (text[i] == '\\' && c == '"') {
          /* escaped quote */
          text[++i] = c;
          c = next_char(f, NULL);
        }
      }
      text[i] = '"';

      return new_token(String, pos, text);
    }

    if (c == '#') {
      char text[32] = {0};
      int i = 0;
      char c_peek = peek_char(f);

      if (!f->lines[pos.ln - 1].cpp) {
        puts("cpp directive must be on its own line");
        exit(1);
      }

      text[i++] = c;
      for (; 'a' <= c_peek && c_peek <= 'z'; c_peek = peek_char(f)) {
        text[i++] = next_char(f, NULL);
      }

      return new_token(Directive, pos, text);
    }

    if (c == '\'') {
      char text[16] = {0};
      int i = 0;

      text[0] = '\'';
      c = next_char(f, NULL);
      for (i = 1; c != '\''; i++) {
        if (c == '\n') {
          printf("malformed character literal at %d:%d", pos.ln, pos.col);
          exit(1);
        }
        text[i] = c;
        c = next_char(f, NULL);
        if (c == '\\' && c == '\'') {
          /* escaped quote */
          text[++i] = c;
          c = next_char(f, NULL);
        }
      }
      text[i] = '\'';

      return new_token(Character, pos, text);
    }

    if ('0' <= c && c <= '9') {
      char text[32] = {0};
      int i = 0;
      char c_peek = peek_char(f);

      text[i++] = c;
      for (; ('0' <= c_peek && c_peek <= '9'); c_peek = peek_char(f)) {
        text[i++] = next_char(f, NULL);
      }

      return new_token(Number, pos, text);
    }

    if (c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
      char text[32] = {0};
      int i = 0;
      int j = 0;
      char c_peek = peek_char(f);

      text[i++] = c;
      for (;
           c_peek == '_' || ('a' <= c_peek && c_peek <= 'z') ||
           ('A' <= c_peek && c_peek <= 'Z') || ('0' <= c_peek && c_peek <= '9');
           c_peek = peek_char(f)) {
        text[i++] = next_char(f, NULL);
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

struct unit *lex_file(file *f) {
  struct unit *u;

  u = new_unit();

  for (; peek_char(f);) {
    token_t tok = lex_next(f);
    unit_append(u, tok);
  }
  if (u->toks[u->len].kind != Eof) {
    token_t tok = new_token(Eof, f->cur, "");
    unit_append(u, tok);
  }

  return u;
}

struct unit *new_unit(void) {
  struct unit *u;
  u = calloc(1, sizeof(*u));
  u->cap = 128;
  u->len = 0;
  u->toks = calloc(u->cap, sizeof(*u->toks));
  return u;
}

void unit_append(struct unit *u, token_t tok) {
  if (u->len >= u->cap) {
    u->cap *= 3;
    u->toks = realloc(u->toks, sizeof(*u->toks) * u->cap);
  }
  u->toks[u->len++] = tok;
}

token_t *unit_at(struct unit *u, int i) {
  return u->toks + i;
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
    "Eof"};

const char *keywords[KEYWORDS] = {
    "auto",     "break",    "case",     "char",   "const",   "continue",
    "default",  "do",       "double",   "else",   "enum",    "extern",
    "float",    "for",      "goto",     "if",     "inline",  "int",
    "long",     "register", "restrict", "return", "short",   "signed",
    "sizeof",   "static",   "struct",   "switch", "typedef", "union",
    "unsigned", "void",     "volatile", "while",
};
