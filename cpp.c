#include "cpp.h"
#include "chocc.h"
#include "lex.h"
#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cpp(token_t **toks_out, token_t *toks_in, int toks_len) {
  token_t *toks = toks_out ? *toks_out : NULL;
  toks_len = cpp_define(&toks, toks_in, toks_len);
  toks_len = cpp_cond(&toks, toks, toks_len);
  toks_len = filter_newline(&toks, toks, toks_len);

  *toks_out = toks;
  return toks_len;
}

int cpp_define(token_t **toks_out, token_t *toks_in, int toks_in_len) {
  int out_len = 0;
  int out_cap = toks_in_len;
  token_t *out = calloc(out_cap, sizeof(token_t));

  int defs_len = 0;
  int defs_cap = 1;
  struct def {
    token_t *id;
    token_t *macro;
    int macro_len;
  } *defs = calloc(defs_cap, sizeof(struct def));

  bool cpp_line = false;

  int i = 0;
  for (i = 0; i < toks_in_len; i++) {
    int j;
    bool defined = false;

    if (toks_in[i].kind == Directive) {
      cpp_line = true;
    }
    if (toks_in[i].kind == Lf) {
      cpp_line = false;
    }

    /* perform definition */
    if (toks_in[i].kind == Directive && !strcmp(toks_in[i].text, "#define") &&
        i + 2 < toks_in_len) {

      if (defs_len >= defs_cap) {
        defs_cap *= 2;
        defs = realloc(defs, defs_cap * sizeof(struct def));
      }

      defs[defs_len].id = toks_in + i + 1;
      if (toks_in[i + 1].kind == Id &&
          toks_in[i + 2].kind == Lf) { /* #define id */
        defs[defs_len].macro = NULL;
        i++;
      } else { /* #define id macro */
        token_t *macro_toks = toks_in + i + 2;
        int k;
        for (k = 0; macro_toks[k].kind != Lf &&
                    macro_toks[k].kind != Eof;) { /*  skip to end of line */
          k++;
        }
        defs[defs_len].macro = calloc(k, sizeof(token_t));
        defs[defs_len].macro_len = k;
        memcpy(defs[defs_len].macro, macro_toks, k * sizeof(token_t));
        i += k + 1;
      }
      defs_len++;
      continue;
    }

    if (toks_in[i].kind == Directive && !strcmp(toks_in[i].text, "#ifdef")) {
      strcpy(toks_in[i].text, "#if defined");
    }
    if (toks_in[i].kind == Directive && !strcmp(toks_in[i].text, "#ifndef")) {
      strcpy(toks_in[i].text, "#if ! defined");
    }

    /* perform defined replacement */
    if (cpp_line && toks_in[i].kind == Id &&
        !strcmp(toks_in[i].text, "defined")) {
      char *target = NULL;
      bool paren;
      if (i + 3 < toks_in_len && toks_in[i + 1].kind == LParen &&
          toks_in[i + 2].kind == Id && toks_in[i + 3].kind == RParen) {
        target = toks_in[i + 2].text;
        paren = true;
      } else if (i + 1 < toks_in_len && toks_in[i + 1].kind == Id) {
        target = toks_in[i + 1].text;
        paren = false;
      }

      if (target) {
        int j;
        loc pos;
        pos.ln = toks_in[i].line;
        pos.col = toks_in[i].column;

        for (j = 0; j < defs_len; j++) {
          if (!strcmp(defs[j].id->text, target)) {
            out[out_len++] = new_token(Number, pos, "1");
            break;
          }
        }

        if (j == defs_len) {
          out[out_len++] = new_token(Number, pos, "0");
        }
        i += paren ? 3 : 1;
        continue;
      }
    }

    /* perform macro expansion */
    for (j = 0; j < defs_len; j++) {
      if (!strcmp(defs[j].id->text, toks_in[i].text) && defs[j].macro_len) {
        if (out_len + defs[j].macro_len > out_cap) {
          out_cap = out_len + defs[j].macro_len;
          out = realloc(out, sizeof(token_t) * out_cap);
        }

        memcpy(out + out_len, defs[j].macro,
               defs[j].macro_len * sizeof(token_t));
        out_len += defs[j].macro_len;
        defined = true;
        break;
      }
    }
    if (!defined) {
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

int cpp_cond(token_t **toks_out, token_t *toks_in, int toks_in_len) {
  token_t *out = calloc(toks_in_len, sizeof(token_t));
  int len = 0;

  int i = 0;
  for (i = 0; i < toks_in_len; i++) {
    if (toks_in[i].kind == Directive && !strcmp(toks_in[i].text, "#if")) {
      bool cond;
      parser_t p;
      ast_node_t *x;
      p.toks = toks_in + i + 1;
      set_pos(&p, 0);
      x = expr(&p, 0);
      cond = eval_cpp_const_expr(x);
      for (; toks_in[i].kind != Lf; i++) {
      }

      if (cond) {
        for (;
             toks_in[i].kind != Directive && strcmp(toks_in[i].text, "#endif");
             i++) {
          out[len++] = toks_in[i];
        }
      } else {
        for (;
             toks_in[i].kind != Directive && strcmp(toks_in[i].text, "#endif");
             i++) {
        }
      }
      continue;
    }

    if (toks_in[i].kind != Lf) {
      out[len++] = toks_in[i];
    }
  }
  *toks_out = out;
  return len;
}

int filter_newline(token_t **toks_out, token_t *toks_in, int toks_in_len) {
  token_t *out = calloc(toks_in_len, sizeof(token_t));
  int len = 0;

  int i = 0;
  for (i = 0; i < toks_in_len; i++) {
    if (toks_in[i].kind != Lf) {
      out[len++] = toks_in[i];
    }
  }
  *toks_out = out;
  return len;
}

unsigned long eval_cpp_const_expr(ast_node_t *root) {
  switch (root->kind) {
  case Lit: {
    switch (root->u.lit.kind) {
    case CharLit:
      return root->u.lit.character[0];
    case DecLit:
    case HexLit:
    case OctLit:
      return root->u.lit.integer;
    default:
      puts("invalid literal in cpp constant expression");
      exit(1);
    }
  }
  case Expr: {
    switch (root->u.expr.kind) {
    case CastExpr:
    case CallExpr:
    case CommaExpr:
    case PostfixExpr:
      puts("invalid expression in cpp constant expression");
      exit(1);
    case PrefixExpr: {
      unsigned long rhs = eval_cpp_const_expr(root->u.expr.rhs);
      switch (root->u.expr.op) {
      case Plus:
        return +rhs;
      case Minus:
        return -rhs;
      case Tilde:
        return ~rhs;
      case Exclaim:
        return !rhs;
      default:
        puts("invalid expression in cpp constant expression");
        exit(1);
      }
    }
    case InfixExpr: {
      unsigned long lhs = eval_cpp_const_expr(root->u.expr.lhs);
      unsigned long rhs = eval_cpp_const_expr(root->u.expr.rhs);
      unsigned long mhs =
          root->u.expr.mhs ? eval_cpp_const_expr(root->u.expr.mhs) : 0;
      switch (root->u.expr.op) {
      case Star:
        return lhs * rhs;
      case Slash:
        return lhs / rhs;
      case Percent:
        return lhs % rhs;
      case Plus:
        return lhs + rhs;
      case Minus:
        return lhs - rhs;
      case LShft:
        return lhs << rhs;
      case RShft:
        return lhs >> rhs;
      case Lt:
        return lhs < rhs;
      case Leq:
        return lhs <= rhs;
      case Gt:
        return lhs > rhs;
      case Geq:
        return lhs >= rhs;
      case Eq:
        return lhs == rhs;
      case Neq:
        return lhs != rhs;
      case Amp:
        return lhs & rhs;
      case Caret:
        return lhs ^ rhs;
      case Bar:
        return lhs | rhs;
      case AmpAmp:
        return lhs && rhs;
      case BarBar:
        return lhs || rhs;
      case Question:
        return lhs ? mhs : rhs;
      default:
        puts("invalid expression in cpp constant expression");
        exit(1);
      }
    }
    }
  }
  default: {
    puts("invalid cpp constant expression");
    exit(1);
  }
  }

  return 0;
}
