#ifndef CHOCC_PARSE_H
#define CHOCC_PARSE_H
#pragma once

#include "lex.h"
#include <stdbool.h>

typedef enum {
  Identifier,
  Constant,
  StringLiteral,
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
  ConstantExpression,
  Declaration,
  DeclarationSpecifiers,
  InitDeclaratorList,
  InitDeclarator,
  StorageClassSpecifier,
  TypeSpecifier,
  StructOrUnionSpecifier,
  StructDeclarationList,
  StructDeclaration,
  SpecifierQualifierList,
  StructDeclaratorList,
  StructDeclarator,
  EnumSpecifier,
  EnumeratorList,
  Enumerator,
  TypeQualifier,
  FunctionSpecifier,
  Declarator,
  DirectDeclarator,
  Pointer,
  TypeQualifierList,
  ParameterTypeList,
  ParameterList,
  ParameterDeclaration,
  IdentifierList,
  TypeName,
  AbstractDeclarator,
  DirectAbstractDeclarator,
  TypedefName,
  Initializer,
  InitializerList,
  Designation,
  DesignatorList,
  Designator,
  Statement,
  LabeledStatement,
  CompoundStatement,
  BlockItemList,
  BlockItem,
  ExpressionStatement,
  SelectionStatement,
  IterationStatement,
  JumpStatement,
  TranslationUnit,
  ExternalDeclaration,
  FunctionDefinition,
  Keyword,
  Punctuator,
} pt_node_kind_t;

static const char *pt_node_kind_map[] = {
    [Keyword] = "keyword",
    [Punctuator] = "punctuator",
    [Identifier] = "identifier",
    [Constant] = "constant",
    [StringLiteral] = "string_literal",
    [PrimaryExpression] = "primary_expression",
    [PostfixExpression] = "postfix_expression",
    [ArgumentExpressionList] = "argument_expression_list",
    [UnaryExpression] = "unary_expression",
    [CastExpression] = "cast_expression",
    [MultiplicativeExpression] = "multiplicative_expression",
    [AdditiveExpression] = "additive_expression",
    [ShiftExpression] = "shift_expression",
    [RelationalExpression] = "relational_expression",
    [EqualityExpression] = "equality_expression",
    [AndExpression] = "and_expression",
    [ExclusiveOrExpression] = "exclusive_or_expression",
    [InclusiveOrExpression] = "inclusive_or_expression",
    [LogicalAndExpression] = "inclusive_and_expression",
    [LogicalOrExpression] = "logical_or_expression",
    [ConditionalExpression] = "conditional_expression",
    [AssignmentExpression] = "assignment_expression",
    [Expression] = "expression",
    [ConstantExpression] = "constant_expression",
    [Declaration] = "declaration",
    [DeclarationSpecifiers] = "declaration_specifiers",
    [InitDeclaratorList] = "init_declarator_list",
    [InitDeclarator] = "init_declarator",
    [StorageClassSpecifier] = "storage_class_specifier",
    [TypeSpecifier] = "type_specifier",
    [StructOrUnionSpecifier] = "struct_or_union_specifier",
    [StructDeclarationList] = "struct_declaration_list",
    [StructDeclaration] = "struct_declaration",
    [SpecifierQualifierList] = "specifier_qualifier_list",
    [StructDeclaratorList] = "struct_declarator_list",
    [StructDeclarator] = "struct_declarator",
    [EnumSpecifier] = "enum_specifier",
    [EnumeratorList] = "enumerator_list",
    [Enumerator] = "enumerator",
    [TypeQualifier] = "type_qualifier",
    [FunctionSpecifier] = "function_specifier",
    [Declarator] = "declarator",
    [DirectDeclarator] = "direct_declarator",
    [Pointer] = "pointer",
    [TypeQualifierList] = "type_qualifier_list",
    [ParameterTypeList] = "parameter_type_list",
    [ParameterList] = "parameter_list",
    [ParameterDeclaration] = "parameter_declaration",
    [IdentifierList] = "identifier_list",
    [TypeName] = "type_name",
    [AbstractDeclarator] = "abstract_declarator",
    [DirectAbstractDeclarator] = "direct_abstract_declarator",
    [TypedefName] = "typedef_name",
    [Initializer] = "initializer",
    [InitializerList] = "initializer_list",
    [Designation] = "designation",
    [DesignatorList] = "designator_list",
    [Designator] = "designator",
    [Statement] = "statement",
    [LabeledStatement] = "labeled_statement",
    [CompoundStatement] = "compound_statement",
    [BlockItemList] = "block_item_list",
    [BlockItem] = "block_item",
    [ExpressionStatement] = "expression_statement",
    [SelectionStatement] = "selection_statement",
    [IterationStatement] = "iteration_statement",
    [JumpStatement] = "jump_statement",
    [TranslationUnit] = "translation_unit",
    [ExternalDeclaration] = "external_declaration",
    [FunctionDefinition] = "function_definition",
};

