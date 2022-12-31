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
               char *token_text, unsigned int *token_line,
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
  bool done = false;

  for (char c = *pos; !done; c = *pos) {
    switch (c) {
    case '\n': {
      *column = 1;
      (*line)++;
      return pos + 1;
    }
    case '#': {
      add_token(&pos, tokens, Hash, "#", line, column);
      done = true;
      break;
    }
    case '{': {
      add_token(&pos, tokens, LBrace, "{", line, column);
      done = true;
      break;
    }
    case '}': {
      add_token(&pos, tokens, RBrace, "}", line, column);
      done = true;
      break;
    }
    case '[': {
      add_token(&pos, tokens, LSquare, "[", line, column);
      done = true;
      break;
    }
    case ']': {
      add_token(&pos, tokens, RSquare, "]", line, column);
      done = true;
      break;
    }
    case '(': {
      add_token(&pos, tokens, LParen, "(", line, column);
      done = true;
      break;
    }
    case ')': {
      add_token(&pos, tokens, RParen, ")", line, column);
      done = true;
      break;
    }
    case ',': {
      add_token(&pos, tokens, Comma, ",", line, column);
      done = true;
      break;
    }
    case ';': {
      add_token(&pos, tokens, Semi, ";", line, column);
      done = true;
      break;
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
      done = true;
    }

    if (c == '-') {
      if (pos[1] == '-') {
        add_token(&pos, tokens, MinusMinus, "--", line, column);
      } else if (pos[1] == '=') {
        add_token(&pos, tokens, MinusAssn, "-=", line, column);
      } else {
        add_token(&pos, tokens, Minus, "-", line, column);
      }
      done = true;
    }

    if (c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
      char text[32] = {0};

      int i = 0;
      for (; c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
             ('0' <= c && c <= '9');
           c = pos[++i]) {
        text[i] = c;
      }

      add_token(&pos, tokens, Id, text, line, column);
      done = true;
    }

    if (!done) {
      skip(&pos, column, 1);
    }
  }

  return pos;
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
