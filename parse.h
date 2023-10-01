#ifndef CHOCC_PARSE_H
#define CHOCC_PARSE_H
#pragma once

#include "chocc.h"
#include "lex.h"

#include <stdlib.h>
#include <string.h>

/*
 * parser_t represents the parser and its state.
 */
typedef struct parser_t {
  token_t *toks;
  int toks_len;
  int pos;
  token_t tok;
  token_kind_t kind;
} parser_t;

void throw(parser_t * parser);

void expect(parser_t *parser, token_kind_t kind);

/* advance advances the parser state by one token. */
void advance(parser_t *parser);

/* set_pos sets the parser state to the position specified by pos. */
void set_pos(parser_t *parser, int pos);

/* peek returns the token at parser->pos + delta without modifying state. */
token_t peek(parser_t *parser, int delta);

/*
 * Type
 *
 * Types represent a C type.
 * A linear representation is used a la
 * https://blog.robertelder.org/building-a-c-compiler-type-system-the-formidable-declarator/.

 *
 * Types are built by consuming DeclSpecs and Decltors.
 * For example, the declaration
 *     const int *x[5];
 *     |         `---- Decltor
 *     `-------- DeclSpecs
 * would be transformed into the following (Ident: Type) pair.
 *     x: const int *[5]
 */

/*
 * numeric_type represents all possible C built-in primitive types.
 * There are 14:
 *    - char
 *    - signed char
 *    - unsigned char
 *    - short, short int, signed short, signed short int,
 *    - unsigned short, unsigned short int
 *    - int, signed, signed int
 *    - unsigned, unsigned int
 *    - long, long int, signed long, signed long int
 *    - unsigned long, unsigned long int
 *    - long long, long long int, signed long long, signed long long int
 *    - unsigned long long, unsigned long long int
 *    - float
 *    - double
 *    - long double
 */
typedef struct numeric_type {
  token_kind_t base;

  /* modifiers */
  bool is_signed;
  bool is_unsigned;
  bool is_short;
  bool is_long;
} numeric_type;

typedef enum type_kind {
  NumericT,
  PtrT,
  ArrT,
  FnT,
  VoidT,
  StructT,
  UnionT,
  EnumT
} type_kind;

typedef struct type {
  type_kind kind;

  /* numeric */
  numeric_type numeric;

  /* ptr / arr */
  struct type *inner;

  /* arr */
  int arr_size;

  /* fn */
  struct type *fn_return_ty;
  struct ast_node_t *fn_param_decls;
  int fn_param_decls_len;
  int fn_param_decls_cap;

  /* struct and union */
  struct ast_node_t *struct_fields; /* ast_list(ast_decl) */
  /* enum */
  struct ast_node_t *enum_idents; /* ast_list(ast_ident) */
  struct ast_node_t *enum_exprs;  /* ast_list(ast_expr) */

  struct ast_node_t *name; /* ast_ident */

  /* type qualifiers */
  bool is_const;
  bool is_volatile; /* unused */

  /* storage class specifers */
  token_kind_t store_class;
} type;

void print_type(type *);

struct ast_node_t;

/*
 * Ident(ifier)
 *
 * 6.4.2.1
 * identifier := identifier_nondigit
 *             | identifier identifier_nondigit
 *             | identifier digit
 * identifier := [a-zA-Z_][a-zA-Z0-9_]+
 */

typedef char *ast_ident;

struct ast_node_t *parse_ident(parser_t *);

/*  Parses token into an ident, no matter its kind */
struct ast_node_t *parse_into_ident(parser_t *);

/*
 * Lit(eral)
 *
 * 6.4.4
 * constant := integer_constant
 *           | floating_constant
 *           | enumeration_constant
 *           | character_constant
 */

typedef enum lit_kind {
  OctLit,
  DecLit,
  HexLit,
  CharLit,
  FloatingLit,
  StrLit
} lit_kind;

typedef struct ast_lit {
  lit_kind kind;

  long int integer;
  long double floating;
  char *string;
  char character;

  bool is_unsigned;
  bool is_long;
  bool is_float;
} ast_lit;

