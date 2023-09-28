#include "parse.h"
#include "chocc.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_type(type *t) {
  switch (t->kind) {
  case PtrT: {
    printf("*");
    if (t->inner->kind == FnT || t->inner->kind == ArrT) {
      printf("(");
    }
    print_type(t->inner);
    if (t->inner->kind == FnT || t->inner->kind == ArrT) {
      printf(")");
    }
    return;
  }
  case ArrT: {
    print_type(t->inner);
    printf("[");
    if (t->arr_size) {
      printf("%d", t->arr_size);
    }
    printf("]");
    return;
  }
  case FnT: {
    int i;
    printf("(");
    for (i = 0; i < t->fn_param_decls_len; i++) {
      ast_node_t *decl = t->fn_param_decls + i;
      if (decl->u.decl.name) {
        printf("%s: ", decl->u.decl.name->u.ident);
      }
      print_type(decl->u.decl.type);
      if (i != t->fn_param_decls_len - 1) {
        printf(", ");
      }
    }
    printf(") -> ");
    print_type(t->inner);
    return;
  }
  case NumericT: {
    printf("%s", token_kind_map[t->numeric.base]);
    return;
  }
  case VoidT: {
    printf("Void");
    return;
  }
  default:
    break;
  }
}

void print_ast(ast_node_t *root, int depth) {
  int i;

  for (i = 0; i < depth; i++) {
    printf(" ");
  }
  if (depth) {
    printf("`-");
  }

  switch (root->kind) {
  case Ident: {
    printf("\033[1mIdent\033[0m: %s\n", root->u.ident);
    break;
  }
  case FnDefn: {
    printf("\033[1mFnDefn\033[0m ");
    if (root->u.fn_defn.decl->name) {
      printf("%s", root->u.fn_defn.decl->name->u.ident);
    }
    print_type(root->u.fn_defn.decl->type);
    puts("");
    print_ast(root->u.fn_defn.body, depth);
    break;
  }
  case BlockStmt: {
    int i;
    for (i = 0; i < root->u.block_stmt.items_len; i++) {
      print_ast(root->u.block_stmt.items + i, depth + 1);
    }
    break;
  }
  case Lit: {
    printf("\033[1mLit\033[0m: %d\n", root->u.lit.value);
    break;
  }
  case Decl: {
    printf("\033[1mDecl\033[0m ");
    if (root->u.decl.name) {
      printf("%s: ", root->u.decl.name->u.ident);
    }
    print_type(root->u.decl.type);
    puts("");
    break;
  }
  case Tok: {
    printf("Tok: %s [%s]\n", root->u.tok.text,
           token_kind_map[root->u.tok.kind]);
    break;
  }
  case Expr: {
    switch (root->u.expr.kind) {
    case PrefixExpr: {
      printf("\033[1mPrefixExpr\033[0m: %s\n", token_kind_map[root->u.expr.op]);
      print_ast(root->u.expr.rhs, depth + 1);
      break;
    }
    case PostfixExpr: {
      printf("\033[1mPostfixExpr\033[0m: %s\n",
             token_kind_map[root->u.expr.op]);
      print_ast(root->u.expr.lhs, depth + 1);
      break;
    }
    case InfixExpr: {
      printf("\033[1mInfixExpr\033[0m: %s\n", token_kind_map[root->u.expr.op]);
      print_ast(root->u.expr.lhs, depth + 1);
      if (root->u.expr.op == Question) {
        print_ast(root->u.expr.mhs, depth + 1);
      }
      print_ast(root->u.expr.rhs, depth + 1);
      break;
    }
    case CommaExpr: {
      int i;
      puts("\033[1mCommaExpr\033[0m");
      for (i = 0; i < root->u.expr.mhs_len; i++) {
        print_ast(root->u.expr.mhs + i, depth + 1);
      }
      break;
    }
    case CallExpr:
      puts("\033[1mCallExpr\033[0m");
      print_ast(root->u.expr.lhs, depth + 1);
      print_ast(root->u.expr.rhs, depth + 1);
      break;
    }
    break;
  }
  default: {
    printf("\033[1m%s\033[0m\n ", ast_node_kind_map[root->kind]);
  }
  }
}