typedef union pt_node_data_t {
  struct {
    char *name;
  } Identifier;
  struct {
    char *text;
    token_kind_t token_kind;
  } Keyword;
  struct {
    char *text;
    token_kind_t token_kind;
  } Punctuator;
  struct {
    char *text;
  } Constant;
} pt_node_data_t;

typedef struct pt_node_t {
  struct pt_node_t *child;
  struct pt_node_t *next;
  int num_children;
  pt_node_kind_t kind;
  pt_node_data_t data;
} pt_node_t;

bool is_type_name(token_t *token);

// 6.4.2.1
// identifier := identifier_nondigit
//             | identifier identifier_nondigit
//             | identifier digit
// identifier := [a-zA-Z_][a-zA-Z0-9_]*
void parse_identifier(pt_node_t *root);

// 6.4.4
// constant := integer_constant
//           | floating_constant
//           | enumeration_constant
//           | character_constant
void parse_constant(pt_node_t *root);

// 6.4.5
// string_literal := "s_char_sequence?"
void parse_string_literal(pt_node_t *root);

// 6.5.1
// primary_expression := identifier
//                     | constant
//                     | string_literal
//                     | (expression)
void parse_primary_expression(pt_node_t *root);

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
void parse_postfix_expression(pt_node_t *root);

// 6.5.2
// argument_expression_list := argument_expression
//                           | argument_expression_list, argument_expression
void parse_argument_expression_list(pt_node_t *root);

// 6.5.3
// unary_expression := postfix_expression
//                   | ++unary_expression
//                   | --unary_expression
//                   | unary_operator cast_expression
//                   | sizeof unary_expression
//                   | sizeof(type_name)
void parse_unary_expression(pt_node_t *root);

// 6.5.4
// cast_expression := unary_expression
//                  | (type_name) cast_expression
void parse_cast_expression(pt_node_t *root);

// 6.5.5
// multiplicative_expression := cast_expression
//                            | multiplicative_expression * cast_expression
//                            | multiplicative_expression % cast_expression
//                            | multiplicative_expression % cast_expression
void parse_multiplicative_expression(pt_node_t *root);

// 6.5.6
// additive_expression := multiplicative_expression
//                      | additive_expression * multiplicative_expression
//                      | additive_expression % multiplicative_expression
void parse_additive_expression(pt_node_t *root);

// 6.5.7
// shift_expression := additive_expression
//                   | shift_expression * additive_expression
//                   | shift_expression % additive_expression
void parse_shift_expression(pt_node_t *root);

// 6.5.8
// relational_expression := shift_expression
//                        | relational_expression < shift_expression
//                        | relational_expression > shift_expression
//                        | relational_expression <= shift_expression
//                        | relational_expression >= shift_expression
void parse_relational_expression(pt_node_t *root);

// 6.5.9
// equality_expression := relational_expression
//                      | equality_expression < relational_expression
//                      | equality_expression > relational_expression
void parse_equality_expression(pt_node_t *root);

// 6.5.10
// and_expression := equality_expression
//                 | and_expression & equality_expression
void parse_and_expression(pt_node_t *root);

// 6.5.11
// exclusive_or_expression := and_expression
//                          | exclusive_or_expression ^ and_expression
void parse_exclusive_or_expression(pt_node_t *root);

// 6.5.12
// inclusive_or_expression := exclusive_or_expression
//                          | inclusive_or_expression | exclusive_or_expression
void parse_inclusive_or_expression(pt_node_t *root);

