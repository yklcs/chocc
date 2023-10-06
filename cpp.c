#include "cpp.h"
#include "chocc.h"
#include "lex.h"
#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cpp(token_t **toks_out, token_t *toks_in, int toks_len) {
  token_t *toks = toks_out ? *toks_out : NULL;
  int i;

  parser_t p;

  p.toks = toks_in;
  p.toks_len = toks_len;
  set_pos(&p, 0);
  toks_len = cpp_define(&toks, &p);

  p.toks = toks;
  p.toks_len = toks_len;
  set_pos(&p, 0);
  toks_len = cpp_cond(&toks, &p);

  toks_len = filter_newline(&toks, toks, toks_len);

  for (i = 0; i < toks_len; i++) {
    print_token(toks[i]);
  }

  *toks_out = toks;
  return toks_len;
}

int cpp_define(token_t **toks_out, parser_t *p) {
  int out_len = 0;
  int out_cap = p->toks_len;
  token_t *out = calloc(out_cap, sizeof(token_t));

  int defs_len = 0;
  int defs_cap = 1;
  struct def {
    token_t *id;
    token_t *macro;
    int macro_len;
  } *defs = calloc(defs_cap, sizeof(struct def));

  bool cpp_line = false;

  for (; p->kind != Eof; advance(p)) {
    int j;
    bool defined = false;

    if (p->kind == Directive) {
      cpp_line = true;
    }
    if (p->kind == Lf) {
      cpp_line = false;
    }

    /* perform definition */
    if (p->kind == Directive && !strcmp(p->tok.text, "#define")) {
      if (defs_len >= defs_cap) {
        defs_cap *= 2;
        defs = realloc(defs, defs_cap * sizeof(struct def));
      }

      defs[defs_len].id = p->toks + p->pos + 1;
      if (peek(p, 1).kind == Id && peek(p, 2).kind == Lf) { /* #define id */
        defs[defs_len].macro = NULL;
        defs[defs_len].macro_len = 0;
        advance(p);
      } else { /* #define id macro */
        token_t *macro_toks = p->toks + p->pos + 2;
        int k;
        for (k = 0; macro_toks[k].kind != Lf &&
                    macro_toks[k].kind != Eof;) { /*  skip to end of line */
          k++;
        }
        defs[defs_len].macro = calloc(k, sizeof(token_t));
        defs[defs_len].macro_len = k;
        memcpy(defs[defs_len].macro, macro_toks, k * sizeof(token_t));
        set_pos(p, p->pos + k + 1);
      }
      defs_len++;
      continue;
    }

    /* perform defined replacement */
    if (cpp_line &&
        ((p->kind == Id && !strcmp(p->tok.text, "defined")) ||
         (p->kind == Directive && (!strcmp(p->tok.text, "#ifdef") ||
                                   !strcmp(p->tok.text, "#ifndef"))))) {
      char *target = NULL;
      bool paren;
      if (p->pos + 3 < p->toks_len && peek(p, 1).kind == LParen &&
          peek(p, 2).kind == Id && peek(p, 3).kind == RParen) {
        target = p->toks[p->pos + 2].text;
        paren = true;
      } else if (p->pos + 1 < p->toks_len && peek(p, 1).kind == Id) {
        target = p->toks[p->pos + 1].text;
        paren = false;
      }

      if (target) {
        int j;
        loc pos;
        pos.ln = p->tok.line;
        pos.col = p->tok.column;

        if (!strcmp(p->tok.text, "#ifdef")) {
          out[out_len++] = new_token(Directive, pos, "#if");
        } else if (!strcmp(p->tok.text, "#ifndef")) {
          out[out_len++] = new_token(Directive, pos, "#if");
          out[out_len++] = new_token(Exclaim, pos, "!");
        }

        for (j = 0; j < defs_len; j++) {
          if (!strcmp(defs[j].id->text, target)) {
            out[out_len++] = new_token(Number, pos, "1");
            break;
          }
        }

        if (j == defs_len) {
          out[out_len++] = new_token(Number, pos, "0");
        }
        set_pos(p, p->pos += paren ? 3 : 1);
        continue;
      }
    }

    /* perform macro expansion */
    for (j = 0; j < defs_len; j++) {
      if (!strcmp(defs[j].id->text, p->tok.text) && defs[j].macro_len) {
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
      out[out_len++] = p->tok;
    }
  }

  out[out_len++] = p->tok;
  *toks_out = out;
  return out_len;
}

bool cpp_cond_cond(parser_t *p) {
  bool cond = false;

  ast_node_t *x = expr(p, 0);
  if (p->kind != Lf) {
    puts("malformed cpp constexpr");
    exit(1);
  }
  cond = eval_cpp_const_expr(x);
  advance(p);

  return cond;
}

int cpp_cond_if(token_t **toks_out, parser_t *p) {
  token_t *out = calloc(p->toks_len, sizeof(token_t));
  int len = 0;
  bool cond = false;
  bool cond_elif = false;

  token_t *toks_nested;
  int toks_nested_len;

  /* #if */
  if (p->kind != Directive ||
      (strcmp(p->tok.text, "#if") && strcmp(p->tok.text, "#elif"))) {
    printf("expected #if, got %s\n", p->tok.text);
  }
  advance(p);
  cond = cpp_cond_cond(p);

  /* text */
  if (cond) {
    for (; strcmp(p->tok.text, "#elif") && strcmp(p->tok.text, "#else") &&
           strcmp(p->tok.text, "#endif");
         advance(p)) {
      if (p->kind == Directive && !strcmp(p->tok.text, "#if")) {
        /* nested #if group */
        int j;
        toks_nested_len = cpp_cond_if(&toks_nested, p);
        for (j = 0; j < toks_nested_len; j++) {
          out[len++] = toks_nested[j];
        }
      } else {
        /* regular text */
        out[len++] = p->tok;
      }
    }
  } else { /* skip to end of text */
    for (; strcmp(p->tok.text, "#elif") && strcmp(p->tok.text, "#else") &&
           strcmp(p->tok.text, "#endif");
         advance(p)) {
    }
  }

  /* elif */
  for (; p->kind == Directive && !strcmp(p->tok.text, "#elif");) {
    advance(p);
    cond_elif = cpp_cond_cond(p);

    if (!cond && cond_elif) {
      for (; strcmp(p->tok.text, "#elif") && strcmp(p->tok.text, "#else") &&
             strcmp(p->tok.text, "#endif");) {
        if (p->kind == Directive && !strcmp(p->tok.text, "#if")) {
          int j;
          toks_nested_len = cpp_cond_if(&toks_nested, p);
          for (j = 0; j < toks_nested_len; j++) {
            out[len++] = toks_nested[j];
          }
        } else {
          out[len++] = p->tok;
          advance(p);
        }
      }
      cond = true;
    } else { /* skip to end of text */
      for (; strcmp(p->tok.text, "#elif") && strcmp(p->tok.text, "#else") &&
             strcmp(p->tok.text, "#endif");
           advance(p)) {
      }
    }
  }

  /* else */
  if (p->kind == Directive && !strcmp(p->tok.text, "#else")) {
    advance(p);
    if (!cond) {
      for (; strcmp(p->tok.text, "#endif"); advance(p)) {
        if (p->kind == Directive && !strcmp(p->tok.text, "#if")) {
          int j;
          toks_nested_len = cpp_cond_if(&toks_nested, p);
          for (j = 0; j < toks_nested_len; j++) {
            out[len++] = toks_nested[j];
          }
        } else {
          out[len++] = p->tok;
        }
      }
    } else {
      for (; strcmp(p->tok.text, "#endif"); advance(p)) {
      }
    }
  }

  /* #endif */

  if (p->kind != Directive || strcmp(p->tok.text, "#endif")) {
    printf("expected #endif, got %s\n", p->tok.text);
    exit(1);
  }
  advance(p);

  *toks_out = out;
  return len;
}

int cpp_cond(token_t **toks_out, parser_t *p) {
  token_t *out = calloc(p->toks_len, sizeof(token_t));
  int len = 0;

  for (; p->kind != Eof; advance(p)) {
    if (p->kind == Directive && !strcmp(p->tok.text, "#if")) {
      token_t *toks_if;
      int toks_if_len = cpp_cond_if(&toks_if, p);

      int j;
      for (j = 0; j < toks_if_len; j++) {
        out[len++] = toks_if[j];
      }
      continue;
    }
    out[len++] = p->tok;
  }
  out[len++] = p->tok;

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
  case Ident: {
    return 0;
  }
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
