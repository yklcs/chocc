#include "lex.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  translation_unit,
  external_declaration,
  function_defintion,
  declaration,
  declaration_specifiers,
  storage_class_specifier,
  type_specifier,
  type_qualifier,
  function_specifier,
  declarator,
  declaration_list,
  compound_statement,
} ast_node_kind_t;

static const char *ast_node_kind_map[] = {
    "translation_unit", "external_declaration",   "function_defintion",
    "declaration",      "declaration_specifiers", "storage_class_specifier",
    "type_specifier",   "type_qualifier",         "function_specifier",
    "declarator",       "declaration_list",       "compound_statement",
};

typedef struct ast_node_t {
  struct ast_node_t *child;
  struct ast_node_t *next;
  ast_node_kind_t kind;
} ast_node_t;

ast_node_t *ast;
token_t *token_ptr;

void append_node(ast_node_t *parent, ast_node_t *child) {
  if (parent->child) {
    ast_node_t *cur;
    for (cur = parent->child; cur->next; cur = cur->next) {
    }
    cur->next = child;
  } else {
    parent->child = child;
  }
}

void print_ast(ast_node_t *root) {
  int depth = 0;
  for (ast_node_t *cur = root; cur; cur = cur->child) {
    for (ast_node_t *sib = cur; sib; sib = sib->next) {
      for (int i = 0; i < depth; i++) {
        putchar(' ');
      }
      printf("%s\n", ast_node_kind_map[sib->kind]);
    }
    depth++;
  }

  // printf("  ");
  // if (root.child) {
  //   print_ast(*root.child);
  // }
}

void parse_translation_unit(token_t *token);
void parse_external_declaration(ast_node_t *root);
void parse_function_definition(ast_node_t *root);
void parse_declaration_specifiers(ast_node_t *root);
void parse_storage_class_specifier(ast_node_t *root);
void parse_type_specifier(ast_node_t *root);

bool is_storage_class_specifier(token_t token);
bool is_type_specifier(token_t token);

void throw() {
  printf("parsing error");
  exit(1);
}

void match(token_kind_t kind) {
  if (token_ptr->kind == kind) {
    token_ptr++;
  } else {
    throw();
  }
}

// translation_unit := external_declaration
//                   | translation_unit external_declaration
void parse_translation_unit(token_t *tokens) {
  token_ptr = tokens;

  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = translation_unit;
  ast = node;
  parse_external_declaration(node);
}

// external_declaration := function_definition
//                       | declaration
void parse_external_declaration(ast_node_t *root) {
  // for (;;) {
  // tokens[0].kind;
  // }
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = external_declaration;
  append_node(root, node);
  parse_function_definition(node);
}

// function_definition
//    := declaration_specifiers declarator declaration_list? compound_statement
void parse_function_definition(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = function_defintion;
  append_node(root, node);
  parse_declaration_specifiers(node);
}

// declaration_specifiers := storage_class_specifier declaration_specifiers?
//                         | type_specifier declaration_specifiers?
//                         | type_qualifier declaration_specifiers?
//                         | function_specifier declaration_specifiers?
void parse_declaration_specifiers(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = declaration_specifiers;
  append_node(root, node);
  while (is_storage_class_specifier(*token_ptr) ||
         is_type_specifier(*token_ptr)) {
    if (is_storage_class_specifier(*token_ptr)) {
      parse_storage_class_specifier(node);
    } else if (is_type_specifier(*token_ptr)) {
      parse_type_specifier(node);
    }
  }
}

// storage_class_specifier := "typedef"
//                            "extern"
//                            "static"
//                            "auto"
//                            "register"
void parse_storage_class_specifier(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = storage_class_specifier;

  if (is_storage_class_specifier(*token_ptr)) {
    match(token_ptr->kind);
    append_node(root, node);
  } else {
    throw();
  }
}

bool is_storage_class_specifier(token_t token) {
  switch (token.kind) {
  case Typedef:
  case Extern:
  case Static:
  case Auto:
  case Register:
    return true;
  default:
    return false;
  }
}

// storage_class_specifier := "void"
//                          | "char"
//                          | "short"
//                          | "int"
//                          | "long"
//                          | "float"
//                          | "double"
//                          | "signed"
//                          | "unsigned"
//                          | "_Bool"
//                          | "_Complex"
//                          | struct_or_union_specifier
//                          | enum_specifier
//                          | typedef_name
void parse_type_specifier(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = type_specifier;

  if (is_type_specifier(*token_ptr)) {
    match(token_ptr->kind);
    append_node(root, node);
  } else {
    throw();
  }
}

bool is_type_specifier(token_t token) {
  switch (token.kind) {
  case Void:
  case Char:
  case Short:
  case Int:
  case Long:
  case Float:
  case Double:
  case Signed:
  case Unsigned:
  case Bool:
  case Complex:
    return true;
  default:
    return false;
  }
}