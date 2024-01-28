#ifndef CHOCC_UNIT_H
#define CHOCC_UNIT_H
#pragma once

#include <string.h>

#include "lex.h"
#include "parse.h"

struct unit {
  file *file;

  token_t *toks;
  int toks_len;
  int toks_cap;

  ast_node_t *nodes;
  int nodes_len;
  int nodes_cap;

  struct error *err;
};

struct unit new_unit(void);
void unit_append_tok(struct unit *u, token_t tok);
void unit_append_node(struct unit *u, ast_node_t node);

#endif
