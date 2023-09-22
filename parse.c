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

  if (depth) {
    printf(" `");
  }
  for (i = 0; i < depth; i++) {
    printf("-");
  }

  switch (root->kind) {
  case Ident: {
    printf("Ident: %s\n", root->u.ident);
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
    printf("Lit: %d\n", root->u.lit.value);
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
  default: {
    printf("\033[1m%s\033[0m ", ast_node_kind_map[root->kind]);
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

const char *ast_node_kind_map[] = {"Ident",   "Lit",  "FnDefn",    "DeclSpecs",
                                   "Decltor", "Decl", "BlockStmt", "Tok"};
