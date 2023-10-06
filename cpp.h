#ifndef CHOCC_CPP_H
#define CHOCC_CPP_H
#pragma once

#include "lex.h"
#include "parse.h"

int cpp(token_t **toks_out, token_t *toks_in, int toks_in_len);

int cpp_cond(token_t **toks_out, parser_t *p);
int cpp_define(token_t **toks_out, parser_t *p);

typedef enum def_kind { Blank, Macro, FnMacro } def_kind;

typedef struct def {
  token_t id;

  token_t *macro;
  int macro_len;

  token_t *params;
  int params_len;

  def_kind kind;
} def;

bool cpp_define_expand(token_t **toks, int *toks_len, parser_t *p, def *defs,
                       int defs_len);

/* Filter out tokens that pass a filter function.  */
int filter_newline(token_t **toks_out, token_t *toks_in, int toks_in_len);

/*
 * Evalulate integer constant expression
 */
unsigned long eval_cpp_const_expr(ast_node_t *root);

#endif
