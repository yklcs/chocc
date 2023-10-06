#include "cpp.h"
#include "chocc.h"
#include "lex.h"
#include "parse.h"

#include <malloc/_malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_null.h>

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

typedef enum def_kind { Blank, Macro, FnMacro } def_kind;

typedef struct def {
  token_t id;

  token_t *macro;
  int macro_len;

  token_t *params;
  int params_len;

  def_kind kind;
} def;

bool cpp_define_def(parser_t *p, def **defs, int defs_len) {
  bool defined = false;
  *defs = realloc(*defs, sizeof(def) * (defs_len + 1));

  if (p->kind == Directive && !strcmp(p->tok.text, "#define")) {
    (*defs)[defs_len].macro = NULL;
    (*defs)[defs_len].params = NULL;
    (*defs)[defs_len].macro_len = 0;
    (*defs)[defs_len].params_len = 0;
    (*defs)[defs_len].id = peek(p, 1);

    if (peek(p, 1).kind == Id && peek(p, 2).kind == Lf) {
      /* #define id */
      (*defs)[defs_len].kind = Blank;
      advance(p);
    } else if (peek(p, 2).kind == LParen &&
               peek(p, 2).column ==
                   peek(p, 1).column + strlen(peek(p, 1).text)) {
      /* #define id(...) macro */
      int params_cap = 1;
      int params_len = 0;
      token_t *params = calloc(params_cap, sizeof(token_t));
      int k;

      expect(p, Directive);
      expect(p, Id);
      expect(p, LParen);
      for (; p->kind != RParen;) {
        if (params_len == params_cap) {
          params_cap *= 2;
          params = realloc(params, sizeof(token_t) * params_cap);
        }

        params[params_len++] = p->tok;
        expect(p, Id);
        if (p->kind != Comma) {
          break;
        }
        expect(p, Comma);
      }
      (*defs)[defs_len].params = params;
      (*defs)[defs_len].params_len = params_len;
      expect(p, RParen);

      for (k = 0; peek(p, k).kind != Lf &&
                  peek(p, k).kind != Eof;) { /*  skip to end of line */
        k++;
      }
      (*defs)[defs_len].macro = calloc(k, sizeof(token_t));
      (*defs)[defs_len].macro_len = k;
      memcpy((*defs)[defs_len].macro, p->toks + p->pos, k * sizeof(token_t));
      set_pos(p, p->pos + k);
      (*defs)[defs_len].kind = FnMacro;
    } else {
      /* #define id macro */
      token_t *macro_toks = p->toks + p->pos + 2;
      int k;
      for (k = 0; macro_toks[k].kind != Lf &&
                  macro_toks[k].kind != Eof;) { /*  skip to end of line */
        k++;
      }
      (*defs)[defs_len].macro = calloc(k, sizeof(token_t));
      (*defs)[defs_len].macro_len = k;
      memcpy((*defs)[defs_len].macro, macro_toks, k * sizeof(token_t));
      set_pos(p, p->pos + k + 1);
      (*defs)[defs_len].kind = Macro;
    }

    defined = true;
  }

  return defined;
}

bool cpp_define_expand(token_t **toks, int *toks_len, parser_t *p, def *defs,
                       int defs_len) {
  int cap = *toks_len + 1;
  struct arg {
    token_t *toks;
    int len;
    int cap;
  };
  int i;
  *toks = realloc(*toks, sizeof(token_t) * cap);
  for (i = 0; i < defs_len; i++) {
    if (!strcmp(defs[i].id.text, p->tok.text) && peek(p, 1).kind == LParen &&
        defs[i].kind == FnMacro) {
      int j;
      int args_len = 0;
      int args_cap = 1;
      struct arg *args = calloc(args_cap, sizeof(struct arg));
      int stack = 0;

      expect(p, Id);
      expect(p, LParen);
      for (; p->kind != RParen;) {
        struct arg a = {0};
        a.cap = 1;
        a.toks = calloc(a.cap, sizeof(token_t));

        if (args_len == args_cap) {
          args_cap *= 2;
          args = realloc(args, sizeof(struct arg) * args_cap);
        }

        for (;; advance(p)) {
          token_t *expanded = NULL;
          int expanded_len = 0;

          if (a.len == a.cap) {
            a.cap *= 2;
            a.toks = realloc(a.toks, sizeof(token_t) * a.cap);
          }
          if (!stack && (p->kind == Comma || p->kind == RParen)) {
            break;
          }

          if (p->kind == LParen) {
            stack++;
          }
          if (p->kind == RParen) {
            stack--;
          }

          cpp_define_expand(&expanded, &expanded_len, p, defs, defs_len);
          for (j = 0; j < expanded_len; j++) {
            print_token(expanded[j]);
            if (a.len == a.cap) {
              a.cap *= 2;
              a.toks = realloc(a.toks, sizeof(token_t) * a.cap);
            }
            a.toks[a.len++] = expanded[j];
          }
          if (!expanded_len) {
            a.toks[a.len++] = p->tok;
          }
        }

        args[args_len++] = a;
        if (p->kind != Comma) {
          break;
        }
        expect(p, Comma);
      }

      for (j = 0; j < defs[i].macro_len; j++) {
        int k;

        /* replace args in macro */
        for (k = 0; k < defs[i].params_len; k++) {
          if (!strcmp(defs[i].macro[j].text, defs[i].params[k].text)) {
            int l;
            for (l = 0; l < args[k].len; l++) {
              if (*toks_len >= cap) {
                cap *= 2;
                *toks = realloc(*toks, sizeof(token_t) * cap);
              }
              (*toks)[(*toks_len)++] = args[k].toks[l];
            }
            break;
          }
        }

        if (k == defs[i].params_len) {
          if (*toks_len >= cap) {
            cap *= 2;
            *toks = realloc(*toks, sizeof(token_t) * cap);
          }
          (*toks)[(*toks_len)++] = defs[i].macro[j];
        }
      }
      return true;
    } else if (!strcmp(defs[i].id.text, p->tok.text) && defs[i].macro_len &&
               defs[i].kind == Macro) {
      if (*toks_len + defs[i].macro_len >= cap) {
        cap = *toks_len + defs[i].macro_len;
        *toks = realloc(*toks, sizeof(token_t) * cap);
      }
      memcpy(*toks + *toks_len, defs[i].macro,
             sizeof(token_t) * defs[i].macro_len);
      *toks_len += defs[i].macro_len;
      return true;
    } else if (!strcmp(defs[i].id.text, p->tok.text) && defs[i].kind == Blank) {
      return true;
    }
  }

  return false;
}

int cpp_define(token_t **toks_out, parser_t *p) {
  int out_len = 0;
  int out_cap = p->toks_len;
  token_t *out = calloc(out_cap, sizeof(token_t));

  int defs_len = 0;
  def *defs = NULL;

  bool cpp_line = false;

  for (; p->kind != Eof; advance(p)) {
    bool defined = false;
    bool expanded = false;

    if (p->kind == Directive) {
      cpp_line = true;
    }
    if (p->kind == Lf) {
      cpp_line = false;
    }

    /* perform definition */
    defined = cpp_define_def(p, &defs, defs_len);
    defs_len += defined;

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
          if (!strcmp(defs[j].id.text, target)) {
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
    expanded = cpp_define_expand(&out, &out_len, p, defs, defs_len);

    if (!defined && !expanded) {
      out = realloc(out, sizeof(token_t) * (out_len + 8));
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
