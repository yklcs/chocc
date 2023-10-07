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

int cpp_define_def(parser_t *p, def **defs, int defs_len) {
  int defined = 0;
  token_t *hideset = NULL;
  int hideset_len = 0;
  int macro_cap = 1;

  *defs = realloc(*defs, sizeof(def) * (defs_len + 1));

  if (p->kind == Directive && !strcmp(p->tok.text, "#define")) {
    (*defs)[defs_len].macro = NULL;
    (*defs)[defs_len].params = NULL;
    (*defs)[defs_len].macro_len = 0;
    (*defs)[defs_len].params_len = 0;
    (*defs)[defs_len].id = peek(p, 1);

    /* do not recursively expand */
    hideset = realloc(hideset, sizeof(token_t) * (hideset_len + 1));
    hideset[hideset_len++] = (*defs)[defs_len].id;

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
      int i = 0;

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

      /* do not expand params */
      for (i = 0; i < params_len; i++) {
        hideset = realloc(hideset, sizeof(token_t) * (hideset_len + 1));
        hideset[hideset_len++] = params[i];
      }

      (*defs)[defs_len].kind = FnMacro;
    } else {
      /* #define id macro */

      expect(p, Directive);
      expect(p, Id);

      (*defs)[defs_len].kind = Macro;
    }

    (*defs)[defs_len].macro = calloc(macro_cap, sizeof(token_t));
    for (; p->kind != Lf && p->kind != Eof; advance(p)) {
      token_t *expanded = NULL;
      int expanded_len = 0;
      int k;
      cpp_define_expand(&expanded, &expanded_len, p, *defs, defs_len, hideset,
                        hideset_len);
      for (k = 0; k < expanded_len; k++) {
        if ((*defs)[defs_len].macro_len >= macro_cap) {
          macro_cap *= 2;
          (*defs)[defs_len].macro =
              realloc((*defs)[defs_len].macro, sizeof(token_t) * macro_cap);
        }
        (*defs)[defs_len].macro[(*defs)[defs_len].macro_len++] = expanded[k];
      }
      if (!expanded_len) {
        if ((*defs)[defs_len].macro_len >= macro_cap) {
          macro_cap *= 2;
          (*defs)[defs_len].macro =
              realloc((*defs)[defs_len].macro, sizeof(token_t) * macro_cap);
        }
        (*defs)[defs_len].macro[(*defs)[defs_len].macro_len++] = p->tok;
      }
    }

    defined = 1;
  }

  if (p->kind == Directive && !strcmp(p->tok.text, "#undef")) {
    int i;

    advance(p);
    for (i = 0; i < defs_len; i++) {
      if (!strcmp((*defs)[i].id.text, p->tok.text)) {
        break;
      }
    }
    if (i != defs_len) {
      memmove(*defs + i, *defs + i + 1, (defs_len - i) * sizeof(def));
      defs_len--;
    } else {
      puts("could not #undef");
      exit(1);
    }

    defined = -1;
  }

  return defined;
}

