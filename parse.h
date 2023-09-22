#ifndef CHOCC_PARSE_H
#define CHOCC_PARSE_H
#pragma once

#include "chocc.h"
#include "token.h"

#include <stdlib.h>
#include <string.h>

/*
 * parser_t represents the parser and its state.
 */
typedef struct parser_t {
  tokens_t *tokens;
  int pos;
  token_t tok;
  token_kind_t kind;
} parser_t;

/* advance advances the parser state by one token. */
void advance(parser_t *parser);

/* set_pos sets the parser state to the position specified by pos. */
void set_pos(parser_t *parser, int pos);

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

typedef enum type_kind { NumericT, PtrT, ArrT, FnT, VoidT, StructT } type_kind;

typedef struct type {
  char *modifiers;
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

  /* struct */
  struct ast_node_t *struct_decls;
  int struct_decls_len;
  int struct_decls_cap;
} type;

void print_type(type *);

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
  BlockStmt,
  Tok
} ast_node_kind_t;

extern const char *ast_node_kind_map[];

struct ast_node_t;

/*
 * decl consumes DeclSpecs and Decltor into a parsed declaration
 */
struct ast_decl *decl(struct ast_node_t *decl_specs,
                      struct ast_node_t *decltor);

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

typedef struct ast_lit {
  int value;
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
  struct ast_node_t *body; /* block_stmt */
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

typedef struct ast_decl_specs {
  struct ast_node_t *specs; /* ast_tok */
  int specs_len;
  int specs_cap;
} ast_decl_specs;

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
} ast_decl;

struct ast_node_t *parse_decl(parser_t *p);

/*
 * BlockStmt (block statement)
 *
 * 6.8.2
 * compound_statement := {
 *    block_item*
 * }
 * block_item := declaration | statement
 */
struct ast_node_t *parse_block_stmt(parser_t *);

typedef struct ast_block_stmt {
  struct ast_node_t *items;
  int items_len;
  int items_cap;
} ast_block_stmt;

/*
 * Tok(en)
 * Tok represents a token used as an AST node.
 * Used only as a temporary node, should not exist in the final AST.
 */

typedef token_t ast_tok;

/*
 * ast_node_t represents AST nodes.
 * Nodes are homogenous, children are irregular.
 */
typedef struct ast_node_t {
  ast_node_kind_t kind;
  union data {
    ast_ident ident;
    ast_fn_defn fn_defn;
    ast_decl_specs decl_specs;
    ast_decltor decltor;
    ast_lit lit;
    ast_decl decl;
    ast_tok tok;
    ast_block_stmt block_stmt;
  } u;
} ast_node_t;

void print_ast(ast_node_t *root, int depth);
ast_node_t **parse(parser_t *);

#endif
