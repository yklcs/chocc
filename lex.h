#ifndef CHOCC_LEX_H
#define CHOCC_LEX_H
#pragma once

#include "dynarrsaur.h"

typedef enum {
  // delimiters
  BRACE_LEFT,
  BRACE_RIGHT,
  BRACKET_LEFT,
  BRACKET_RIGHT,
  PAREN_LEFT,
  PAREN_RIGHT,
  COMMA,
  SEMICOLON,
} token_name_t;

typedef struct {
  token_name_t name;
  uint32_t line;
} token_t;

typedef dynarr_t(token_t) token_dynarr_t;

char *next_token(char *pos, token_dynarr_t *tokens, uint32_t *line);
token_dynarr_t lex(char *src);
char *format_token(char *buffer, size_t len, token_t token);

#endif
