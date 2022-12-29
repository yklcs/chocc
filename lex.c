#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "dynarrsaur.h"
#include "lex.h"

const char *token_kind_map[] = {"BRACE_LEFT",    "BRACE_RIGHT", "BRACKET_LEFT",
                                "BRACKET_RIGHT", "PAREN_LEFT",  "PAREN_RIGHT",
                                "COMMA",         "SEMICOLON",   "ID"};

char *next_token(char *pos, token_dynarr_t *tokens, uint32_t *line) {
  token_t token = {0};
  bool done = false;

  while (!done && *pos++) {
    switch (*pos) {
    case '\n': {
      (*line)++;
      return pos;
    }
    case '{': {
      strcpy(token.text, "{");
      token.kind = BRACE_LEFT;
      done = true;
      break;
    }
    case '}': {
      strcpy(token.text, "}");
      token.kind = BRACE_RIGHT;
      done = true;
      break;
    }
    case '[': {
      strcpy(token.text, "[");
      token.kind = BRACKET_LEFT;
      done = true;
      break;
    }
    case ']': {
      strcpy(token.text, "]");
      token.kind = BRACKET_RIGHT;
      done = true;
      break;
    }
    case '(': {
      strcpy(token.text, "(");
      token.kind = PAREN_LEFT;
      done = true;
      break;
    }
    case ')': {
      strcpy(token.text, ")");
      token.kind = PAREN_RIGHT;
      done = true;
      break;
    }
    case ',': {
      strcpy(token.text, ",");
      token.kind = COMMA;
      done = true;
      break;
    }
    case ';': {
      strcpy(token.text, ";");
      token.kind = SEMICOLON;
      done = true;
      break;
    }
    }

    if (*pos == '_' || ('a' <= *pos && *pos <= 'z') ||
        ('A' <= *pos && *pos <= 'Z')) {
      char current[2] = {0};
      token.kind = ID;

      while (*pos == '_' || ('a' <= *pos && *pos <= 'z') ||
             ('A' <= *pos && *pos <= 'Z') || ('0' <= *pos && *pos <= '9')) {
        current[0] = *pos;
        strcat(token.text, current);
        pos++;
      }

      done = true;
    }

    break;
  }

  if (done) {
    token.line = *line;
    dynarr_push(tokens, token);
  }
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
  snprintf(buffer, len, "line %d\t%s\t%s", token.line,
           token_kind_map[token.kind], token.text);
  return buffer;
}
