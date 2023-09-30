#include <stdio.h>
#include <stdlib.h>

#include "chocc.h"
#include "io.h"
#include "lex.h"
#include "parse.h"

int main(int argc, char *argv[]) {
  token_t *toks = NULL;
  int toks_len;
  parser_t parser;
  ast_node_t **ast;
  ast_node_t **node;
  file *f;
  int i;

  if (argc != 2) {
    puts("usage: chocc input.c");
    exit(1);
  }

  f = load_file(argv[1]);
  print_file(f);

  toks_len = lex_file(f, &toks);
  for (i = 0; i < toks_len; i++) {
    print_token(toks[i]);
  }

  parser.toks = toks;
  set_pos(&parser, 0);
  ast = parse(&parser);

  for (node = ast; *node != NULL; node++) {
    print_ast(*node, 0);
  }

  return 0;
}
