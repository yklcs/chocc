#include "cpp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool token_not_lf(token_t tok) {
  return tok.kind != Lf;
}

int filter_tokens(token_t **toks_out, bool (*test)(token_t), token_t *toks_in,
                  int toks_in_len) {
  int len = 0;
  int i = 0;
  token_t *out;
  out = calloc(toks_in_len, sizeof(token_t));
  for (i = 0; i < toks_in_len; i++) {
    if (test(toks_in[i])) {
      out[len++] = toks_in[i];
    }
  }
  *toks_out = out;
  return len;
}
