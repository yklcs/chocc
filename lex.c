#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "dynarrsaur.h"
#include "lex.h"

const char *token_name_map[] = {
    "BRACE_LEFT", "BRACE_RIGHT", "BRACKET_LEFT", "BRACKET_RIGHT",
    "PAREN_LEFT", "PAREN_RIGHT", "COMMA",        "SEMICOLON",
};

char *next_token(char *pos, token_dynarr_t *tokens, uint32_t *line) {
  char c;
  token_t token;
  bool done = false;

  while (!done && (c = *pos++)) {
    switch (c) {
    case '\n': {
      (*line)++;
      return pos;
    }
    case '{': {
      token.name = BRACE_LEFT;
      done = true;
      break;
    }
    case '}': {
      token.name = BRACE_RIGHT;
      done = true;
      break;
    }
    case '[': {
      token.name = BRACKET_LEFT;
      done = true;
      break;
    }
    case ']': {
      token.name = BRACKET_RIGHT;
      done = true;
      break;
    }
    case '(': {
      token.name = PAREN_LEFT;
      done = true;
      break;
    }
    case ')': {
      token.name = PAREN_RIGHT;
      done = true;
      break;
    }
    case ',': {
      token.name = COMMA;
      done = true;
      break;
    }
    case ';': {
      token.name = SEMICOLON;
      done = true;
      break;
    }

    default: {
      return pos;
    }
    }
  }

  token.line = *line;
  dynarr_push(tokens, token);
  return pos;
}

token_dynarr_t lex(char *src) {
  token_dynarr_t tokens = dynarr_init();
  dynarr_allocate(&tokens, 16);

  char *pos = src;
  uint32_t line = 1;

  while (*pos) {
    pos = next_token(pos, &tokens, &line);
  }

  return tokens;
}

char *format_token(char *buffer, size_t len, token_t token) {
  snprintf(buffer, len, "token %s\tline %d", token_name_map[token.name],
           token.line);
  return buffer;
}
