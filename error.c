#include "error.h"
#include <stdlib.h>

struct error *new_error(error_kind kind, char *msg, loc pos) {
  struct error *err;

  err = calloc(1, sizeof(*err));
  err->kind = kind;
  err->msg = msg;
  err->pos = pos;

  return err;
}

void print_error(struct error *err) {
  switch (err->kind) {
  case ParseErr:
    printf("Parse");
    break;
  case LexErr:
    printf("Lex");
    break;
  case CppErr:
    printf("Preprocessor");
    break;
  }

  printf(" error at %d:%d\n", err->pos.ln, err->pos.col);
  puts(err->msg);
}