// 6.5.13
// logical_and_expression := inclusive_or_expression
//                         | logical_and_expression && inclusive_or_expression
void parse_logical_and_expression(pt_node_t *root);

// 6.5.14
// logical_or_expression := logical_and_expression
//                         | logical_or_expression || logical_and_expression
void parse_logical_or_expression(pt_node_t *root);

// 6.5.15
// conditional_expression := logical_or_expression
//                         | logical_or_expression ? expression :
//                         conditional_expression
void parse_conditional_expression(pt_node_t *root);

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
void parse_assignment_expression(pt_node_t *root);

// 6.5.17
// expression := assignment_expression
//             | expression, assignment_expression
void parse_expression(pt_node_t *root);

// 6.6
// constant_expression := conditional_expression
void parse_constant_expression(pt_node_t *root);

// 6.7
// declaration := declaration_specifiers init_declarator_list?;
void parse_declaration(pt_node_t *root);

// 6.7
// storage_class_specifier declaration_specifiers?
// type_specifier declaration_specifiers?
// type_qualifier declaration_specifiers?
// function_specifier declaration_specifiers?
void parse_declaration_specifiers(pt_node_t *root);
bool is_declaration_specifier(token_t token);

// 6.7
// init_declarator_list := init_declarator
//                       | init_declarator_list, init_declarator
void parse_init_declarator_list(pt_node_t *root);

// 6.7
// init_declarator := declarator
//                  | declarator = initializer
void parse_init_declarator(pt_node_t *root);

// 6.7.1
// storage_class_specifier := typedef
//                            extern
//                            static
//                            auto
//                            register
void parse_storage_class_specifier(pt_node_t *root);
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
void parse_type_specifier(pt_node_t *root);
bool is_type_specifier(token_t token);

// 6.7.2.1
// struct_or_union_specifier := struct identifier? {struct_declaration_list}
//                            | struct identifier
//                            | union identifier? {struct_declaration_list}
//                            | union identifier
void parse_struct_or_union_specifier(pt_node_t *root);

// 6.7.2.1
// struct_declaration_list := struct_declaration
//                          | struct_declaration_list struct_declaration
void parse_struct_declaration_list(pt_node_t *root);

// 6.7.2.1
// struct_declaration := specifier_qualifier_list struct_declarator_list;
void parse_struct_declaration(pt_node_t *root);

// 6.7.2.1
// specifier_qualifier_list := type_specifier specifier_qualifier_list?
//                           | type_qualifier specifier_qualifier_list?
void parse_specifier_qualifier_list(pt_node_t *root);

// 6.7.2.1
// struct_declarator_list := struct_declarator
//                         | struct_declarator_list, struct_declarator
void parse_struct_declarator_list(pt_node_t *root);

// 6.7.2.1
// struct_declarator := declarator
//                    | declarator? : constant_expression
void parse_struct_declarator(pt_node_t *root);

// 6.7.2.2
// enum_specifier := enum identifier? {enumerator_list}
//                 | enum identifier? {enumerator_list,}
//                 | enum identifier
void parse_enum_specifier(pt_node_t *root);

// 6.7.2.2
// enumerator_list := enumerator
//                  | enumerator_list, enumerator
void parse_enumerator_list(pt_node_t *root);

// 6.7.2.2
// enumerator := enumeration_constant
//             | enumeration_constant = constant_expression
void parse_enumerator(pt_node_t *root);

// 6.7.3
// type_qualifier := const | restrict | volatile
void parse_type_qualifier(pt_node_t *root);
bool is_type_qualifier(token_t token);

// 6.7.4
// function_specifier := inline
void parse_function_specifier(pt_node_t *root);

// 6.7.5
// declarator := pointer? direct_declarator
void parse_declarator(pt_node_t *root);

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
void parse_direct_declarator(pt_node_t *root);

// 6.7.5
// pointer := *type_qualifier_list?
//          | *type_qualifier_list? pointer
void parse_pointer(pt_node_t *root);

// 6.7.5
// type_qualifier_list := type_qualifier
//                      | type_qualifier_list type_qualifier
void parse_type_qualifier_list(pt_node_t *root);

