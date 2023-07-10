#include "parse.h"
#include "lex.h"

#include <locale.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

ast_node_t *ast;
token_t *token_ptr;

void append_node(ast_node_t *parent, ast_node_t *child) {
  if (child == NULL) {
    return;
  }
  printf("%s -> %s\n", ast_node_kind_map[parent->kind],
         ast_node_kind_map[child->kind]);

  if (parent->child) {
    ast_node_t *cur;
    for (cur = parent->child; cur->next; cur = cur->next) {
    }
    cur->next = child;
  } else {
    parent->child = child;
  }
  parent->num_children++;
}

void _print_ast(ast_node_t *root, int depth) {
  // if (root->num_children != 1)
  printf("%s %d\n", ast_node_kind_map[root->kind], root->num_children);
  for (ast_node_t *sib = root->child; sib; sib = sib->next) {
    for (int i = 0; i < depth; i++) {
      printf("  ");
    }
    _print_ast(sib, depth + 1);
  }
}

void print_ast(ast_node_t *root) {
  _print_ast(root, 1);
}

void throw() {
  printf("parsing error at %s [%d:%d]\n", token_ptr->text, token_ptr->line,
         token_ptr->column);
  exit(1);
}

void match(token_kind_t kind) {
  if (token_ptr->kind == kind) {
    token_ptr++;
  } else {
    printf("expected %s, got %s\n", token_kind_map[kind],
           token_kind_map[token_ptr->kind]);
    throw();
  }
}

void parse_identifier(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = Identifier;
  match(Id);
  append_node(root, node);
}

void parse_constant(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = Constant;
  match(Number);
  append_node(root, node);
}

void parse_string_literal(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = StringLiteral;
  match(String);
  append_node(root, node);
}

void parse_primary_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = PrimaryExpression;
  switch (token_ptr->kind) {
  case LParen: {
    match(LParen);
    parse_expression(node);
    match(RParen);
    break;
  }
  case Number: {
    parse_constant(node);
    break;
  }
  case String: {
    parse_string_literal(node);
    break;
  }
  case Id: {
    parse_identifier(node);
    break;
  }
  }
  append_node(root, node);
}

void parse_postfix_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = PostfixExpression;

  if (token_ptr->kind == LParen) {
    match(LParen);
    // TODO: parse type_name
    match(RParen);
    match(LBrace);
    // TODO: parse initializer_list
    if (token_ptr->kind == Comma) {
      match(Comma);
    }
    match(RBrace);
  } else {
    parse_primary_expression(node);
  }

  bool postfix_done = false;
  for (; !postfix_done;) {
    ast_node_t *node_postfix = calloc(1, sizeof(ast_node_t));
    node_postfix->kind = PostfixExpression;

    switch (token_ptr->kind) {
    case LSquare: {
      match(LSquare);
      parse_expression(node_postfix);
      match(RSquare);
      break;
    }
    case LParen: {
      match(LParen);
      parse_argument_expression_list(node_postfix);
      match(RParen);
      break;
    }
    case Dot: {
      match(Dot);
      parse_identifier(node_postfix);
      break;
    }
    case Arrow: {
      match(Arrow);
      parse_identifier(node_postfix);
      break;
    }
    case PlusPlus: {
      match(PlusPlus);
      break;
    }
    case MinusMinus: {
      match(MinusMinus);
      break;
    }
    default: {
      free(node_postfix);
      node_postfix = NULL;
      postfix_done = true;
    }
    }
    append_node(node, node_postfix);
  }

  append_node(root, node);
}

void parse_argument_expression_list(ast_node_t *root) {
  // TODO
}

