#ifndef CHOCC_CPP_H
#define CHOCC_CPP_H
#pragma once

#include "lex.h"

bool token_not_lf(token_t tok);

/* Filter out tokens that pass a filter function.  */
int filter_tokens(token_t **toks_out, bool (*filter)(token_t tok),
                  token_t *toks_in, int toks_in_len);

int cpp_replace(token_t **toks_out, token_t *toks_in, int toks_in_len);
#endif
