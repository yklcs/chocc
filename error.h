#ifndef CHOCC_ERROR_H
#define CHOCC_ERROR_H
#pragma once

#include <stdio.h>

#include "chocc.h"
#include "io.h"

typedef enum { ParseErr, LexErr, CppErr } error_kind;
struct error {
  char *msg;
  error_kind kind;
  loc pos;
};

struct error new_error(error_kind kind, char *msg, loc pos);
void print_error(struct error);

#endif