void parse_unary_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = UnaryExpression;

  if (token_ptr->kind == PlusPlus || token_ptr->kind == MinusMinus) {
    match(token_ptr->kind);
    parse_unary_expression(node);
  } else if (token_ptr->kind == Amp || token_ptr->kind == Star ||
             token_ptr->kind == Plus || token_ptr->kind == Minus ||
             token_ptr->kind == Tilde || token_ptr->kind == Exclaim) {
    match(token_ptr->kind);
    parse_cast_expression(node);
  } else if (token_ptr->kind == Sizeof) {
    match(Sizeof);
    token_t *sizeof_begin = token_ptr;

    if (token_ptr->kind == LParen) {
      match(LParen);
      // TODO: parse type_name
      match(RParen);
    }
    if (token_ptr->kind == RBrace) { // compound literal, reparse
      node = calloc(1, sizeof(ast_node_t));
      node->kind = UnaryExpression;
      token_ptr = sizeof_begin;
      parse_unary_expression(node);
    }
  } else {
    parse_postfix_expression(node);
  }
  append_node(root, node);
}

void parse_cast_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = CastExpression;

  token_t *lparen_begin = token_ptr;
  if (token_ptr->kind == LParen) {
    match(LParen);
    // TODO: parse type_name
    match(RParen);
    if (token_ptr->kind == LBrace) { // compound literal, reparse
      node = calloc(1, sizeof(ast_node_t));
      node->kind = CastExpression;
      token_ptr = lparen_begin;
      parse_unary_expression(node);
    } else {
      parse_cast_expression(node);
    }
  } else {
    parse_unary_expression(node);
  }
  append_node(root, node);
}