// 6.7.5
// parameter_type_list := parameter_list
//                      | parameter_list, ...
void parse_parameter_type_list(pt_node_t *root);

// 6.7.5
// parameter_list := parameter_declaration
//                 | parameter_list, parameter_declaration
void parse_parameter_list(pt_node_t *root);

// 6.7.5
// parameter_declaration := declaration_specifiers declarator
//                        | declaration_specifiers abstract_declarator?
void parse_parameter_declaration(pt_node_t *root);

// 6.7.5
// identifier_list := identifier
//                  | identifier_list, identifier
void parse_identifier_list(pt_node_t *root);

// 6.7.6
// type_name := specifier_qualifier_list abstract_declarator?
void parse_type_name(pt_node_t *root);

// 6.7.6
// abstract_declarator := pointer
//                      | pointer? direct_abstract_declarator
void parse_abstract_declarator(pt_node_t *root);

// 6.7.6
// direct_abstract_declarator := (abstract_declarator)
//                             | direct_abstract_declarator?
//                                [type_qualifier_list? assignment_expression?]
//                             | direct_abstract_declarator?
//                                [static type_qualifier_list?
//                                  assignment_expression]
//                             | direct_abstract_declarator?
//                                [type_qualifier_list static
//                                  assignment_expression]
//                             | direct_abstract_declarator? [*]
//                             | direct_abstract_declarator?
//                             (parameter_type_list?)
void parse_direct_abstract_declarator(pt_node_t *root);

// 6.7.7
// typedef_name := identifier
void parse_typedef_name(pt_node_t *root);

// 6.7.8
// initializer := assignment_expression
//              | {initializer_list}
//              | {initializer_list,}
void parse_initializer(pt_node_t *root);

// 6.7.8
// initializer_list := designation? initializer
//                   | initializer_list, designation? initializer
void parse_initializer_list(pt_node_t *root);

// 6.7.8
// designation := designator_list =
void parse_designation(pt_node_t *root);

// 6.7.8
// designator_list := designator
//                  | designator_list designator
void parse_designator_list(pt_node_t *root);

// 6.7.9
// designator := [constant_expression]
//             | .identifier
void parse_designator(pt_node_t *root);

// 6.8
// statement := labeled_statement
//            | compound_statement
//            | expression_statement
//            | selection_statement
//            | iteration_statement
//            | jump_statement
void parse_statement(pt_node_t *root);

// 6.8.1
// labeled_statement := identifier: statement
//                    | case constant_expression: statement
//                    | default: statement
void parse_labeled_statement(pt_node_t *root);

// 6.8.2
// compound_statement := {block_item_list?}
void parse_compound_statement(pt_node_t *root);

// 6.8.2
// block_item_list := block_item
//                  | block_item_list block_item
void parse_block_item_list(pt_node_t *root);

// 6.8.2
// block_item := declaration
//             | statement
void parse_block_item(pt_node_t *root);

// 6.8.3
// expression_statement := expression?;
void parse_expression_statement(pt_node_t *root);

// 6.8.4
// selection_statement := if (expression) statement
//                      | if (expression) statement else statement
//                      | switch (expression) statement
void parse_selection_statement(pt_node_t *root);

// 6.8.5
// iteration_statement := while (expression) statement
//                      | do statement while (expression);
//                      | for (expression?; expression?; expression?) statement
//                      | for (declaration expression?; expression?) statement
void parse_iteration_statement(pt_node_t *root);

// 6.8.6
// jump_statement := goto identifier;
//                 | continue;
//                 | break;
//                 | return expression?;
void parse_jump_statement(pt_node_t *root);

// 6.9
// translation_unit := external_declaration
//                   | translation_unit external_declaration
void parse_translation_unit(token_t *token);

// 6.9
// external_declaration := function_definition
//                       | declaration
void parse_external_declaration(pt_node_t *root);

// 6.9.1
// function_definition
//    := declaration_specifiers declarator declaration_list? compound_statement
void parse_function_definition(pt_node_t *root);

// 6.9.1
// declaration_list := declaration
//                   | declaration_list declaration
void parse_declaration_list(pt_node_t *root);

#endif
