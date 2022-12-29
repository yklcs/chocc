#include <stdio.h>
#include <stdlib.h>

#include "io.h"

void load_file(char *fname, char **fcontent) {
  FILE *file = fopen(fname, "rb");
  fseek(file, 0, SEEK_END);

  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  *fcontent = calloc(fsize + 1, sizeof(char));
  fread(*fcontent, sizeof(char), fsize, file);

  fclose(file);
}