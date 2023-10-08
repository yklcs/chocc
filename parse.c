#include "parse.h"
#include "chocc.h"
#include "lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_type(type *t) {
  if (t->store_class) {
    printf("%s ", token_kind_map[t->store_class]);
  }
  if (t->is_const) {
    printf("Const ");
  }
  if (t->is_volatile) {
    printf("Volatile ");
  }

  switch (t->kind) {
  case PtrT: {
    printf("*");
    if (t->inner->is_const || t->inner->is_volatile) {
      printf(" ");
    }

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
    if (t->inner->kind == PtrT) {
      printf("(");
    }
    print_type(t->inner);
    if (t->inner->kind == PtrT) {
      printf(")");
    }
    printf("[");
    if (t->arr_size) {
      printf("%d", t->arr_size);
    }
    printf("]");
    return;
  }
  case FnT: {
    int i;
    if (t->fn_param_decls_len != 1) {
      printf("(");
    }
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
    if (t->fn_param_decls_len != 1) {
      printf(")");
    }
    printf(" -> ");
    print_type(t->inner);
    return;
  }
  case NumericT: {
    printf("%s", token_kind_map[t->numeric.base]);
    return;
  }
  case UnionT:
  case StructT: {
    ast_node_t *fields = t->struct_fields;
    int i;
    if (t->kind == StructT) {
      printf("Struct ");
    } else {
      printf("Union ");
    }
    if (t->name) {
      printf("%s ", t->name->u.ident);
    }
    printf("{ ");
    for (i = 0; i < fields->u.list.len; i++) {
      ast_decl decl = ast_list_at(fields, i)->u.decl;
      if (decl.name) {
        printf("%s: ", decl.name->u.ident);
      }
      print_type(decl.type);
      if (i != fields->u.list.len - 1) {
        printf(", ");
      }
    }
    printf(" }");
    return;
  }
  case EnumT: {
    ast_node_t *idents = t->enum_idents;
    ast_node_t *exprs = t->enum_exprs;
    int i;
    printf("Enum ");
    if (t->name) {
      printf("%s ", t->name->u.ident);
    }
    printf("{ ");
    for (i = 0; i < idents->u.list.len; i++) {
      printf("%s", ast_list_at(idents, i)->u.ident);

      if (ast_list_at(exprs, i)) {
        printf(" = ?");
      }

      if (i != idents->u.list.len - 1) {
        printf(", ");
      }
    }
    printf(" }");
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

void print_ast(ast_node_t *root, int depth, bool last, char *pad) {
  int i;
  int len_pad;
  char *pad_new;

  for (i = 2; i < depth * 2; i++) {
    printf("%c", pad[i]);
  }

  len_pad = strlen(pad);
  pad_new = calloc(1, len_pad + 3);
  strcpy(pad_new, pad);

  pad = pad_new;
  if (last) {
    pad[len_pad] = ' ';
  } else {
    pad[len_pad] = '|';
  }
  pad[len_pad + 1] = ' ';
  pad[len_pad + 2] = 0;

  if (depth && last) {
    printf("`-");
  } else if (depth && !last) {
    printf("|-");
  }

  switch (root->kind) {
  case Ident: {
    printf("\033[1mIdent\033[0m: %s\n", root->u.ident);
    break;
  }
  case FnDefn: {
    printf("\033[1mFnDefn\033[0m ");
    if (root->u.fn_defn.decl->name) {
      printf("%s ", root->u.fn_defn.decl->name->u.ident);
    }
    print_type(root->u.fn_defn.decl->type);
    puts("");
    print_ast(root->u.fn_defn.body, depth + 1, true, pad);
    break;
  }
  case Stmt: {
    switch (root->u.stmt.kind) {
    case LabelStmt: {
      printf("\033[1mLabelStmt\033[0m ");
      if (root->u.stmt.label->kind == Ident) {
        printf("%s:\n", root->u.stmt.label->u.ident);
      } else {
        printf("%s:\n", token_kind_map[root->u.stmt.label->u.tok.kind]);
      }
      if (root->u.stmt.case_expr) {
        print_ast(root->u.stmt.case_expr, depth + 1, true, pad);
      }
      break;
    }
    case BlockStmt: {
      int i;
      puts("\033[1mBlockStmt\033[0m");
      for (i = 0; i < root->u.stmt.inner->u.list.len; i++) {
        print_ast(ast_list_at(root->u.stmt.inner, i), depth + 1,
                  i == root->u.stmt.inner->u.list.len - 1, pad);
      }
      break;
    }
    case ExprStmt: {
      puts("\033[1mExprStmt\033[0m");
      print_ast(root->u.stmt.inner, depth + 1, true, pad);
      break;
    }
    case IfStmt: {
      puts("\033[1mIfStmt\033[0m");
      print_ast(root->u.stmt.cond, depth + 1, false, pad);
      print_ast(root->u.stmt.inner, depth + 1, true, pad);
      break;
    }
    case IfElseStmt: {
      puts("\033[1mIfElseStmt\033[0m");
      print_ast(root->u.stmt.cond, depth + 1, false, pad);
      print_ast(root->u.stmt.inner, depth + 1, false, pad);
      print_ast(root->u.stmt.inner_else, depth + 1, true, pad);
      break;
    }
    case SwitchStmt: {
      puts("\033[1mSwitchStmt\033[0m");
      print_ast(root->u.stmt.cond, depth + 1, false, pad);
      print_ast(root->u.stmt.inner, depth + 1, true, pad);
      break;
    }
    case WhileStmt: {
      puts("\033[1mWhileStmt\033[0m");
      print_ast(root->u.stmt.cond, depth + 1, false, pad);
      print_ast(root->u.stmt.inner, depth + 1, true, pad);
      break;
    }
    case DoWhileStmt: {
      puts("\033[1mWhileStmt\033[0m");
      print_ast(root->u.stmt.inner, depth + 1, false, pad);
      print_ast(root->u.stmt.cond, depth + 1, true, pad);
      break;
    }
    case ForStmt: {
      puts("\033[1mForStmt\033[0m");
      if (root->u.stmt.init) {
        print_ast(root->u.stmt.init, depth + 1, false, pad);
      }
      if (root->u.stmt.cond) {
        print_ast(root->u.stmt.cond, depth + 1, false, pad);
      }
      if (root->u.stmt.iter) {
        print_ast(root->u.stmt.iter, depth + 1, false, pad);
      }
      print_ast(root->u.stmt.inner, depth + 1, true, pad);
      break;
    }
    case JumpStmt: {
      printf("\033[1mJumpStmt\033[0m");
      printf(" %s\n", token_kind_map[root->u.stmt.jump->u.tok.kind]);
      if (root->u.stmt.inner) {
        print_ast(root->u.stmt.inner, depth + 1, true, pad);
      }
      break;
    }
    default: {
      printf("expr\n");
    }
    }
    break;
  }
  case Lit: {
    printf("\033[1mLit\033[0m: ");
    switch (root->u.lit.kind) {
    case DecLit: {
      printf("%ld\n", root->u.lit.integer);
      break;
    }
    case StrLit: {
      printf("\"%s\"\n", root->u.lit.string);
      break;
    }
    case CharLit: {
      printf("'%s'\n", root->u.lit.character);
      break;
    }
    default: {
      puts("not yet implemented");
      break;
    }
    }
    break;
  }
  case Decl: {
    printf("\033[1mDecl\033[0m");
    if (root->u.decl.name) {
      printf(" %s: ", root->u.decl.name->u.ident);
    }
    print_type(root->u.decl.type);
    puts("");
    if (root->u.decl.init) {
      print_ast(root->u.decl.init, depth + 1, true, pad);
    }
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
      printf("\033[1mPrefixExpr\033[0m: %s", token_kind_map[root->u.expr.op]);
      if (root->u.expr.op == Sizeof && root->u.expr.rhs->kind == TypeName) {
        printf(" (");
        print_type(&root->u.expr.rhs->u.type_name);
        puts(")");
      } else {
        puts("");
        print_ast(root->u.expr.rhs, depth + 1, true, pad);
      }
      break;
    }
    case PostfixExpr: {
      printf("\033[1mPostfixExpr\033[0m: %s\n",
             token_kind_map[root->u.expr.op]);
      print_ast(root->u.expr.lhs, depth + 1, false, pad);
      if (root->u.expr.op == LBrack || root->u.expr.op == Dot ||
          root->u.expr.op == Arrow)
        print_ast(root->u.expr.rhs, depth + 1, true, pad);
      break;
    }
    case InfixExpr: {
      printf("\033[1mInfixExpr\033[0m: %s\n", token_kind_map[root->u.expr.op]);
      print_ast(root->u.expr.lhs, depth + 1, false, pad);
      if (root->u.expr.op == Question) {
        print_ast(root->u.expr.mhs, depth + 1, false, pad);
      }
      print_ast(root->u.expr.rhs, depth + 1, true, pad);
      break;
    }
    case CommaExpr: {
      int i;
      puts("\033[1mCommaExpr\033[0m");
      for (i = 0; i < root->u.expr.mhs_len; i++) {
        print_ast(root->u.expr.mhs + i, depth + 1,
                  i == root->u.expr.mhs_len - 1, pad);
      }
      break;
    }
    case CallExpr: {
      puts("\033[1mCallExpr\033[0m");
      print_ast(root->u.expr.lhs, depth + 1, root->u.expr.rhs == NULL, pad);
      if (root->u.expr.rhs) {
        print_ast(root->u.expr.rhs, depth + 1, true, pad);
      }
      break;
    }
    case CastExpr: {
      printf("\033[1mCastExpr\033[0m (");
      print_type(&root->u.expr.lhs->u.type_name);
      puts(")");
      print_ast(root->u.expr.rhs, depth + 1, true, pad);
    }
    }
    break;
  }
  case List: {
    int i;
    printf("\033[1mList\033[0m (len %d)\n", root->u.list.len);
    for (i = 0; i < root->u.list.len; i++) {
      print_ast(root->u.list.nodes[i], depth + 1, i == root->u.list.len - 1,
                pad);
    }
    break;
  }
  default: {
    printf("\033[1m%s\033[0m\n ", ast_node_kind_map[root->kind]);
  }
  }
}