void append_node(ast_node_t **parent, int *len, int *cap, ast_node_t child) {
  if (parent == NULL) {
    return;
  }

  if (*cap == 0) {
    *cap = 16;
    *parent = calloc(*cap, sizeof(ast_node_t));
  }
  if (*cap <= *len + 1) {
    *cap *= 2;
    *parent = realloc(*parent, sizeof(ast_node_t) * *cap);
  }

  (*parent)[(*len)++] = child;
}

void throw(parser_t * parser) {
  printf("parsing error at %s [%d:%d]\n", parser->tok.text, parser->tok.line,
         parser->tok.column);
  exit(1);
}

void expect(parser_t *parser, token_kind_t kind) {
  if (parser->tok.kind == kind) {
    advance(parser);
  } else {
    printf("expected %s, got %s\n", token_kind_map[kind], parser->tok.text);
    throw(parser);
  }
}

void set_pos(parser_t *parser, int pos) {
  parser->pos = pos;
  parser->tok = parser->tokens->tokens[pos];
  parser->kind = parser->tok.kind;
}

void advance(parser_t *parser) {
  set_pos(parser, ++parser->pos);
}

ast_node_t *new_node(ast_node_kind_t kind) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = kind;
  return node;
}

ast_decl *decl(struct ast_node_t *decl_specs, struct ast_node_t *decltor) {
  ast_decl *d = calloc(1, sizeof(ast_decl));
  type *t = NULL;

  /* Since types are outside-in and decltors are inside-out, decltors are
   * inverted recursively with a stack */

  ast_node_t **stack =
      calloc(256, sizeof(ast_node_t *)); /* worry about size later */
  ast_node_t **stack_top = stack;
  ast_node_t *cur;
  type *prev = NULL;

  for (cur = decltor; cur->kind == Decltor && cur->u.decltor.inner;
       cur = cur->u.decltor.inner) {
    if (cur->u.decltor.kind != AbstDecltor) {
      *stack_top++ = cur;
    }
  }

  for (; stack < stack_top;) {
    ast_node_t *top = *--stack_top;
    t = calloc(1, sizeof(type));

    switch (top->u.decltor.kind) {
    case IdentDecltor: {
      d->name = top->u.decltor.inner;
      break;
    }
    case PtrDecltor: {
      t->kind = PtrT;
      if (prev) {
        prev->inner = t;
      }
      break;
    }
    case ArrDecltor: {
      t->kind = ArrT;
      if (top->u.decltor.data.arr_size) {
        t->arr_size = top->u.decltor.data.arr_size->u.lit.value;
      } else {
        t->arr_size = 0;
      }
      if (prev) {
        prev->inner = t;
      }
      break;
    }
    case FnDecltor: {
      int i;
      t->kind = FnT;
      for (i = 0; i < top->u.decltor.data.params.decl_specs_len; i++) {
        ast_node_t *param = new_node(Decl);
        param->u.decl = *decl(top->u.decltor.data.params.decl_specs + i,
                              top->u.decltor.data.params.decltors + i);
        append_node(&t->fn_param_decls, &t->fn_param_decls_len,
                    &t->fn_param_decls_cap, *param);
      }
      if (top->u.decltor.data.params.decl_specs_len == 0) {
        t->fn_param_decls = new_node(Decl);
        t->fn_param_decls->u.decl.type = calloc(1, sizeof(type));
        t->fn_param_decls->u.decl.type->kind = VoidT;
        t->fn_param_decls_len = 1;
      }

      if (prev) {
        prev->inner = t;
      }
      break;
    }
    case GroupDecltor: {
      t = prev;
      break;
    }
    default:
      break;
    };

    if (!d->type && top->u.decltor.kind != IdentDecltor &&
        top->u.decltor.kind != AbstDecltor) {
      d->type = t;
    }

    prev = t;
  }

  /* DeclSpecs form the innermost type */

  if (decl_specs) {
    ast_node_t *specs = decl_specs->u.decl_specs.specs;
    int i;

    t = calloc(1, sizeof(type));
    for (i = 0; i < decl_specs->u.decl_specs.specs_len; i++) {
      token_t tok = specs[i].u.tok;

      switch (tok.kind) {
      case Char:
      case Int:
      case Float:
      case Double: {
        t->kind = NumericT;
        t->numeric.base = tok.kind;
        break;
      }
      case Void: {
        t->kind = VoidT;
        break;
      }
      default:
        break;
      }
    }

    if (prev) {
      prev->inner = t;
    }
    if (!d->type) {
      d->type = t;
    }
  }

  return d;
}

