#ifndef CHOCC_IO_H
#define CHOCC_IO_H
#pragma once

#include "chocc.h"

typedef struct loc {
  int ln;
  int col;
} loc;

typedef struct file {
  struct line *lines;
  int lines_len;
  int lines_cap;
} file;

typedef struct line {
  int num;
  char *src;
  int len;
  bool splice;
  bool cpp;
} line;

/*
 * Loads file from path.
 */
file *load_file(char *fname);

void print_file(file *);

#endif
