#ifndef CHOCC_PARSE_H
#define CHOCC_PARSE_H
#pragma once

#include "lex.h"
#include <stdbool.h>

typedef enum {
  Identifier,
  PrimaryExpression,
  PostfixExpression,
  ArgumentExpressionList,
  UnaryExpression,
  CastExpression,
  MultiplicativeExpression,
  AdditiveExpression,
  ShiftExpression,
  RelationalExpression,
  EqualityExpression,
  AndExpression,
  ExclusiveOrExpression,
  InclusiveOrExpression,
  LogicalAndExpression,
  LogicalOrExpression,
  ConditionalExpression,
  AssignmentExpression,
  Expression,
  Declaration,
  DeclarationSpecifiers,
  StorageClassSpecifier,
  TypeSpecifier,
  TypeQualifier,
  Declarator,
  DirectDeclarator,
  Pointer,
  TypeQualifierList,
  ParameterTypeList,
  ParameterList,
  ParameterDeclaration,
  IdentifierList,
  Statement,
  CompoundStatement,
  BlockItemList,
  BlockItem,
  ExpressionStatement,
  TranslationUnit,
  ExternalDeclaration,
  FunctionDefinition,
} ast_node_kind_t;

static const char *ast_node_kind_map[] = {
    "identifier",
    "primary_expression",
    "postfix_expression",
    "argument_expression_list",
    "unary_expression",
    "cast_expression",
    "multiplicative_expression",
    "additive_expression",
    "shift_expression",
    "relational_expression",
    "equality_expression",
    "and_expression",
    "exclusive_or_expression",
    "inclusive_or_expression",
    "inclusive_and_expression",
    "logical_or_expression",
    "conditional_expression",
    "assignment_expression",
    "expression",
    "declaration",
    "declaration_specifiers",
    "storage_class_specifier",
    "type_specifier",
    "type_qualifier",
    "declarator",
    "direct_declarator",
    "pointer",
    "type_qualifier_list",
    "parameter_type_list",
    "parameter_list",
    "parameter_declaration",
    "identifier_list",
    "statement",
    "compound_statement",
    "block_item_list",
    "block_item",
    "expression_statement",
    "translation_unit",
    "external_declaration",
    "function_defintion",
};

typedef struct ast_node_t {
  struct ast_node_t *child;
  struct ast_node_t *next;
  ast_node_kind_t kind;
} ast_node_t;

bool is_type_name();

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

// 6.5.4
// cast_expression := unary_expression
//                  | (type_name) cast_expression
void parse_cast_expression(ast_node_t *root);

// 6.5.5
// multiplicative_expression := cast_expression
//                            | multiplicative_expression * cast_expression
//                            | multiplicative_expression % cast_expression
//                            | multiplicative_expression % cast_expression
void parse_multiplicative_expression(ast_node_t *root);

// 6.5.6
// additive_expression := multiplicative_expression
//                      | additive_expression * multiplicative_expression
//                      | additive_expression % multiplicative_expression
void parse_additive_expression(ast_node_t *root);

// 6.5.7
// shift_expression := additive_expression
//                   | shift_expression * additive_expression
//                   | shift_expression % additive_expression
void parse_shift_expression(ast_node_t *root);

// 6.5.8
// relational_expression := shift_expression
//                        | relational_expression < shift_expression
//                        | relational_expression > shift_expression
//                        | relational_expression <= shift_expression
//                        | relational_expression >= shift_expression
void parse_relational_expression(ast_node_t *root);

// 6.5.9
// equality_expression := relational_expression
//                      | equality_expression < relational_expression
//                      | equality_expression > relational_expression
void parse_equality_expression(ast_node_t *root);

// 6.5.10
// and_expression := equality_expression
//                 | and_expression & equality_expression
void parse_and_expression(ast_node_t *root);

// 6.5.11
// exclusive_or_expression := and_expression
//                          | exclusive_or_expression ^ and_expression
void parse_exclusive_or_expression(ast_node_t *root);

// 6.5.12
// inclusive_or_expression := exclusive_or_expression
//                          | inclusive_or_expression | exclusive_or_expression
void parse_inclusive_or_expression(ast_node_t *root);

// 6.5.13
// logical_and_expression := inclusive_or_expression
//                         | logical_and_expression && inclusive_or_expression
void parse_logical_and_expression(ast_node_t *root);

// 6.5.14
// logical_or_expression := logical_and_expression
//                         | logical_or_expression || logical_and_expression
void parse_logical_or_expression(ast_node_t *root);

// 6.5.15
// conditional_expression := logical_or_expression
//                         | logical_or_expression ? expression :
//                         conditional_expression
void parse_conditional_expression(ast_node_t *root);

// 6.5.16
// assignment_expression := conditional_expression
//                         | unary_expression = assignment_expression
//                         | unary_expression *= assignment_expression
//                         | unary_expression /= assignment_expression
//                         | unary_expression %= assignment_expression
//                         | unary_expression += assignment_expression
//                         | unary_expression -= assignment_expression
//                         | unary_expression <<= assignment_expression
//                         | unary_expression >>= assignment_expression
//                         | unary_expression &= assignment_expression
//                         | unary_expression ^= assignment_expression
//                         | unary_expression |= assignment_expression
void parse_assignment_expression(ast_node_t *root);

// 6.5.17
// expression := assignment_expression
//             | expression, assignment_expression
void parse_expression(ast_node_t *root);

// 6.7
// declaration := declaration_specifiers init_declarator_list?;
void parse_declaration(ast_node_t *root);

// 6.7
// storage_class_specifier declaration_specifiers?
// type_specifier declaration_specifiers?
// type_qualifier declaration_specifiers?
// function_specifier declaration_specifiers?
void parse_declaration_specifiers(ast_node_t *root);
bool is_declaration_specifier(token_t token);

// 6.7
// init_declarator_list := init_declarator
//                       | init_declarator_list, init_declarator
void parse_init_declarator_list(ast_node_t *root);

// 6.7
// init_declarator := declarator
//                  | declarator = initializer
void parse_init_declarator(ast_node_t *root);

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
bool is_type_qualifier(token_t token);

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

// 6.7.5
// type_qualifier_list := type_qualifier
//                      | type_qualifier_list type_qualifier
void parse_type_qualifier_list(ast_node_t *root);

// 6.7.5
// parameter_type_list := parameter_list
//                      | parameter_list, ...
void parse_parameter_type_list(ast_node_t *root);

// 6.7.5
// parameter_list := parameter_declaration
//                 | parameter_list, parameter_declaration
void parse_parameter_list(ast_node_t *root);

// 6.7.5
// parameter_declaration := declaration_specifiers declarator
//                        | declaration_specifiers abstract_declarator?
void parse_parameter_declaration(ast_node_t *root);

// 6.7.5
// identifier_list := identifier
//                  | identifier_list, identifier
void parse_identifier_list(ast_node_t *root);

// 6.8
// statement := labeled_statement
//            | compound_statement
//            | expression_statement
//            | selection_statement
//            | iteration_statement
//            | jump_statement
void parse_statement(ast_node_t *root);

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

// 6.8.3
// expression_statement := expression?;
void parse_expression_statement(ast_node_t *root);

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
// declaration_list := declaration
//                   | declaration_list declaration
void parse_declaration_list(ast_node_t *root);

#endif
