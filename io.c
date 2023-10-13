#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"

void read_file(char *fname, char **fcontent) {
  FILE *file = fopen(fname, "rb");
  long fsize;

  fseek(file, 0, SEEK_END);
  fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  *fcontent = calloc(fsize + 1, sizeof(char));
  fread(*fcontent, sizeof(char), fsize, file);

  fclose(file);
}

file *src_to_file(char *src) {
  file *f = calloc(1, sizeof(file));

  char *pos = src;
  char *ln_begin = src;
  char *ln_end = NULL;
  int i = 1;

  f->lines_len = 0;
  f->lines_cap = 64;
  f->lines = calloc(f->lines_cap, sizeof(line));

  for (;; pos++) {
    if (*pos == '\n' || !*pos) {
      line ln = {0};

      int j;
      bool space = true;

      ln.num = i++;
      if (pos > src) {
        ln.splice = *(pos - 1) == '\\';
      }

      ln_end = pos;
      ln.src = calloc(ln_end - ln_begin + 1, 1);
      ln.len = ln_end - ln_begin;
      strncpy(ln.src, ln_begin, ln.len);

      for (j = 0; j < ln.len; j++) {
        if (space && ln.src[j] == '#') {
          ln.cpp = true;
        }
        if (!isspace(ln.src[j])) {
          space = false;
        }
      }

      ln_begin = pos + 1;

      if (f->lines_cap == f->lines_len) {
        f->lines_cap *= 2;
        f->lines = realloc(f->lines, sizeof(line) * f->lines_cap);
      }
      f->lines[f->lines_len++] = ln;

      if (*pos) {
        continue;
      }
    } else {
      continue;
    }
    break;
  }

  return f;
}

file *load_file(char *fname) {
  char *fcontent;
  read_file(fname, &fcontent);
  return src_to_file(fcontent);
}

void print_file(file *f) {
  int i;
  for (i = 0; i < f->lines_len; i++) {
    printf("%3d | %s\n", f->lines[i].num, f->lines[i].src);
  }
}
