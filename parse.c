#include "parse.h"
#include "chocc.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *ast_decltor_kind_map[DECLTOR_KINDS] = {
    [IdentDecltor] = "Ident", [PtrDecltor] = "Ptr",   [FnDecltor] = "Fn",
    [ArrDecltor] = "Arr",     [AbstDecltor] = "Abst", [GroupDecltor] = "Group"};

void print_type(type *t, int depth) {
  int i;

  for (i = 0; i < depth; i++) {
    printf(" ");
  }

  switch (t->kind) {
  case PtrT:
    printf("Pointer %p:\n", (void *)t);
    print_type(t->inner, depth + 1);
    break;
  case NumericT:
    printf("Numeric %p: %s\n", (void *)t, token_kind_map[t->numeric.base]);
    break;
  default:
    break;
  }
}

void print_ast(ast_node_t *root, int depth) {
  int i;
  int j;

  for (i = 0; i < depth; i++) {
    printf(" ");
  }

  switch (root->kind) {
  case Ident: {
    printf("Ident: %s\n", root->data.ident.name);
    break;
  }
  case FnDefn: {
    puts("FnDefn:");
    print_ast(root->data.fn_defn.decl_specs, depth + 1);
    print_ast(root->data.fn_defn.decltor, depth + 1);
    print_ast(root->data.fn_defn.body, depth + 1);
    break;
  }
  case DeclSpecs: {
    puts("DeclSpecs:");
    for (i = 0; i < root->data.decl_specs.specs_len; i++) {
      print_ast(root->data.decl_specs.specs + i, depth + 1);
    }
    break;
  }
  case Decltor: {
    printf("Decltor: %s\n", ast_decltor_kind_map[root->data.decltor.kind]);
    if (root->data.decltor.kind == ArrDecltor) {
      for (i = 0; i < depth + 1; i++) {
        printf(" ");
      }
      puts("Size:");
      if (root->data.decltor.data.arr_size != NULL) {
        print_ast(root->data.decltor.data.arr_size, depth + 2);
      }
    } else if (root->data.decltor.kind == FnDecltor) {
      for (i = 0; i < depth + 1; i++) {
        printf(" ");
      }
      puts("Params:");
      for (i = 0; i < root->data.decltor.data.params.decl_specs_len; i++) {
        for (j = 0; j < depth + 2; j++) {
          printf(" ");
        }
        printf("%d: \n", i);
        print_ast(root->data.decltor.data.params.decl_specs + i, depth + 3);
        print_ast(root->data.decltor.data.params.decltors + i, depth + 3);
      }
      for (i = 0; i < depth + 1; i++) {
        printf(" ");
      }
      puts("Inner:");
      print_ast(root->data.decltor.inner, depth + 3);
    default:
      break;
    }

    if (root->data.decltor.kind != AbstDecltor) {
      print_ast(root->data.decltor.inner, depth + 1);
    }

    break;
  }
  case BlockStmt: {
    puts("BlockStmt");
    break;
  }
  case Lit: {
    printf("Lit: %d\n", root->data.lit.value);
    break;
  }
  case Decl: {
    puts("Decl:");
    print_type(root->data.decl.type, depth + 1);
    /*     print_ast(root->data.decl.decl_specs, depth + 1);
        print_ast(root->data.decl.decltor, depth + 1); */
    break;
  }
  case Tok: {
    printf("Tok: %s [%s]\n", root->data.tok.tok.text,
           token_kind_map[root->data.tok.tok.kind]);
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

type *parse_type(struct ast_node_t *decl_specs, struct ast_node_t *decltor) {
  type *t;

  type *topt;
  /* DeclSpecs form the innermost type, so parse first */

  /* Since types are outside-in and decltors are inside-out, decltors are
   * inverted recursively with a stack */

  ast_node_t **stack =
      calloc(256, sizeof(ast_node_t *)); /* worry about size later */
  ast_node_t **stack_top = stack;
  ast_node_t *cur;
  type *prev = NULL;

  for (cur = decltor; cur->kind == Decltor && cur->data.decltor.inner;
       cur = cur->data.decltor.inner) {
    *stack_top++ = cur;
  }

  for (; stack < stack_top;) {
    ast_node_t *top = *--stack_top;
    t = calloc(1, sizeof(type));
    switch (top->data.decltor.kind) {
    case PtrDecltor:
      t->kind = PtrT;
      prev->inner = t;
      printf("ptr: %p -> %p\n", (void *)prev, (void *)t);
      break;
    default:
      break;
    };

    if (!topt && top->data.decltor.kind != IdentDecltor &&
        top->data.decltor.kind != AbstDecltor) {
      topt = t;
    }
    prev = t;
  }

  if (decl_specs) {
    ast_node_t *specs = decl_specs->data.decl_specs.specs;
    int i;

    t = calloc(1, sizeof(type));
    for (i = 0; i < decl_specs->data.decl_specs.specs_len; i++) {
      token_t tok = specs[i].data.tok.tok;
      switch (tok.kind) {
      case Char:
      case Int:
      case Float:
      case Double: {
        t->kind = NumericT;
        t->numeric.base = tok.kind;
        break;
      }
      default:
        break;
      }
    }

    prev->inner = t;
    printf("decl_specs: %p -> %p\n", (void *)prev, (void *)t);
    printf("topt: %p\n", (void *)topt);

    return topt;
  }

  printf("topt: %p\n", (void *)topt);
  printf("topt->inner: %p\n", (void *)topt->inner);

  return topt;
}

ast_node_t *parse_ident(parser_t *p) {
  ast_node_t *node = new_node(Ident);
  ast_ident *ident = &node->data.ident;

  ident->name = malloc(sizeof(p->tok.text) + 1);
  strcpy(ident->name, p->tok.text);
  expect(p, Id);
  return node;
}

ast_node_t *parse_into_ident(parser_t *p) {
  ast_node_t *node = new_node(Ident);
  ast_ident *ident = &node->data.ident;

  ident->name = malloc(sizeof(p->tok.text) + 1);
  strcpy(ident->name, p->tok.text);
  advance(p);

  return node;
}

ast_node_t *parse_fn_defn(parser_t *p) {
  ast_node_t *node = new_node(FnDefn);
  ast_node_t *decl_specs = parse_decl_specs(p);
  ast_node_t *decltor = parse_decltor(p);
  ast_node_t *block_stmt = parse_block_stmt(p);

  ast_fn_defn *fn_defn = &node->data.fn_defn;
  fn_defn->decl_specs = decl_specs;
  fn_defn->decltor = decltor;
  fn_defn->body = block_stmt;

  return node;
}

ast_node_t *parse_decl_specs(parser_t *p) { /* -> ast_decl_specs */
  ast_node_t *node = new_node(DeclSpecs);
  ast_decl_specs *decl_specs = &node->data.decl_specs;

  while (is_decl_spec(p->tok)) {
    ast_node_t spec = {0};
    spec.kind = Tok;
    spec.data.tok.tok = p->tok;
    append_node(&decl_specs->specs, &decl_specs->specs_len,
                &decl_specs->specs_cap, spec);
    advance(p);
  }

  return node;
}

ast_node_t *parse_lit(parser_t *p) {
  ast_node_t *node = new_node(Lit);
  node->data.lit.value = atoi(p->tok.text);
  advance(p);
  return node;
}

ast_node_t *parse_decltor(parser_t *p) { /* -> ast_decltor */
  ast_node_t *node = new_node(Decltor);
  ast_node_t *inner = NULL;
  ast_node_t *outer = NULL;
  ast_decltor *decltor = &node->data.decltor;

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
    outer_decltor = &outer->data.decltor;
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
    outer->data.decltor.inner = node;
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
  while (p->kind != RBrace) {
    advance(p);
  }
  expect(p, RBrace);
  return node;
}

ast_node_t *parse_decl(parser_t *p) {
  ast_node_t *node = new_node(Decl);
  ast_decl *decl = &node->data.decl;

  decl->decl_specs = parse_decl_specs(p);
  decl->decltor = parse_decltor(p);
  decl->type = parse_type(decl->decl_specs, decl->decltor);

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