ast_node_t *parse_ident(parser_t *p) {
  ast_node_t *node = new_node(Ident);

  node->u.ident = malloc(sizeof(p->tok.text) + 1);
  strcpy(node->u.ident, p->tok.text);
  expect(p, Id);

  return node;
}

ast_node_t *parse_into_ident(parser_t *p) {
  ast_node_t *node = new_node(Ident);

  node->u.ident = malloc(sizeof(p->tok.text) + 1);
  strcpy(node->u.ident, p->tok.text);
  advance(p);

  return node;
}

ast_node_t *parse_fn_defn(parser_t *p) {
  ast_node_t *node = new_node(FnDefn);

  ast_node_t *decl_specs = parse_decl_specs(p);
  ast_node_t *decltor = parse_decltor(p);
  ast_node_t *block_stmt = parse_block_stmt(p);

  ast_fn_defn *fn_defn = &node->u.fn_defn;
  fn_defn->decl = decl(decl_specs, decltor);
  fn_defn->body = block_stmt;

  return node;
}

ast_node_t *parse_decl_specs(parser_t *p) { /* -> ast_decl_specs */
  ast_node_t *node = new_node(DeclSpecs);
  ast_decl_specs *decl_specs = &node->u.decl_specs;

  while (is_decl_spec(p->tok)) {
    ast_node_t spec = {0};
    spec.kind = Tok;
    spec.u.tok = p->tok;
    append_node(&decl_specs->specs, &decl_specs->specs_len,
                &decl_specs->specs_cap, spec);
    advance(p);
  }

  return node;
}

ast_node_t *parse_lit(parser_t *p) {
  ast_node_t *node = new_node(Lit);
  node->u.lit.value = atoi(p->tok.text);
  advance(p);
  return node;
}

ast_node_t *parse_decltor(parser_t *p) { /* -> ast_decltor */
  ast_node_t *node = new_node(Decltor);
  ast_node_t *inner = NULL;
  ast_node_t *outer = NULL;
  ast_decltor *decltor = &node->u.decltor;

  if (p->kind == Star) {
    decltor->kind = PtrDecltor;
    expect(p, Star);
    inner = parse_decltor(p);
    decltor->inner = inner;
    return node;
  }

  if (p->kind == Id) { /* identifier */
    decltor->kind = IdentDecltor;
    inner = parse_ident(p);
    decltor->inner = inner;
  } else if (p->kind == LParen) { /* (declarator) */
    decltor->kind = GroupDecltor;
    expect(p, LParen);
    inner = parse_decltor(p);
    expect(p, RParen);
    decltor->inner = inner;
  } else {
    decltor->kind = AbstDecltor;
  }

  for (outer = NULL; p->kind == LBrack || p->kind == LParen; node = outer) {
    ast_decltor *outer_decltor;
    outer = new_node(Decltor);
    outer_decltor = &outer->u.decltor;
    if (p->kind == LBrack) { /* decltor[] */
      outer_decltor->kind = ArrDecltor;
      expect(p, LBrack);
      if (p->kind != RBrack) {
        outer_decltor->data.arr_size = parse_lit(p);
      }
      expect(p, RBrack);
    } else {
      /* char(*(*x[3])()); */
      outer_decltor->kind = FnDecltor; /* decltor() */
      expect(p, LParen);

      for (; p->kind != RParen;) {
        ast_node_t *param_decl_specs = parse_decl_specs(p);
        ast_node_t *param_decltor = parse_decltor(p);

        append_node(&outer_decltor->data.params.decl_specs,
                    &outer_decltor->data.params.decl_specs_len,
                    &outer_decltor->data.params.decl_specs_cap,
                    *param_decl_specs);
        append_node(&outer_decltor->data.params.decltors,
                    &outer_decltor->data.params.decltors_len,
                    &outer_decltor->data.params.decltors_cap, *param_decltor);

        if (p->kind == Comma) {
          expect(p, Comma);
        }
      }
      expect(p, RParen);
    }
    outer->u.decltor.inner = node;
  }

  return node;
}