parser_t new_parser(struct unit *src) {
  parser_t p = {0};
  p.toks = src->toks;
  p.toks_len = src->len;
  set_pos(&p, 0);
  return p;
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

void advance(parser_t *parser) {
  set_pos(parser, ++parser->pos);
}

void set_pos(parser_t *parser, int pos) {
  parser->pos = pos;
  parser->tok = parser->toks[pos];
  parser->kind = parser->tok.kind;
}

token_t peek(parser_t *parser, int delta) {
  return parser->toks[parser->pos + delta];
}

ast_node_t *new_node(ast_node_kind_t kind) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = kind;

  if (kind == List) {
    node->u.list.cap = 16;
    node->u.list.len = 0;
    node->u.list.nodes = calloc(node->u.list.cap, sizeof(ast_node_t *));
  }

  return node;
}

ast_node_t *parse_lit(parser_t *p) {
  ast_node_t *node = new_node(Lit);

  if (p->kind == Number) {
    node->u.lit.kind = DecLit;
    node->u.lit.integer = atoi(p->tok.text);
    advance(p);
  } else if (p->kind == String) {
    int len = strlen(p->tok.text) - 2;
    char *str = calloc(len + 1, 1);
    node->u.lit.kind = StrLit;
    strncpy(str, p->tok.text + 1, len);
    advance(p);

    for (; p->kind == String;) {
      int len_new = strlen(p->tok.text) - 2;
      str = realloc(str, len + len_new + 1);
      strncpy(str + len, p->tok.text + 1, len_new);
      str[len += len_new] = 0;
      advance(p);
    }

    node->u.lit.string = str;
  } else if (p->kind == Character) {
    int len = strlen(p->tok.text) - 2;
    char *str = calloc(len + 1, 1);
    node->u.lit.kind = CharLit;
    strncpy(str, p->tok.text + 1, len);
    node->u.lit.character = str;
    advance(p);
  }

  return node;
}