bool cpp_define_expand(token_t **toks, int *toks_len, parser_t *p, def *defs,
                       int defs_len, token_t *hideset, int hideset_len) {
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

      /* build up args */
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
          bool hidden = false;

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

          for (j = 0; j < hideset_len; j++) {
            if (!strcmp(p->tok.text, hideset[j].text)) {
              hidden = true;
              break;
            }
          }
          if (!hidden) {
            cpp_define_expand(&expanded, &expanded_len, p, defs, defs_len,
                              hideset, hideset_len);
          }
          for (j = 0; j < expanded_len; j++) {
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

      /* perform expansion */
      for (j = 0; j < defs[i].macro_len; j++) {
        int k;
        bool hidden = false;

        for (k = 0; k < hideset_len; k++) {
          if (!strcmp(defs[i].macro[j].text, hideset[k].text)) {
            hidden = true;
            break;
          }
        }
        if (hidden) {
          return false;
        }

        /* replace args in macro */
        for (k = 0; k < defs[i].params_len; k++) {
          /* stringification */
          if (defs[i].macro[j].kind == Directive &&
              defs[i].macro[j].column + 1 == defs[i].macro[j + 1].column &&
              !strcmp(defs[i].macro[j + 1].text, defs[i].params[k].text)) {
            int str_len = 0;
            unsigned long str_cap = 1;
            char *str = calloc(str_cap + 1, 1);
            loc pos;
            int l;

            str[str_len++] = '"';
            for (l = 0; l < args[k].len; l++) {
              unsigned long m;
              if (str_len + strlen(args[k].toks[l].text) >= str_cap) {
                str_cap = str_len + strlen(args[k].toks[l].text) + 4;
                str = realloc(str, str_cap + 1);
              }

              for (m = 0; m < strlen(args[k].toks[l].text); m++) {
                if ((args[k].toks[l].kind == String ||
                     args[k].toks[l].kind == Character) &&
                    (args[k].toks[l].text[m] == '\\' ||
                     args[k].toks[l].text[m] == '"')) {
                  str[str_len++] = '\\';
                }
                str[str_len++] = args[k].toks[l].text[m];
              }
              if (l < args[k].len - 1 &&
                  args[k].toks[l].column + strlen(args[k].toks[l].text) !=
                      args[k].toks[l + 1].column) {
                str[str_len++] = ' ';
              }
            }

            str[str_len++] = '"';
            str[str_len] = 0;
            pos.ln = args[k].toks[0].line;
            pos.col = args[k].toks[0].column;

            if (*toks_len >= cap) {
              cap *= 2;
              *toks = realloc(*toks, sizeof(token_t) * cap);
            }
            (*toks)[(*toks_len)++] = new_token(String, pos, str);
            j++;
            break;
          }

          /* concatenation */
          if (defs[i].macro[j].kind == Directive &&
              defs[i].macro[j + 1].kind == Directive &&
              defs[i].macro[j].column + 1 == defs[i].macro[j + 1].column &&
              !strcmp(defs[i].macro[j + 2].text, defs[i].params[k].text)) {
            char *cat_str;
            loc pos;

            int l;

            token_t *prev = (*toks) + *toks_len - 1;
            token_kind_t cat_kind = -1;

            switch (prev->kind) {
            case Number: {
              if (args[k].toks[0].kind == Number) {
                cat_kind = Number;
              }
              break;
            }
            case Id: {
              if (args[k].toks[0].kind == Number) {
                cat_kind = Id;
              } else if (args[k].toks[0].kind == Id) {
                cat_kind = Id;
              }
              break;
            }
            case Assn: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = Eq;
                cat_str = "==";
              }
              break;
            }
            case Plus: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = PlusAssn;
              } else if (args[k].toks[0].kind == Plus) {
                cat_kind = PlusPlus;
              }
              break;
            }
            case Minus: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = MinusAssn;
              } else if (args[k].toks[0].kind == MinusMinus) {
                cat_kind = MinusMinus;
              }
              break;
            }
            case Star: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = StarAssn;
              }
              break;
            }
            case Slash: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = SlashAssn;
              }
              break;
            }
            case Percent: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = PercentAssn;
              }
              break;
            }
            case Amp: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = AmpAssn;
              } else if (args[k].toks[0].kind == Amp) {
                cat_kind = AmpAmp;
              }
              break;
            }
            case Bar: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = BarAssn;
              } else if (args[k].toks[0].kind == Bar) {
                cat_kind = BarBar;
              }
              break;
            }
            case Caret: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = CaretAssn;
              }
              break;
            }
            case Lt: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = Leq;
              } else if (args[k].toks[0].kind == Lt) {
                cat_kind = LShft;
              } else if (args[k].toks[0].kind == Leq) {
                cat_kind = LShftAssn;
              }
              break;
            }
            case LShft: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = LShftAssn;
              }
              break;
            }
            case Gt: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = Geq;
              } else if (args[k].toks[0].kind == Gt) {
                cat_kind = RShft;
              } else if (args[k].toks[0].kind == Geq) {
                cat_kind = RShft;
              }
              break;
            }
            case RShft: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = RShftAssn;
              }
              break;
            }
            case Exclaim: {
              if (args[k].toks[0].kind == Assn) {
                cat_kind = Neq;
              }
              break;
            }
            default:
              cat_kind = -1;
            }
            if (cat_kind < 0) {
              puts("invalid cpp concatenation tokens");
              exit(1);
            }

            cat_str = calloc(
                strlen(prev->text) + strlen(args[k].toks[0].text) + 1, 1);
            strcpy(cat_str, prev->text);
            strcpy(cat_str + strlen(prev->text), args[k].toks[0].text);

            pos.col = prev->column;
            pos.ln = prev->line;
            if (*toks_len >= cap) {
              cap *= 2;
              *toks = realloc(*toks, sizeof(token_t) * cap);
            }
            (*toks)[(*toks_len) - 1] = new_token(cat_kind, pos, cat_str);

            for (l = 1; l < args[k].len; l++) {
              if (*toks_len >= cap) {
                cap *= 2;
                *toks = realloc(*toks, sizeof(token_t) * cap);
              }
              (*toks)[(*toks_len)++] = args[k].toks[l];
            }
            j += 2;
            break;
          }

          /* macro id matches param id */
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
        /* no replacement */
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
      bool hidden = false;
      int j;
      for (j = 0; j < hideset_len; j++) {
        if (!strcmp(defs[i].id.text, hideset[j].text)) {
          hidden = true;
          break;
        }
      }
      if (hidden) {
        return false;
      }

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
    expanded = cpp_define_expand(&out, &out_len, p, defs, defs_len, NULL, 0);

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
