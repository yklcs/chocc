#include "unit.h"

#include <stdlib.h>

struct unit new_unit(void) {
  struct unit u = {0};

  u.nodes_cap = 64;
  u.nodes = calloc(u.nodes_cap, sizeof(*u.nodes));

  u.toks_cap = 64;
  u.toks = calloc(u.toks_cap, sizeof(*u.toks));

  return u;
}

void unit_append_tok(struct unit *u, token_t tok) {
  if (u->toks_len == u->toks_cap) {
    u->toks_cap *= 2;
    u->toks = realloc(u->toks, u->toks_cap * sizeof(*u->toks));
  }
  u->toks[u->toks_len++] = tok;
}

void unit_append_node(struct unit *u, ast_node_t node) {
  if (u->nodes_len == u->nodes_cap) {
    u->nodes_cap *= 2;
    u->nodes = realloc(u->nodes, u->nodes_cap * sizeof(*u->nodes));
  }
  u->nodes[u->nodes_len++] = node;
}