ast_node_t *parse_ident(parser_t *p) {
  ast_node_t *node = new_node(Ident);

  node->u.ident = malloc(strlen(p->tok.text) + 1);
  strcpy(node->u.ident, p->tok.text);
  expect(p, Id);

  return node;
}

ast_node_t *parse_into_ident(parser_t *p) {
  ast_node_t *node = new_node(Ident);

  node->u.ident = malloc(strlen(p->tok.text) + 1);
  strcpy(node->u.ident, p->tok.text);
  advance(p);

  return node;
}

ast_node_t *parse_fn_defn(parser_t *p) {
  ast_node_t *node = new_node(FnDefn);

  ast_node_t *decl_specs = parse_decl_specs(p);
  ast_node_t *decltor = parse_decltor(p);
  ast_node_t *block_stmt = parse_stmt(p);

  ast_fn_defn *fn_defn = &node->u.fn_defn;
  fn_defn->decl = decl(decl_specs, decltor);
  fn_defn->body = block_stmt;

  return node;
}

ast_node_t *parse_decl_specs(parser_t *p) { /* -> ast_decl_specs */
  ast_node_t *specs = new_node(List);

  while (is_decl_spec(p, p->tok)) {
    ast_node_t *node = new_node(DeclSpecs);
    ast_decl_spec *spec = &node->u.decl_spec;
    spec->tok = p->kind;

    switch (p->kind) {
    case Const:
    case Volatile: {
      spec->kind = TypeQual;
      advance(p);
      break;
    }
    case Typedef:
    case Extern:
    case Static:
    case Auto:
    case Register: {
      spec->kind = StoreClass;
      advance(p);
      break;
    }
    case Id: {
      int i;
      type *alias = NULL;

      for (i = 0; i < p->tdefs_len; i++) {
        puts(p->tdefs[i].u.decl.name->u.ident);
        if (!strcmp(p->tdefs[i].u.decl.name->u.ident, p->tok.text)) {
          alias = p->tdefs[i].u.decl.type;
          break;
        }
      }

      spec->kind = TypeSpec;
      spec->name = parse_ident(p);
      spec->alias = alias;
      break;
    }
    case Void:
    case Char:
    case Short:
    case Int:
    case Long:
    case Float:
    case Double:
    case Signed:
    case Unsigned: {
      spec->kind = TypeSpec;
      advance(p);
      break;
    }
    case Struct:
    case Union: {
      spec->kind = TypeSpec;
      advance(p);
      if (p->kind == Id) {
        spec->name = parse_ident(p);
      }
      if (p->kind == LBrace) {
        ast_node_t *ls;
        expect(p, LBrace);
        ls = parse_decl(p);
        spec->struct_fields = ls;
        expect(p, RBrace);
      }
      break;
    }
    case Enum: {
      spec->kind = TypeSpec;
      advance(p);
      if (p->kind == Id) {
        spec->name = parse_ident(p);
      }

      if (!spec->name) {
        expect(p, LBrace);
      }

      if (p->kind == LBrace || !spec->name) {
        ast_node_t *ls_id = new_node(List);
        ast_node_t *ls_ex = new_node(List);

        if (spec->name) {
          expect(p, LBrace);
        }

        for (; p->kind != RBrace;) {
          ast_node_t *id = parse_ident(p);
          ast_node_t *ex = NULL;
          if (p->kind == Assn) {
            advance(p);
            ex = expr(p, 0);
          }
          ast_list_append(ls_id, id);
          if (ex) {
            ast_list_append(ls_ex, ex);
          } else {
            ast_list_append(ls_ex, NULL);
          }

          if (p->kind != Comma) {
            break;
          }
          expect(p, Comma);
        }

        expect(p, RBrace);

        spec->enum_idents = ls_id;
        spec->enum_exprs = ls_ex;
      }

      break;
    }
    default:
      puts("invalid type");
      throw(p);
    }

    ast_list_append(specs, node);
  }

  if (!specs->u.list.len) {
    puts("empty declaration specifiers");
    throw(p);
  }

  return specs;
}

