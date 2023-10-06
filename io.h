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

  struct loc cur;
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

/*
 * Gets the next character after the file cursor and advances the cursor,
 * while setting pos to the position of the character.
 */
char next_char(file *f, loc *pos);

/*
 * Gets the next character after the file cursor without advancing the cursor.
 */
char peek_char(file *f);

#endif