/*
 * FnDefn (function definition)
 * K&R style is not supported.
 *
 * 6.9.1
 * function_definition
 *    := declaration_specifiers declarator compound_statement
 */

typedef struct ast_fn_defn {
  struct ast_decl *decl;
  struct ast_node_t *body; /* stmt */
} ast_fn_defn;

struct ast_node_t *parse_fn_defn(parser_t *);

/*
 * DeclSpecs (declaration specifiers)
 *
 * Decspecs are not in the final AST, instead being a temporary node that's
 * consumed into a type together with Decltors.
 *
 * 6.9.1
 * declaration_specifiers :=
 *    | storage_class_specifier declaration_specifiers?
 *    | type_specifier declaration_specifiers?
 *    | type_qualifier declaration_specifiers?
 *    | function_specifier declaration_specifiers?
 */

typedef enum ast_decl_spec_kind {
  StoreClass,
  TypeSpec,
  TypeQual
} ast_decl_spec_kind;

typedef struct ast_decl_spec {
  ast_decl_spec_kind kind;
  token_kind_t tok;

  /* struct and union */
  struct ast_node_t *struct_fields; /* ast_list(ast_decl) */
  /* enum */
  struct ast_node_t *enum_idents; /* ast_list(ast_ident) */
  struct ast_node_t *enum_exprs;  /* ast_list(ast_expr) */

  struct ast_node_t *name;
} ast_decl_spec;

struct ast_node_t *parse_decl_specs(parser_t *);
bool is_decl_spec(token_t token);

/*
 * Decl(ara)tors
 *
 * Decltors are not in the final AST, instead being a temporary node
 * that's consumed into a type together with DeclSpecs.
 *
 * 6.7.5
 * declarator := pointer? direct_declarator
 * pointer := * type_qualifier_list?
 *          | * type_qualifier_list? pointer
 * direct_declarator := identifier
 *                    | ( declarator )
 *                    | direct_declarator
 *                        [type_qualifier_list? assignment_expression?]
 *                    | direct_declarator
 *                        [static type_qualifier_list? assignment_expression]
 *                    | direct_declarator
 *                        [type_qualifier_list static assignment_expression]
 *                    | direct_declarator [type_qualifier_list? *]
 *                    | direct_declarator (parameter_type_list)
 *                    | direct_declarator (identifier_list?)
 */

/* ast_decltor_kind_t enumerates the kinds of decltors. */
typedef enum ast_decltor_kind_t {
  IdentDecltor, /* ident */
  PtrDecltor,   /* *decltor */
  FnDecltor,    /* decltor() */
  ArrDecltor,   /* decltor[] */
  GroupDecltor, /* (decltor) */
  AbstDecltor   /* abstract */
} ast_decltor_kind_t;

typedef struct ast_decltor {
  struct ast_node_t *inner; /* ast_ident | ast_decltor */
  ast_decltor_kind_t kind;
  union {
    struct ast_node_t *arr_size; /* ast_lit */
    struct {
      struct ast_node_t *decl_specs; /* ast_declspec */
      struct ast_node_t *decltors;   /* ast_decltor */
      int decl_specs_len;
      int decl_specs_cap;
      int decltors_len;
      int decltors_cap;
    } params;
  } data;
} ast_decltor;

struct ast_node_t *parse_decltor(parser_t *);

/*
 * Decl(aration)
 *
 * 6.7
 * declaration := declaration_specifiers init_declarator_list? ;
 */

typedef struct ast_decl {
  struct ast_node_t *name; /* ast_ident */
  struct type *type;
  struct ast_node_t *init; /* ast_expr */
} ast_decl;

/*
 * Parses declarations, returning the head node and setting the tail node.
 */
struct ast_node_t *parse_decl(parser_t *p);

/*
 * decl consumes DeclSpecs and Decltor into a parsed declaration
 */
struct ast_decl *decl(struct ast_node_t *decl_specs,
                      struct ast_node_t *decltor);

