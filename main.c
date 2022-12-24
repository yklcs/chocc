#include <stdio.h>

#include "lex.h"

void load_file(char *fname, char **fcontent) {
  FILE *file = fopen(fname, "rb");
  fseek(file, 0, SEEK_END);

  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  *fcontent = calloc(fsize + 1, sizeof(char));
  fread(*fcontent, fsize, 1, file);
  fclose(file);
}

int main(int argc, char *argv[]) {
  char *src;

  load_file(argv[1], &src);
  token_dynarr_t tokens = lex(src);

  size_t buffer_size = 128;
  char buffer[buffer_size];
  for (size_t i = 0; i < tokens.data_end - tokens.data; i++) {
    puts(format_token(buffer, buffer_size, tokens.data[i]));
  }

  dynarr_free(tokens);
  return 0;
}