ast_node_t *parse_decltor(parser_t *p) { /* -> ast_decltor */
  ast_node_t *node = new_node(Decltor);
  ast_node_t *inner = NULL;
  ast_node_t *outer = NULL;
  ast_decltor *decltor = &node->u.decltor;

  if (p->kind == Star) {
    decltor->kind = PtrDecltor;
    expect(p, Star);

    if (p->kind == Const) {
      decltor->is_const = true;
      advance(p);
      if (p->kind == Volatile) {
        decltor->is_volatile = true;
        advance(p);
      }
    } else if (p->kind == Volatile) {
      decltor->is_volatile = true;
      advance(p);
      if (p->kind == Const) {
        decltor->is_const = true;
        advance(p);
      }
    }

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

bool is_decl_spec(parser_t *p, token_t token) {
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
  case Volatile:
    return true;
  /* struct or union */
  case Struct:
  case Union:
    return true;
  /* enum */
  case Enum:
    return true;
  case Id: {
    int i;
    bool tdef = false;
    for (i = 0; i < p->tdefs_len; i++) {
      if (!strcmp(p->tdefs[i].u.decl.name->u.ident, p->tok.text)) {
        tdef = true;
        break;
      }
    }
    return tdef;
  }
  default:
    return false;
  }
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
      t->is_const = top->u.decltor.is_const;
      t->is_volatile = top->u.decltor.is_volatile;
      if (prev) {
        prev->inner = t;
      }
      break;
    }
    case ArrDecltor: {
      t->kind = ArrT;
      if (top->u.decltor.data.arr_size) {
        t->arr_size = top->u.decltor.data.arr_size->u.lit.integer;
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
    token_kind_t store_class = 0;
    bool is_const = false;
    bool is_volatile = false;
    int i;

    t = calloc(1, sizeof(type));
    for (i = 0; i < decl_specs->u.list.len; i++) {
      ast_decl_spec specs = ast_list_at(decl_specs, i)->u.decl_spec;
      token_kind_t tok = specs.tok;

      switch (specs.kind) {
      case StoreClass: {
        store_class = tok;
        continue;
      }
      case TypeQual: {
        if (tok == Const) {
          is_const = true;
        } else {
          is_volatile = true;
        }
        continue;
      }
      case TypeSpec:
        break;
      }

      switch (tok) {
      case Id: {
        t = specs.alias;
        break;
      }
      case Char:
      case Int:
      case Float:
      case Double: {
        t->kind = NumericT;
        t->numeric.base = tok;
        break;
      }
      case Void: {
        t->kind = VoidT;
        break;
      }
      case Struct: {
        t->kind = StructT;
        t->name = specs.name;
        t->struct_fields = specs.struct_fields;
        break;
      }
      case Union: {
        t->kind = UnionT;
        t->name = specs.name;
        t->struct_fields = specs.struct_fields;
        break;
      }
      case Enum: {
        t->kind = EnumT;
        t->name = specs.name;
        t->enum_idents = specs.enum_idents;
        t->enum_exprs = specs.enum_exprs;
        break;
      }
      default:
        break;
      }

      if (prev) {
        prev->inner = t;
      }
      if (!d->type) {
        d->type = t;
      }
      d->type->store_class = store_class;
      t->is_const = is_const;
      t->is_volatile = is_volatile;
    }
  }

  return d;
}

ast_node_t *parse_stmt(parser_t *p) {
  switch (p->kind) {
  case LBrace: {
    return parse_stmt_block(p);
  }
  case Case:
  case Default: {
    return parse_stmt_label(p);
  }
  case Id: {
    if (peek(p, 1).kind == Colon) {
      return parse_stmt_label(p);
    } else {
      return parse_stmt_expr(p);
    }
  }
  case If:
  case Switch: {
    return parse_stmt_branch(p);
  }
  case While:
  case Do:
  case For: {
    return parse_stmt_iter(p);
  }
  case Goto:
  case Continue:
  case Break:
  case Return: {
    return parse_stmt_jump(p);
  }
  default: {
    return parse_stmt_expr(p);
  }
  }
  return NULL;
}

ast_node_t *parse_stmt_label(parser_t *p) {
  ast_node_t *node = new_node(Stmt);
  int pos;

  if (p->kind == Id && peek(p, 1).kind == Colon) {
    node->u.stmt.label = parse_ident(p);
  } else if (p->kind == Case) {
    node->u.stmt.label = parse_tok(p);
    node->u.stmt.case_expr = parse_expr(p);
  } else if (p->kind == Default) {
    node->u.stmt.label = parse_tok(p);
  } else {
    puts("invalid label statement");
    throw(p);
  }

  expect(p, Colon);

  /* Just make sure the following statement is there without consuming it */
  pos = p->pos;
  parse_stmt(p);
  set_pos(p, pos);
  node->u.stmt.kind = LabelStmt;

  return node;
}

ast_node_t *parse_stmt_block(parser_t *p) {
  ast_node_t *node = new_node(Stmt);

  ast_node_t *item = NULL;
  ast_node_t *ls = new_node(List);
  node->u.stmt.inner = ls;

  expect(p, LBrace);

  for (; p->tok.kind != RBrace;) { /* allow mixed decls and stmts? */
    if (is_decl_spec(p, p->tok)) {
      int i;
      ast_node_t *decls = parse_decl(p);
      for (i = 0; i < decls->u.list.len; i++) {
        item = ast_list_at(decls, i);
        ast_list_append(ls, item);
      }
    } else {
      item = parse_stmt(p);
      ast_list_append(ls, item);
    }
  }

  expect(p, RBrace);

  node->u.stmt.kind = BlockStmt;
  return node;
}

ast_node_t *parse_stmt_expr(parser_t *p) {
  ast_node_t *node = new_node(Stmt);

  node->u.stmt.inner = parse_expr(p);
  expect(p, Semi);
  node->u.stmt.kind = ExprStmt;

  return node;
}

ast_node_t *parse_stmt_branch(parser_t *p) {
  ast_node_t *node = new_node(Stmt);

  if (p->kind == If) {
    node->u.stmt.kind = IfStmt;
  } else if (p->kind == Switch) {
    node->u.stmt.kind = SwitchStmt;
  }
  advance(p);

  expect(p, LParen);
  node->u.stmt.cond = parse_expr(p);
  expect(p, RParen);
  node->u.stmt.inner = parse_stmt(p);

  if (p->kind == Else) {
    node->u.stmt.kind = IfElseStmt;
    advance(p);
    node->u.stmt.inner_else = parse_stmt(p);
  }

  return node;
}

ast_node_t *parse_stmt_iter(parser_t *p) {
  ast_node_t *node = new_node(Stmt);

  if (p->kind == While) {
    advance(p);
    node->u.stmt.kind = WhileStmt;

    expect(p, LParen);
    node->u.stmt.cond = parse_expr(p);
    expect(p, RParen);

    node->u.stmt.inner = parse_stmt(p);
  } else if (p->kind == Do) {
    advance(p);
    node->u.stmt.kind = DoWhileStmt;

    node->u.stmt.inner = parse_stmt(p);
    expect(p, While);

    expect(p, LParen);
    node->u.stmt.cond = parse_expr(p);
    expect(p, RParen);
    expect(p, Semi);
  } else if (p->kind == For) {
    advance(p);
    node->u.stmt.kind = ForStmt;

    expect(p, LParen);
    if (p->kind != Semi) {
      node->u.stmt.init = parse_expr(p);
    }
    expect(p, Semi);
    if (p->kind != Semi) {
      node->u.stmt.cond = parse_expr(p);
    }
    expect(p, Semi);
    if (p->kind != RParen) {
      node->u.stmt.iter = parse_expr(p);
    }
    expect(p, RParen);

    node->u.stmt.inner = parse_stmt(p);
  }

  return node;
}

ast_node_t *parse_stmt_jump(parser_t *p) {
  ast_node_t *node = new_node(Stmt);
  node->u.stmt.kind = JumpStmt;

  switch (p->kind) {
  case Goto: {
    node->u.stmt.jump = parse_tok(p);
    node->u.stmt.inner = parse_ident(p);
    break;
  }
  case Continue:
  case Break: {
    node->u.stmt.jump = parse_tok(p);
    break;
  }
  case Return: {
    node->u.stmt.jump = parse_tok(p);
    if (p->kind != Semi) {
      node->u.stmt.inner = parse_expr(p);
    }
    break;
  }
  default: {
    puts("invalid jump stmt");
    throw(p);
  }
  }

  expect(p, Semi);
  return node;
}

ast_node_t *parse_init(parser_t *p) {
  if (p->kind == LBrace) {
    ast_node_t *ls = new_node(List);
    advance(p);

    for (;; expect(p, Comma)) {
      ast_list_append(ls, parse_init(p));
      if (p->kind != Comma) {
        break;
      }
    }

    expect(p, RBrace);
    return ls;
  } else {
    return expr(p, 0);
  }
}

ast_node_t *parse_decl(parser_t *p) {
  ast_node_t *node = new_node(List);
  ast_node_t *decl_specs = parse_decl_specs(p);
  ast_node_t *new;

  new = new_node(Decl);
  new->u.decl = *decl(decl_specs, parse_decltor(p));
  ast_list_append(node, new);

  /* TODO: scope typedefs properly */
  if (new->u.decl.type->store_class == Typedef) {
    p->tdefs = realloc(p->tdefs, sizeof(*p->tdefs) * (p->tdefs_len + 1));
    p->tdefs[p->tdefs_len++] = *new;
  }

  if (p->kind == Assn) {
    expect(p, Assn);
    ast_list_at(node, 0)->u.decl.init = parse_init(p);
  }

  for (; p->kind == Comma;) {
    new = new_node(Decl);
    expect(p, Comma);
    new->u.decl = *decl(decl_specs, parse_decltor(p));
    if (p->kind == Assn) {
      expect(p, Assn);
      new->u.decl.init = parse_init(p);
    }
    ast_list_append(node, new);
    if (new->u.decl.type->store_class == Typedef) {
      p->tdefs = realloc(p->tdefs, sizeof(*p->tdefs) * (p->tdefs_len + 1));
      p->tdefs[p->tdefs_len++] = *new;
    }
  }

  expect(p, Semi);

  return node;
}

struct ast_node_t *parse_tok(parser_t *p) {
  ast_node_t *node = new_node(Tok);
  node->u.tok = p->tok;
  advance(p);
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
  case Character:
  case String: {
    lhs = parse_lit(p);
    break;
  }
  case LParen: { /* group or cast */
    advance(p);
    if (is_decl_spec(p, p->tok)) {
      expr_power power = expr_power_prefix(LParen);

      ast_node_t *type_name = parse_type_name(p);
      expect(p, RParen);

      lhs->u.expr.kind = CastExpr;
      lhs->u.expr.op = LParen;
      lhs->u.expr.lhs = type_name;
      lhs->u.expr.rhs = expr(p, power.right);
    } else {
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

    if (op == Sizeof && p->kind == LParen &&
        is_decl_spec(p, peek(p, 1))) { /* sizeof(type_name) */
      advance(p);
      lhs->u.expr.rhs = parse_type_name(p);
      expect(p, RParen);
    } else {
      lhs->u.expr.rhs = expr(p, power.right);
    }
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
        if (p->kind != RParen) {
          lhs->u.expr.rhs = parse_expr(p);
        }
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

void ast_list_append(ast_node_t *list, struct ast_node_t *item) {
  if (list->u.list.len >= list->u.list.cap) {
    list->u.list.cap *= 2;
    list->u.list.nodes =
        realloc(list->u.list.nodes, sizeof(ast_node_t *) * list->u.list.cap);
  }

  list->u.list.nodes[list->u.list.len++] = item;
}

ast_node_t *ast_list_at(ast_node_t *list, int idx) {
  if (idx == list->u.list.len) {
    puts("out of bounds");
    exit(1);
  }
  return list->u.list.nodes[idx];
}

ast_node_t *parse_type_name(parser_t *p) {
  ast_node_t *node = new_node(TypeName);
  ast_node_t *decltor;
  ast_node_t *decl_specs;
  ast_decl *decltion;

  decl_specs = parse_decl_specs(p);
  decltor = parse_decltor(p);
  decltion = decl(decl_specs, decltor);
  if (decltion->init || decltion->name) {
    puts("malformed type name");
    throw(p);
  }
  if (decltion->type->store_class) {
    puts("type name cannot have storage class specifier");
    throw(p);
  }

  node->u.type_name = *decltion->type;

  return node;
}

int parse(struct unit *src, ast_node_t **ast) {
  parser_t p;
  ast_node_t *nodes;
  int len;
  int cap;

  p = new_parser(src);
  len = 0;
  cap = 128;
  nodes = calloc(cap, sizeof(*nodes));

  for (len = 0; p.kind != Eof;) {
    ast_node_t *node = NULL;
    int pos = p.pos;

    parse_decl_specs(&p);
    parse_decltor(&p);

    if (p.kind == LBrace) { /* FnDefn */
      set_pos(&p, pos);
      node = parse_fn_defn(&p);

      if (len >= cap) {
        cap *= 2;
        nodes = realloc(nodes, cap * sizeof(*nodes));
      }
      nodes[len++] = *node;
    } else if (p.kind == Semi || p.kind == Comma || p.kind == Assn) { /* Decl */
      ast_node_t *decls;
      set_pos(&p, pos);
      decls = parse_decl(&p);

      if (len + decls->u.list.len >= cap) {
        cap = (len + decls->u.list.len) * 2;
        nodes = realloc(nodes, cap * sizeof(*nodes));
      }
      memcpy(nodes + len, *decls->u.list.nodes,
             decls->u.list.len * sizeof(*nodes));
      len += decls->u.list.len;
    } else {
      printf("unexpected token %s (%s)\n", p.tok.text, token_kind_map[p.kind]);
      throw(&p);
    }
  }

  *ast = nodes;
  return len;
}

const char *ast_node_kind_map[] = {"Ident",   "Lit",  "FnDefn",  "DeclSpecs",
                                   "Decltor", "Decl", "Stmt",    "Tok",
                                   "Expr",    "List", "TypeName"};
