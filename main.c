#include <stdio.h>
#include <stdlib.h>

#include "chocc.h"
#include "io.h"
#include "parse.h"
#include "token.h"

int main(int argc, char *argv[]) {
  char *src;
  tokens_t tokens;
  parser_t parser;
  ast_node_t **ast;
  ast_node_t **node;

  if (argc != 2) {
    puts("usage: chocc input.c");
    exit(1);
  }
  load_file(argv[1], &src);
  tokens = lex(src);

  parser.tokens = &tokens;
  set_pos(&parser, 0);
  ast = parse(&parser);

  for (node = ast; *node != NULL; node++) {
    print_ast(*node, 0);
  }

  return 0;
}
