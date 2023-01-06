#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "dynarrsaur.h"
#include "lex.h"

void skip(char **pos, unsigned int *column, int n) {
  *pos += n;
  *column += n;
}

void add_token(char **pos, token_dynarr_t *tokens, token_kind_t token_kind,
               const char *token_text, unsigned int *token_line,
               unsigned int *token_column) {
  token_t token = {
      .kind = token_kind, .column = *token_column, .line = *token_line};
  strcpy(token.text, token_text);
  dynarr_push(tokens, token);

  int len = strlen(token_text);
  skip(pos, token_column, len);
}

char *next_token(char *pos, token_dynarr_t *tokens, unsigned int *line,
                 unsigned int *column) {
  for (char c = *pos;; c = *pos) {
    switch (c) {
    case '\n': {
      *column = 1;
      (*line)++;
      return pos + 1;
    }
    case '#': {
      add_token(&pos, tokens, Hash, "#", line, column);
      return pos;
    }
    case '{': {
      add_token(&pos, tokens, LBrace, "{", line, column);
      return pos;
    }
    case '}': {
      add_token(&pos, tokens, RBrace, "}", line, column);
      return pos;
    }
    case '[': {
      add_token(&pos, tokens, LSquare, "[", line, column);
      return pos;
    }
    case ']': {
      add_token(&pos, tokens, RSquare, "]", line, column);
      return pos;
    }
    case '(': {
      add_token(&pos, tokens, LParen, "(", line, column);
      return pos;
    }
    case ')': {
      add_token(&pos, tokens, RParen, ")", line, column);
      return pos;
    }
    case '.': {
      add_token(&pos, tokens, Dot, ".", line, column);
      return pos;
    }
    case ',': {
      add_token(&pos, tokens, Comma, ",", line, column);
      return pos;
    }
    case ';': {
      add_token(&pos, tokens, Semi, ";", line, column);
      return pos;
    }
    case '~': {
      add_token(&pos, tokens, Tilde, "~", line, column);
      return pos;
    }
    case '?': {
      add_token(&pos, tokens, Question, "?", line, column);
      return pos;
    }
    case ':': {
      add_token(&pos, tokens, Colon, ":", line, column);
      return pos;
    }
    }

    if (c == '+') {
      if (pos[1] == '+') {
        add_token(&pos, tokens, PlusPlus, "++", line, column);
      } else if (pos[1] == '=') {
        add_token(&pos, tokens, PlusAssn, "+=", line, column);
      } else {
        add_token(&pos, tokens, Plus, "+", line, column);
      }
      return pos;
    }

    if (c == '-') {
      if (pos[1] == '-') {
        add_token(&pos, tokens, MinusMinus, "--", line, column);
      } else if (pos[1] == '=') {
        add_token(&pos, tokens, MinusAssn, "-=", line, column);
      } else if (pos[1] == '>') {
        add_token(&pos, tokens, Arrow, "->", line, column);
      } else {
        add_token(&pos, tokens, Minus, "-", line, column);
      }
      return pos;
    }

    if (c == '*') {
      if (pos[1] == '=') {
        add_token(&pos, tokens, StarAssn, "*=", line, column);
      } else {
        add_token(&pos, tokens, Star, "*", line, column);
      }
      return pos;
    }

    if (c == '/') {
      if (pos[1] == '=') {
        add_token(&pos, tokens, SlashAssn, "/=", line, column);
      } else if (pos[1] == '/') {
        for (skip(&pos, column, 2); *pos != '\n';) {
          skip(&pos, column, 1);
        }
      } else if (pos[1] == '*') {
        for (skip(&pos, column, 2); *pos != '*' || pos[1] != '/';) {
          skip(&pos, column, 1);
        }
        skip(&pos, column, 2);
      } else {
        add_token(&pos, tokens, Slash, "/", line, column);
      }
      // TODO: comments should be parsed somewhere here
      return pos;
    }

    if (c == '%') {
      if (pos[1] == '=') {
        add_token(&pos, tokens, PercentAssn, "%=", line, column);
      } else {
        add_token(&pos, tokens, Percent, "%", line, column);
      }
      return pos;
    }

    if (c == '&') {
      if (pos[1] == '=') {
        add_token(&pos, tokens, AmpAssn, "&=", line, column);
      } else if (pos[1] == '&') {
        add_token(&pos, tokens, AmpAmp, "&&", line, column);
      } else {
        add_token(&pos, tokens, Amp, "&", line, column);
      }
      return pos;
    }

    if (c == '|') {
      if (pos[1] == '=') {
        add_token(&pos, tokens, BarAssn, "|=", line, column);
      } else if (pos[1] == '&') {
        add_token(&pos, tokens, BarBar, "||", line, column);
      } else {
        add_token(&pos, tokens, Bar, "|", line, column);
      }
      return pos;
    }

    if (c == '^') {
      if (pos[1] == '=') {
        add_token(&pos, tokens, CaretAssn, "^=", line, column);
      } else {
        add_token(&pos, tokens, Caret, "^", line, column);
      }
      return pos;
    }

    if (c == '<') {
      if (pos[1] == '=') {
        add_token(&pos, tokens, Leq, "<=", line, column);
      } else if (pos[1] == '<') {
        if (pos[2] == '=') {
          add_token(&pos, tokens, LShftAssn, "<<=", line, column);
        } else {
          add_token(&pos, tokens, LShft, "<<", line, column);
        }
      } else {
        add_token(&pos, tokens, Lt, "<", line, column);
      }
      return pos;
    }

    if (c == '>') {
      if (pos[1] == '=') {
        add_token(&pos, tokens, Geq, ">=", line, column);
      } else if (pos[1] == '>') {
        if (pos[2] == '=') {
          add_token(&pos, tokens, RShftAssn, ">>=", line, column);
        } else {
          add_token(&pos, tokens, RShft, ">>", line, column);
        }
      } else {
        add_token(&pos, tokens, Gt, ">", line, column);
      }
      return pos;
    }

    if (c == '!') {
      if (pos[1] == '=') {
        add_token(&pos, tokens, Neq, "!=", line, column);
      } else {
        add_token(&pos, tokens, Exclaim, "!", line, column);
      }
      return pos;
    }

    if (c == '"') {
      char text[1024] = {0};

      size_t i;
      for (i = 1; pos[i] != '"'; i++) {
        text[i] = pos[i];
      }
      text[0] = '"';
      text[i] = '"';

      add_token(&pos, tokens, String, text, line, column);
      return pos;
    }

    if (c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
      char text[32] = {0};

      int i = 0;
      for (; c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
             ('0' <= c && c <= '9');
           c = pos[++i]) {
        text[i] = c;
      }

      // check if keyword
      for (int i = 0; i < 37; i++) {
        if (strcmp(keywords[i], text) == 0) {
          add_token(&pos, tokens, 48 + i, keywords[i], line, column);
          return pos;
        }
      }

      add_token(&pos, tokens, Id, text, line, column);
      return pos;
    }

    skip(&pos, column, 1);
  }
}

token_dynarr_t lex(char *src) {
  token_dynarr_t tokens = dynarr_init();
  dynarr_allocate(&tokens, 16);

  char *pos = src;
  unsigned int column = 1;
  unsigned int line = 1;

  while (*pos) {
    pos = next_token(pos, &tokens, &line, &column);
  }

  return tokens;
}

char *format_token(char *buffer, size_t len, token_t token) {
  snprintf(buffer, len, "%s\t%s\t%d:%d", token_kind_map[token.kind], token.text,
           token.line, token.column);
  return buffer;
}
