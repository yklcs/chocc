#ifndef CHOCC_CPP_H
#define CHOCC_CPP_H
#pragma once

#include "lex.h"
#include "parse.h"

typedef enum def_kind { Blank, Macro, FnMacro } def_kind;
typedef struct def {
  token_t id;

  token_t *macro;
  int macro_len;

  token_t *params;
  int params_len;

  def_kind kind;
} def;

void cpp(struct unit *in);

struct unit cpp_replace(struct unit *in);
int cpp_replace_define(parser_t *p, def **defs, int defs_len);
int cpp_replace_expand(struct unit *out, parser_t *p, def *defs, int defs_len,
                       token_t *hideset, int hideset_len);

struct unit cpp_cond(struct unit *in);
struct unit cpp_cond_if(parser_t *p);

struct unit cpp_pragma(struct unit *in);

struct unit cpp_include(struct unit *in);

struct unit filter_newline(struct unit *in);

/*
 * Evalulate integer constant expression
 */
unsigned long eval_cpp_const_expr(ast_node_t *root);

#endif
