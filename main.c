#include <stdio.h>

#include "io.h"
#include "lex.h"
#include "parse.c"

int main(int argc, char *argv[]) {
  char *src;

  load_file(argv[1], &src);
  token_dynarr_t tokens = lex(src);

  size_t buffer_size = 128;
  char buffer[buffer_size];
  for (size_t i = 0; i < tokens.data_end - tokens.data; i++) {
    puts(format_token(buffer, buffer_size, tokens.data[i]));
  }

  tokens.data_end = NULL;

  parse_translation_unit(tokens.data);
  print_pt(pt);

  return 0;
}
