#include "parse.h"
#include "lex.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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
}

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

void parse_translation_unit(token_t *tokens) {
  token_ptr = tokens;

  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = translation_unit;
  ast = node;
  parse_external_declaration(node);
}

void parse_external_declaration(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = external_declaration;
  append_node(root, node);
  parse_function_definition(node);
}

void parse_function_definition(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = function_defintion;
  append_node(root, node);
  parse_declaration_specifiers(node);
}

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

void parse_declarator(ast_node_t *root) {
}

void parse_direct_declarator(ast_node_t *root) {
}

void parse_pointer(ast_node_t *root) {
}
