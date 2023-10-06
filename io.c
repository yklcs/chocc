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

  f->cur.col = 1;
  f->cur.ln = 1;
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

char next_char(file *f, loc *pos) {
  line ln;

  if (f->cur.ln > f->lines_len ||
      (f->cur.ln == f->lines_len &&
       f->lines[f->lines_len - 1].len < f->cur.col)) {
    /* eof */
    if (pos) {
      ln = f->lines[f->lines_len];
      pos->ln = f->lines_len;
      pos->col = ln.len + 1;
    }

    return 0;
  }

  ln = f->lines[f->cur.ln - 1];

  if (f->cur.col > ln.len) {
    /* overflow to next line */
    if (pos) {
      pos->col = f->cur.col;
      pos->ln = f->cur.ln;
    }

    f->cur.col = 1;
    f->cur.ln++;

    return '\n';
  } else if (f->cur.col == ln.len && ln.splice) {
    /* splicing, return characters after splicing but maintain cursor */
    ln = f->lines[++f->cur.ln - 1];
    f->cur.col = 1;

    if (pos) {
      pos->col = f->cur.col;
      pos->ln = f->cur.ln;
    }

    return ln.src[f->cur.col++ - 1];
  } else {
    /* normal */
    if (pos) {
      pos->col = f->cur.col;
      pos->ln = f->cur.ln;
    }

    return ln.src[f->cur.col++ - 1];
  }
}

char peek_char(file *f) {
  file f_fake = {0};
  f_fake.lines = f->lines;
  f_fake.lines_cap = f->lines_cap;
  f_fake.lines_len = f->lines_len;
  f_fake.cur = f->cur;
  return next_char(&f_fake, NULL);
}