/*
 * Tok(en)
 * Tok represents a token used as an AST node.
 */

typedef token_t ast_tok;
struct ast_node_t *parse_tok(parser_t *p);

typedef enum ast_expr_kind_t {
  InfixExpr,
  PrefixExpr,
  PostfixExpr,
  CommaExpr,
  CallExpr
} ast_expr_kind_t;

/*
 * Expr(ression)
 */
typedef struct ast_expr {
  struct ast_node_t *lhs;
  struct ast_node_t *rhs;
  struct ast_node_t *mhs; /* mhs stores ?mhs: and mhs, mhs, mhs */
  int mhs_len;
  int mhs_cap;
  ast_expr_kind_t kind;
  token_kind_t op;
} ast_expr;

typedef struct expr_power {
  int left;
  int right;
} expr_power;

struct ast_node_t *parse_expr(parser_t *);
struct ast_node_t *expr(parser_t *, int min_bp);

expr_power expr_power_infix(token_kind_t);
expr_power expr_power_prefix(token_kind_t);
expr_power expr_power_postfix(token_kind_t);

/*
 * St(ate)m(en)t
 *
 * label
 * block
 * expr
 * select: if, if-else, switch
 * iter: while, do-while, for
 * jump: goto, continue, break, return
 */

typedef enum ast_stmt_kind {
  LabelStmt,
  BlockStmt,
  ExprStmt,
  IfStmt,
  IfElseStmt,
  SwitchStmt,
  WhileStmt,
  DoWhileStmt,
  ForStmt,
  JumpStmt
} ast_stmt_kind;

typedef struct ast_stmt {
  ast_stmt_kind kind;

  struct ast_node_t *label; /* Ident, Tok(case), Tok(default) */
  struct ast_node_t *case_expr;

  struct ast_node_t *init;
  struct ast_node_t *cond;
  struct ast_node_t *iter;

  struct ast_node_t *inner;      /* Stmt, List, Expr */
  struct ast_node_t *inner_else; /* Stmt, List, Expr */

  struct ast_node_t *jump; /* Goto, Continue, Break, Return */
} ast_stmt;

struct ast_node_t *parse_stmt(parser_t *);

struct ast_node_t *parse_stmt_label(parser_t *);
struct ast_node_t *parse_stmt_block(parser_t *);
struct ast_node_t *parse_stmt_expr(parser_t *);
struct ast_node_t *parse_stmt_branch(parser_t *);
struct ast_node_t *parse_stmt_iter(parser_t *);
struct ast_node_t *parse_stmt_jump(parser_t *);

/*
 * ast_list represents a list of AST nodes.
 */

typedef struct ast_list {
  int len;
  int cap;
  struct ast_node_t **nodes;
} ast_list;

void ast_list_append(struct ast_node_t *list, struct ast_node_t *item);
struct ast_node_t *ast_list_at(struct ast_node_t *list, int idx);

/*
 * ast_node_kind_t enumerates the kinds of AST nodes ast_node_t can have.
 * Each item corresponds to a ast_* struct.
 */
typedef enum ast_node_kind_t {
  Ident,
  Lit,
  FnDefn,
  DeclSpecs,
  Decltor,
  Decl,
  Stmt,
  Tok,
  Expr,
  List
} ast_node_kind_t;

extern const char *ast_node_kind_map[];

/*
 * ast_node_t represents AST nodes.
 * Nodes are homogenous, children are irregular.
 */
typedef struct ast_node_t {
  ast_node_kind_t kind;
  union data {
    ast_ident ident;
    ast_fn_defn fn_defn;
    ast_decl_spec decl_spec;
    ast_decltor decltor;
    ast_lit lit;
    ast_decl decl;
    ast_tok tok;
    ast_stmt stmt;
    ast_expr expr;
    ast_list list;
  } u;
  struct ast_node_t *next;
} ast_node_t;

void print_ast(ast_node_t *root, int depth, bool last, char *pad);
ast_node_t **parse(parser_t *);

#endif
