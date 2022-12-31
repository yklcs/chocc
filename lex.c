#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "dynarrsaur.h"
#include "lex.h"

char *next_token(char *pos, token_dynarr_t *tokens, unsigned int *line,
                 unsigned int *column_begin) {
  token_t token = {0};
  unsigned int column_end = *column_begin + 1;
  bool done = false;
  char c;

  for (; !done && (c = *pos++);) {
    switch (c) {
    case '\n': {
      *column_begin = 1;
      (*line)++;
      return pos + 1;
    }
    case '{': {
      strcpy(token.text, "{");
      token.kind = LBrace;
      done = true;
      break;
    }
    case '}': {
      strcpy(token.text, "}");
      token.kind = RBrace;
      done = true;
      break;
    }
    case '[': {
      strcpy(token.text, "[");
      token.kind = LSquare;
      done = true;
      break;
    }
    case ']': {
      strcpy(token.text, "]");
      token.kind = RSquare;
      done = true;
      break;
    }
    case '(': {
      strcpy(token.text, "(");
      token.kind = LParen;
      done = true;
      break;
    }
    case ')': {
      strcpy(token.text, ")");
      token.kind = RParen;
      done = true;
      break;
    }
    case ',': {
      strcpy(token.text, ",");
      token.kind = Comma;
      done = true;
      break;
    }
    case ';': {
      strcpy(token.text, ";");
      token.kind = Semi;
      done = true;
      break;
    }
    }

    if (c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
      char current[2] = {0};
      token.kind = Id;

      while (c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
             ('0' <= c && c <= '9')) {
        current[0] = c;
        strcat(token.text, current);
        c = *pos++;
        column_end++;
      }

      done = true;
    }

    if (!done) {
      *column_begin = column_end++;
    }
  }

  if (done) {
    token.line = *line;
    token.column = *column_begin;
    dynarr_push(tokens, token);
  }

  *column_begin = column_end;
  return pos;
}

token_dynarr_t lex(char *src) {
  token_dynarr_t tokens = dynarr_init();
  dynarr_allocate(&tokens, 16);

  char *pos = src;
  unsigned int column = 1;
  unsigned int line = 1;

  while (*(pos + 1)) {
    pos = next_token(pos, &tokens, &line, &column);
  }

  return tokens;
}

char *format_token(char *buffer, size_t len, token_t token) {
  snprintf(buffer, len, "%s\t%s\t%d:%d", token_kind_map[token.kind], token.text,
           token.line, token.column);
  return buffer;
}