bool is_decl_spec(token_t token) {
  switch (token.kind) {
  /*  storage class specifiers */
  case Typedef:
  case Extern:
  case Static:
  case Auto:
  case Register:
    return true;
  /* type specifiers */
  case Void:
  case Char:
  case Short:
  case Int:
  case Long:
  case Float:
  case Double:
  case Signed:
  case Unsigned:
    return true;
  /* type qualifiers */
  case Const:
  case Restrict:
  case Volatile:
    return true;
  /* function specifiers */
  case Inline:
    return true;
  default:
    return false;
  }
}

ast_node_t *parse_block_stmt(parser_t *p) {
  ast_node_t *node = new_node(BlockStmt);
  expect(p, LBrace);

  for (; p->tok.kind != RBrace;) {
    ast_node_t *item = NULL;
    if (is_decl_spec(p->tok)) {
      item = parse_decl(p);
    } else {
      item = parse_expr(p);
      expect(p, Semi);
    }
    append_node(&node->u.block_stmt.items, &node->u.block_stmt.items_len,
                &node->u.block_stmt.items_cap, *item);
  }

  expect(p, RBrace);
  return node;
}

ast_node_t *parse_decl(parser_t *p) {
  ast_node_t *node = new_node(Decl);

  node->u.decl = *decl(parse_decl_specs(p), parse_decltor(p));

  expect(p, Semi);

  return node;
}

ast_node_t *expr(parser_t *p, int min_bp) {
  ast_node_t *lhs = new_node(Expr);

  switch (p->kind) {
  case Id: {
    lhs = parse_ident(p);
    break;
  }
  case Number:
  case String: {
    lhs = parse_lit(p);
    break;
  }
  case LParen: { /* group or cast */
    advance(p);
    if (p->kind == Int) { /* TODO: parse type casts properly */
      expr_power power = expr_power_prefix(LParen);
      advance(p);
      expect(p, RParen);
      lhs->u.expr.kind = PrefixExpr;
      lhs->u.expr.op = LParen;
      lhs->u.expr.rhs = expr(p, power.right);
    } else {
      /* TODO: add sizeof(type-name) */
      lhs = parse_expr(p);
      expect(p, RParen);
    }
    break;
  }
  case PlusPlus:
  case MinusMinus:
  case Amp:
  case Star:
  case Plus:
  case Minus:
  case Tilde:
  case Exclaim:
  case Sizeof: { /* prefix */
    token_kind_t op = p->kind;
    expr_power power = expr_power_prefix(op);
    advance(p);

    lhs->u.expr.kind = PrefixExpr;
    lhs->u.expr.op = op;
    /* TODO: add sizeof(type-name) */
    lhs->u.expr.rhs = expr(p, power.right);
    break;
  }
  default:
    break;
  }

  for (;;) {
    expr_power power = expr_power_postfix(p->kind);
    if (power.left) { /* postfix */
      ast_node_t *lhs_old = lhs;
      token_kind_t op = p->kind;

      if (power.left < min_bp) {
        break;
      }

      advance(p);

      lhs = new_node(Expr);
      lhs->u.expr.kind = PostfixExpr;
      lhs->u.expr.op = op;
      lhs->u.expr.lhs = lhs_old;

      if (op == Dot || op == Arrow) {
        lhs->u.expr.rhs = parse_ident(p);
      } else if (op == LParen) {
        lhs->u.expr.kind = CallExpr;
        lhs->u.expr.rhs = parse_expr(p);
        expect(p, RParen);
      } else if (op == LBrack) {
        lhs->u.expr.rhs = expr(p, 0);
        expect(p, RBrack);
      }

      continue;
    }

    power = expr_power_infix(p->kind);
    if (power.left && power.right) {
      ast_node_t *lhs_old = lhs;
      token_kind_t op = p->kind;

      if (power.left < min_bp) {
        break;
      }

      advance(p);

      lhs = new_node(Expr);
      lhs->u.expr.kind = InfixExpr;
      lhs->u.expr.op = op;
      lhs->u.expr.lhs = lhs_old;

      if (op == Question) {
        lhs->u.expr.mhs = expr(p, 0);
        expect(p, Colon);
      }
      lhs->u.expr.rhs = expr(p, power.right);

      continue;
    }

    break;
  }

  return lhs;
}

