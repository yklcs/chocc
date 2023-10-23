#include <stdio.h>
#include <stdlib.h>

#include "chocc.h"
#include "cpp.h"
#include "io.h"
#include "lex.h"
#include "parse.h"
#include "unit.h"

int main(int argc, char *argv[]) {
  file *f;
  struct unit u;
  int i;

  if (argc != 2) {
    puts("usage: chocc input.c");
    exit(1);
  }

  f = load_file(argv[1]);
  print_file(f);

  u = new_unit();
  u.file = f;

  lex(&u);
  cpp(&u);

  parse(&u);

  for (i = 0; i < u.nodes_len; i++) {
    print_ast(u.nodes + i, 0, i == u.nodes_len - 1, "");
  }

  return 0;
}
