#ifndef CHOCC_PARSE_H
#define CHOCC_PARSE_H
#pragma once

#include "lex.h"
#include <stdbool.h>

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
  pointer,
  direct_declarator
} ast_node_kind_t;

static const char *ast_node_kind_map[] = {
    "translation_unit", "external_declaration",   "function_defintion",
    "declaration",      "declaration_specifiers", "storage_class_specifier",
    "type_specifier",   "type_qualifier",         "function_specifier",
    "declarator",       "declaration_list",       "compound_statement",
    "pointer",          "direct_declarator"};

typedef struct ast_node_t {
  struct ast_node_t *child;
  struct ast_node_t *next;
  ast_node_kind_t kind;
} ast_node_t;

// 6.4.2.1
// identifier := identifier_nondigit
//             | identifier identifier_nondigit
//             | identifier digit
// identifier := [a-zA-Z_][a-zA-Z0-9_]*
void parse_identifier(ast_node_t *root);

// 6.4.4
// constant := integer_constant
//           | floating_constant
//           | enumeration_constant
//           | character_constant
void parse_constant(ast_node_t *root);

// 6.4.5
// string_literal := "s_char_sequence?"
void parse_string_literal(ast_node_t *root);

// 6.5.1
// primary_expression := identifier
//                     | constant
//                     | string_literal
//                     | (expression)
void parse_primary_expression(ast_node_t *root);

// 6.5.2
// postfix_expression := primary_expression
//                     | postfix_expression[expression]
//                     | postfix_expression(argument_expression_list?)
//                     | postfix_expression.identifier
//                     | postfix_expression->identifier
//                     | postfix_expression++
//                     | postfix_expression--
//                     | (type_name){initializer_list}
//                     | (type_name){initializer_list,}
void parse_postfix_expression(ast_node_t *root);

// 6.5.2
// argument_expression_list := argument_expression
//                           | argument_expression_list, argument_expression
void parse_argument_expression_list(ast_node_t *root);

// 6.5.3
// unary_expression := postfix_expression
//                   | ++unary_expression
//                   | --unary_expression
//                   | unary_operator cast_expression
//                   | sizeof unary_expression
//                   | sizeof(type_name)
void parse_unary_expression(ast_node_t *root);

// 6.7
// storage_class_specifier declaration_specifiers?
// type_specifier declaration_specifiers?
// type_qualifier declaration_specifiers?
// function_specifier declaration_specifiers?
void parse_declaration_specifiers(ast_node_t *root);

// 6.7.1
// storage_class_specifier := typedef
//                            extern
//                            static
//                            auto
//                            register
void parse_storage_class_specifier(ast_node_t *root);
bool is_storage_class_specifier(token_t token);

// 6.7.2
// type_specifier := void
//                 | char
//                 | short
//                 | int
//                 | long
//                 | float
//                 | double
//                 | signed
//                 | unsigned
//                 | _Bool
//                 | _Complex
//                 | struct_or_union_specifier
//                 | enum_specifier
//                 | typedef_name
void parse_type_specifier(ast_node_t *root);
bool is_type_specifier(token_t token);

// 6.7.3
// type_qualifier := const | restrict | volatile
void parse_type_qualifier(ast_node_t *root);

// 6.7.4
// function_specifier := inline
void parse_function_specifier(ast_node_t *root);

// 6.7.5
// declarator := pointer? direct_declarator
void parse_declarator(ast_node_t *root);

// 6.7.5
// direct_declarator := identifier
//                    | (declarator)
//                    | direct_declarator
//                        [type_qualifier_list? assignment_expression?]
//                    | direct_declarator
//                        [static type_qualifier_list? assignment_expression]
//                    | direct_declarator
//                        [type_qualifier_list static assignment_expression]
//                    | direct_declarator [type_qualifier_list? *]
//                    | direct_declarator (parameter_type_list)
//                    | direct_declarator (identifier_list?)
void parse_direct_declarator(ast_node_t *root);

// 6.7.5
// pointer := *type_qualifier_list?
//          | *type_qualifier_list? pointer
void parse_pointer(ast_node_t *root);

// 6.8.2
// compound_statement := {block_item_list?}
void parse_compound_statement(ast_node_t *root);

// 6.8.2
// block_item_list := block_item
//                  | block_item_list block_item
void parse_block_item_list(ast_node_t *root);

// 6.8.2
// block_item := declaration
//             | statement
void parse_block_item(ast_node_t *root);

// 6.9
// translation_unit := external_declaration
//                   | translation_unit external_declaration
void parse_translation_unit(token_t *token);

// 6.9
// external_declaration := function_definition
//                       | declaration
void parse_external_declaration(ast_node_t *root);

// 6.9.1
// function_definition
//    := declaration_specifiers declarator declaration_list? compound_statement
void parse_function_definition(ast_node_t *root);

// 6.9.1
// declaration_specifiers := storage_class_specifier declaration_specifiers?
//                         | type_specifier declaration_specifiers?
//                         | type_qualifier declaration_specifiers?
//                         | function_specifier declaration_specifiers?

#endif