void parse_multiplicative_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = MultiplicativeExpression;
  for (;;) {
    parse_cast_expression(node);
    if (token_ptr->kind == Star || token_ptr->kind == Slash ||
        token_ptr->kind == Percent) {
      match(token_ptr->kind);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_additive_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = AdditiveExpression;
  for (;;) {
    parse_multiplicative_expression(node);
    if (token_ptr->kind == Plus || token_ptr->kind == Minus) {
      match(token_ptr->kind);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_shift_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = ShiftExpression;
  for (;;) {
    parse_additive_expression(node);
    if (token_ptr->kind == LShft || token_ptr->kind == RShft) {
      match(token_ptr->kind);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_relational_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = RelationalExpression;
  for (;;) {
    parse_shift_expression(node);
    if (token_ptr->kind == Lt || token_ptr->kind == Gt ||
        token_ptr->kind == Leq || token_ptr->kind == Geq) {
      match(token_ptr->kind);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_equality_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = EqualityExpression;
  for (;;) {
    parse_relational_expression(node);
    if (token_ptr->kind == Eq || token_ptr->kind == Neq) {
      match(token_ptr->kind);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_and_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = AndExpression;

  for (;;) {
    parse_equality_expression(node);
    if (token_ptr->kind == Amp) {
      match(token_ptr->kind);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_exclusive_or_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = ExclusiveOrExpression;

  for (;;) {
    parse_and_expression(node);
    if (token_ptr->kind == Caret) {
      match(token_ptr->kind);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_inclusive_or_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = InclusiveOrExpression;

  for (;;) {
    parse_exclusive_or_expression(node);
    if (token_ptr->kind == Bar) {
      match(token_ptr->kind);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_logical_and_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = LogicalAndExpression;

  for (;;) {
    parse_inclusive_or_expression(node);

    if (token_ptr->kind == AmpAmp) {
      match(token_ptr->kind);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_logical_or_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = LogicalOrExpression;

  for (;;) {
    parse_logical_and_expression(node);

    if (token_ptr->kind == BarBar) {
      match(token_ptr->kind);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_conditional_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = ConditionalExpression;

  parse_logical_or_expression(node);

  if (token_ptr->kind == Question) {
    match(Question);
    parse_expression(node);
    match(Colon);
    parse_conditional_expression(node);
  }

  append_node(root, node);
}

void parse_assignment_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = AssignmentExpression;

  token_t *assignment_begin = token_ptr;

  parse_conditional_expression(node);

  if (token_ptr->kind == Assn || token_ptr->kind == StarAssn ||
      token_ptr->kind == SlashAssn || token_ptr->kind == PercentAssn ||
      token_ptr->kind == PlusAssn || token_ptr->kind == MinusAssn ||
      token_ptr->kind == LShftAssn || token_ptr->kind == RShftAssn ||
      token_ptr->kind == AmpAssn || token_ptr->kind == CaretAssn ||
      token_ptr->kind == BarAssn) { // assignment, reparse
    node = calloc(1, sizeof(ast_node_t));
    node->kind = AssignmentExpression;
    token_ptr = assignment_begin;

    parse_unary_expression(node);
    match(token_ptr->kind);
    parse_assignment_expression(node);
  }

  append_node(root, node);
}

void parse_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = Expression;

  for (;;) {
    parse_assignment_expression(node);
    if (token_ptr->kind == Comma) {
      match(Comma);
    } else {
      break;
    }
  }
  append_node(root, node);
}

void parse_constant_expression(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = ConstantExpression;
  parse_conditional_expression(node);
  append_node(node, root);
}

void parse_declaration(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = Declaration;
  parse_declaration_specifiers(node);
  if (token_ptr->kind != Comma) {
    parse_init_declarator_list(node);
  }
  match(Semi);
  append_node(root, node);
}

void parse_declaration_specifiers(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = DeclarationSpecifiers;
  append_node(root, node);
  while (is_declaration_specifier(*token_ptr)) {
    if (is_storage_class_specifier(*token_ptr)) {
      parse_storage_class_specifier(node);
    } else if (is_type_specifier(*token_ptr)) {
      parse_type_specifier(node);
    }
  }
}

bool is_declaration_specifier(token_t token) {
  if (is_storage_class_specifier(token) || is_type_specifier(token) ||
      is_type_qualifier(token)) {
    return true;
  } else {
    return false;
  }
}

void parse_init_declarator_list(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = InitDeclaratorList;
  parse_init_declarator(node);
  if (token_ptr->kind == Comma) {
    match(Comma);
    parse_init_declarator_list(node);
  }
  append_node(root, node);
}

void parse_init_declarator(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = InitDeclarator;
  parse_declarator(node);
  if (token_ptr->kind == Assn) {
    match(Assn);
    parse_initializer(node);
  }
  append_node(root, node);
}

void parse_storage_class_specifier(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = StorageClassSpecifier;

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
  node->kind = TypeSpecifier;

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

void parse_type_qualifier(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = TypeQualifier;

  if (is_type_qualifier(*token_ptr)) {
    match(token_ptr->kind);
    append_node(root, node);
  } else {
    throw();
  }
}

bool is_type_qualifier(token_t token) {
  switch (token.kind) {
  case Const:
  case Restrict:
  case Volatile:
    return true;
  default:
    return false;
    ;
  }
}

void parse_declarator(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = Declarator;

  if (token_ptr->kind == Star) {
    parse_pointer(node);
  }

  parse_direct_declarator(node);
  append_node(root, node);
}

void parse_direct_declarator(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = DirectDeclarator;

  if (token_ptr->kind == Id) {
    parse_identifier(node);
  } else if (token_ptr->kind == LParen) {
    match(LParen);
    parse_declarator(node);
    match(RParen);
  }

  for (; token_ptr->kind == LSquare || token_ptr->kind == LParen;) {
    // parse_direct_declarator(node);
    if (token_ptr->kind == LSquare) {
      match(LSquare);
      match(RSquare);
    } else if (token_ptr->kind == LParen) {
      match(LParen);
      parse_parameter_type_list(node);
      match(RParen);
    }
  }

  append_node(root, node);
}

void parse_pointer(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = Pointer;

  match(Star);

  for (; token_ptr->kind == Star;) {
    parse_pointer(node);
  }

  append_node(root, node);
}

void parse_type_qualifier_list(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = TypeQualifierList;

  parse_type_qualifier(node);

  for (; is_type_qualifier(*token_ptr);) {
    parse_type_qualifier(node);
  }

  append_node(root, node);
}

void parse_parameter_type_list(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = ParameterTypeList;

  parse_parameter_list(node);
  if (token_ptr->kind == Comma) {
    match(Comma);
    // TODO: figure out ellipsis
  }

  append_node(root, node);
}

void parse_parameter_list(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = ParameterList;

  parse_parameter_declaration(node);
  for (; token_ptr->kind == Comma;) {
    match(Comma);
    parse_parameter_declaration(node);
  }

  append_node(root, node);
}

void parse_parameter_declaration(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = ParameterDeclaration;

  parse_declaration_specifiers(node);
  parse_declarator(node);

  append_node(root, node);
}

void parse_identifier_list(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = IdentifierList;

  parse_identifier(node);
  for (; token_ptr->kind == Comma;) {
    match(Comma);
    parse_identifier(node);
  }
  append_node(root, node);
}

void parse_initializer(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = Initializer;
  if (token_ptr->kind == LBrace) {
    match(LBrace);
    parse_initializer_list(node);
    match(RBrace);
    if (token_ptr->kind == Comma) {
      match(Comma);
    }
  } else {
    parse_assignment_expression(node);
  }
  append_node(root, node);
}

void parse_initializer_list(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = InitializerList;

  if (token_ptr->kind == LSquare || token_ptr->kind == Dot) { // designation
    parse_designation(node);
  }
  parse_initializer(node);
  if (token_ptr->kind == Comma) {
    match(Comma);
    parse_initializer_list(node);
  }
  append_node(root, node);
}

void parse_designation(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = Designation;

  parse_designator_list(node);
  match(Assn);

  append_node(root, node);
}

void parse_designator_list(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = DesignatorList;

  for (; token_ptr->kind == LSquare || token_ptr->kind == Dot;) {
    parse_designator(node);
  }

  append_node(root, node);
}

void parse_designator(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = Designator;
  if (token_ptr->kind == LSquare) {
    match(LSquare);
    parse_constant_expression(node);
    match(RSquare);
  } else if (token_ptr->kind == Dot) {
    match(Dot);
    parse_identifier(node);
  } else {
    throw();
  }
  append_node(root, node);
}

void parse_statement(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = Statement;
  if (token_ptr->kind == LBrace) {
    parse_compound_statement(node);
  } else {
    parse_expression_statement(node);
  }
  append_node(root, node);
}

void parse_compound_statement(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = CompoundStatement;
  match(LBrace);
  for (; token_ptr->kind != RBrace;) {
    parse_block_item_list(node);
  }
  match(RBrace);
  append_node(root, node);
}

void parse_block_item_list(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = BlockItemList;
  for (; token_ptr->kind != RBrace;) {
    parse_block_item(node);
  }
  append_node(root, node);
}

void parse_block_item(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = BlockItem;
  if (is_declaration_specifier(*token_ptr)) {
    parse_declaration(node);
  } else {
    parse_statement(node);
  }
  append_node(root, node);
}

void parse_expression_statement(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = ExpressionStatement;
  if (token_ptr->kind == Semi) {
    match(Semi);
  } else {
    parse_expression(node);
    match(Semi);
  }
  append_node(root, node);
}

void parse_translation_unit(token_t *tokens) {
  token_ptr = tokens;

  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = TranslationUnit;
  ast = node;
  parse_external_declaration(node);
}

void parse_external_declaration(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = ExternalDeclaration;
  append_node(root, node);
  parse_function_definition(node);
}

void parse_function_definition(ast_node_t *root) {
  ast_node_t *node = calloc(1, sizeof(ast_node_t));
  node->kind = FunctionDefinition;
  append_node(root, node);
  parse_declaration_specifiers(node);
  parse_declarator(node);
  // Don't support K&R style function declarations
  parse_compound_statement(node);
}