expr_power expr_power_prefix(token_kind_t op) {
  expr_power power = {0};
  switch (op) {
  case PlusPlus:
  case MinusMinus:
  case Plus:
  case Minus:
  case Amp:
  case Star:
  case Tilde:
  case Exclaim:
  case Sizeof:
  case LParen:
    power.right = 27;
    break;
  default:
    break;
  }
  return power;
}

expr_power expr_power_infix(token_kind_t op) {
  expr_power power = {0};
  switch (op) {
  case Star:
  case Slash:
  case Percent:
    power.right = 26;
    power.left = 25;
    break;
  case Plus:
  case Minus:
    power.right = 24;
    power.left = 23;
    break;
  case LShft:
  case RShft:
    power.right = 22;
    power.left = 21;
    break;
  case Lt:
  case Leq:
  case Gt:
  case Geq:
    power.right = 20;
    power.left = 19;
    break;
  case Eq:
  case Neq:
    power.right = 18;
    power.left = 17;
    break;
  case Amp:
    power.right = 16;
    power.left = 15;
    break;
  case Caret:
    power.right = 14;
    power.left = 13;
    break;
  case Bar:
    power.right = 12;
    power.left = 11;
    break;
  case AmpAmp:
    power.right = 10;
    power.left = 9;
    break;
  case BarBar:
    power.right = 8;
    power.left = 7;
    break;
  case Question:
    power.right = 5;
    power.left = 6;
    break;
  case Assn:
  case PlusAssn:
  case MinusAssn:
  case StarAssn:
  case SlashAssn:
  case PercentAssn:
  case LShftAssn:
  case RShftAssn:
  case AmpAssn:
  case CaretAssn:
  case BarAssn:
    power.right = 3;
    power.left = 4;
    break;
  default:
    break;
  }
  return power;
}

expr_power expr_power_postfix(token_kind_t op) {
  expr_power power = {0};
  switch (op) {
  case PlusPlus:
  case MinusMinus:
  case LParen:
  case LBrack:
  case Dot:
  case Arrow:
    power.left = 28;
    break;
  default:
    break;
  }
  return power;
}

ast_node_t *parse_expr(parser_t *p) {
  ast_node_t *root = expr(p, 0);
  ast_node_t *root_comma;

  if (p->kind != Comma) {
    return root;
  }

  root_comma = new_node(Expr);
  root_comma->u.expr.kind = CommaExpr;
  root_comma->u.expr.op = Comma;
  append_node(&root_comma->u.expr.mhs, &root_comma->u.expr.mhs_len,
              &root_comma->u.expr.mhs_cap, *root);
  for (; p->kind == Comma;) {
    expect(p, Comma);
    append_node(&root_comma->u.expr.mhs, &root_comma->u.expr.mhs_len,
                &root_comma->u.expr.mhs_cap, *expr(p, 0));
  }

  return root_comma;
}

ast_node_t **parse(parser_t *p) {
  ast_node_t **nodes = calloc(128, sizeof(ast_node_t *));

  int i;
  for (i = 0; p->kind != Eof; i++) {
    ast_node_t *node = NULL;

    int pos = p->pos;
    parse_decl_specs(p);
    parse_decltor(p);

    if (p->kind == LBrace) { /* FnDefn */
      set_pos(p, pos);
      node = parse_fn_defn(p);
    } else if (p->kind == Semi) { /* Decl */
      set_pos(p, pos);
      node = parse_decl(p);
    } else {
      throw(p);
    }

    nodes[i] = node;
  }
  nodes[i] = NULL;

  return nodes;
}

const char *ast_node_kind_map[] = {"Ident",     "Lit",     "FnDefn",
                                   "DeclSpecs", "Decltor", "Decl",
                                   "BlockStmt", "Tok",     "Expr"};
