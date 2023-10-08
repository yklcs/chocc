#include <stdio.h>
#include <stdlib.h>

#include "chocc.h"
#include "cpp.h"
#include "io.h"
#include "lex.h"
#include "parse.h"

int main(int argc, char *argv[]) {
  file *f;
  struct unit *u;
  ast_node_t *ast;
  int ast_len;
  int i;

  if (argc != 2) {
    puts("usage: chocc input.c");
    exit(1);
  }

  f = load_file(argv[1]);
  print_file(f);

  u = lex_file(f);
  u = cpp(u);

  ast_len = parse(u, &ast);

  for (i = 0; i < ast_len; i++) {
    print_ast(ast + i, 0, i == ast_len - 1, "");
  }

  return 0;
}
