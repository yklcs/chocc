#include "cpp.h"
#include "chocc.h"
#include "lex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define A

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

int cpp_replace(token_t **toks_out, token_t *toks_in, int toks_in_len) {
  int out_len = 0;
  int out_cap = toks_in_len;
  int i = 0;

  token_t *out = calloc(out_cap, sizeof(token_t));

  int subs_len = 0;
  int subs_cap = 1;

  struct sub {
    char *id;
    token_t *toks;
    int toks_len;
  } *subs = calloc(subs_cap, sizeof(struct sub));

  for (i = 0; i < toks_in_len; i++) {
    int j;
    token_t directive = toks_in[i];
    bool subbed = false;

    if (directive.kind == Directive && !strncmp(directive.text, "#define", 8)) {
      token_t ident = toks_in[i + 1];
      token_t *sub_toks = toks_in + i + 2;

      if (subs_len >= subs_cap) {
        subs_cap *= 2;
        subs = realloc(subs, subs_cap * sizeof(struct sub));
      }

      if (ident.kind == Id && sub_toks->kind == Lf) { /* #define ident */
        subs[subs_len].id = calloc(strlen(ident.text) + 1, 1);
        strcpy(subs[subs_len].id, ident.text);
        subs[subs_len].toks = NULL;
      } else { /* #define ident tokens */
        int k;
        for (k = 0; sub_toks[k].kind != Lf && sub_toks[k].kind != Eof;) {
          k++;
        }
        subs[subs_len].id = calloc(strlen(ident.text) + 1, 1);
        strcpy(subs[subs_len].id, ident.text);
        subs[subs_len].toks = calloc(k, sizeof(token_t));
        subs[subs_len].toks_len = k;
        memcpy(subs[subs_len].toks, sub_toks, k * sizeof(token_t));
        i += k + 1;
      }
      subs_len++;
      continue;
    }

    for (j = 0; j < subs_len; j++) {
      if (!strcmp(subs[j].id, toks_in[i].text)) {
        if (out_len + subs[j].toks_len > out_cap) {
          out_cap = out_len + subs[j].toks_len;
          out = realloc(out, sizeof(token_t) * out_cap);
        }

        memcpy(out + out_len, subs[j].toks, subs[j].toks_len * sizeof(token_t));
        out_len += subs[j].toks_len;
        subbed = true;
        break;
      }
    }

    if (!subbed) {
      if (out_len > out_cap) {
        out_cap *= 2;
        out = realloc(out, sizeof(token_t) * out_cap);
      }
      out[out_len++] = toks_in[i];
    }
  }

  *toks_out = out;
  return out_len;
}
