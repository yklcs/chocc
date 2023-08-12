#include "parse.h"
#include "lex.h"

#include <locale.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pt_node_t *ast;
token_t *token_ptr;

void append_node(pt_node_t *parent, pt_node_t *child) {
  if (parent == NULL || child == NULL) {
    return;
  }

  if (parent->child) {
    pt_node_t *cur;
    for (cur = parent->child; cur->next; cur = cur->next) {
    }
    cur->next = child;
  } else {
    parent->child = child;
  }
  parent->num_children++;
}

char *stringify_data(pt_node_data_t data, pt_node_kind_t kind) {
  switch (kind) {
  case Identifier:
    return data.Identifier.name;
  case Punctuator:
    return data.Punctuator.text;
  case Keyword:
    return data.Keyword.text;
  case Constant:
    return data.Constant.text;
  default:
    return "";
  }
}

void _print_pt(pt_node_t *root, int depth) {
  printf("%s \033[31m%s\033[0m\n", pt_node_kind_map[root->kind],
         stringify_data(root->data, root->kind));
  for (pt_node_t *sib = root->child; sib; sib = sib->next) {
    for (int i = 0; i < depth; i++) {
      printf("  ");
    }
    _print_pt(sib, depth + 1);
  }
}

void print_pt(pt_node_t *root) {
  _print_pt(root, 1);
}

void throw() {
  printf("parsing error at %s [%d:%d]\n", token_ptr->text, token_ptr->line,
         token_ptr->column);
  exit(1);
}

void match(token_kind_t kind, pt_node_t *parent) {
  if (parent != NULL) {
    pt_node_t *node = calloc(1, sizeof(pt_node_t));
    switch (token_ptr->kind) {
    case Auto:
    case Break:
    case Case:
    case Char:
    case Const:
    case Continue:
    case Default:
    case Do:
    case Double:
    case Else:
    case Enum:
    case Extern:
    case Float:
    case For:
    case Goto:
    case If:
    case Inline:
    case Int:
    case Long:
    case Register:
    case Restrict:
    case Return:
    case Short:
    case Signed:
    case Sizeof:
    case Static:
    case Struct:
    case Switch:
    case Typedef:
    case Union:
    case Unsigned:
    case Void:
    case Volatile:
    case While:
    case Bool:
    case Complex:
    case Imaginary: {
      node->kind = Keyword;
      node->data.Keyword.text = token_ptr->text;
      node->data.Keyword.token_kind = token_ptr->kind;
      break;
    }
    default: {
      node->kind = Punctuator;
      node->data.Punctuator.text = token_ptr->text;
      node->data.Punctuator.token_kind = token_ptr->kind;
    }
    }
    append_node(parent, node);
  }

  if (token_ptr->kind == kind) {
    token_ptr++;
  } else {
    printf("expected %s, got %s\n", token_kind_map[kind],
           token_kind_map[token_ptr->kind]);
    throw();
  }
}

void parse_identifier(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Identifier;
  node->data.Identifier.name = token_ptr->text;
  match(Id, NULL);
  append_node(root, node);
}

void parse_constant(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Constant;
  node->data.Constant.text = token_ptr->text;
  match(Number, NULL);
  append_node(root, node);
}

void parse_string_literal(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = StringLiteral;
  match(String, NULL);
  append_node(root, node);
}

void parse_primary_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = PrimaryExpression;

  switch (token_ptr->kind) {
  case LParen: {
    match(LParen, node);
    parse_expression(node);
    match(RParen, node);
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
  default: {
    throw();
    break;
  }
  }
  append_node(root, node);
}

void parse_postfix_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = PostfixExpression;

  if (token_ptr->kind == LParen && is_type_name(token_ptr)) {
    match(LParen, node);
    parse_type_name(node);
    match(RParen, node);
    match(LBrace, node);
    parse_initializer_list(node);
    if (token_ptr->kind == Comma) {
      match(Comma, node);
    }
    match(RBrace, node);
  } else {
    parse_primary_expression(node);
  }

  bool postfix_done = false;
  for (; !postfix_done;) {
    pt_node_t *node_postfix = calloc(1, sizeof(pt_node_t));
    node_postfix->kind = PostfixExpression;

    switch (token_ptr->kind) {
    case LSquare: {
      match(LSquare, node_postfix);
      parse_expression(node_postfix);
      match(RSquare, node_postfix);
      break;
    }
    case LParen: {
      match(LParen, node_postfix);
      parse_argument_expression_list(node_postfix);
      match(RParen, node_postfix);
      break;
    }
    case Dot: {
      match(Dot, node_postfix);
      parse_identifier(node_postfix);
      break;
    }
    case Arrow: {
      match(Arrow, node_postfix);
      parse_identifier(node_postfix);
      break;
    }
    case PlusPlus: {
      match(PlusPlus, node_postfix);
      break;
    }
    case MinusMinus: {
      match(MinusMinus, node_postfix);
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

void parse_argument_expression_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = ArgumentExpressionList;

  parse_assignment_expression(node);
  if (token_ptr->kind == Comma) {
    match(Comma, node);
    parse_argument_expression_list(node);
  }

  append_node(root, node);
}

void parse_unary_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = UnaryExpression;

  if (token_ptr->kind == PlusPlus || token_ptr->kind == MinusMinus) {
    match(token_ptr->kind, node);
    parse_unary_expression(node);
  } else if (token_ptr->kind == Amp || token_ptr->kind == Star ||
             token_ptr->kind == Plus || token_ptr->kind == Minus ||
             token_ptr->kind == Tilde || token_ptr->kind == Exclaim) {
    match(token_ptr->kind, node);
    parse_cast_expression(node);
  } else if (token_ptr->kind == Sizeof) {
    match(Sizeof, node);
    token_t *sizeof_begin = token_ptr;

    if (token_ptr->kind == LParen) {
      match(LParen, node);
      parse_type_name(node);
      match(RParen, node);
    }
    if (token_ptr->kind == RBrace) { // compound literal, reparse
      node = calloc(1, sizeof(pt_node_t));
      node->kind = UnaryExpression;
      token_ptr = sizeof_begin;
      parse_unary_expression(node);
    }
  } else {
    parse_postfix_expression(node);
  }

  append_node(root, node);
}

void parse_cast_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = CastExpression;

  token_t *lparen_begin = token_ptr;
  if (token_ptr->kind == LParen && is_type_name(token_ptr)) {
    match(LParen, node);
    parse_type_name(node);
    match(RParen, node);
    if (token_ptr->kind == LBrace) { // compound literal, reparse
      node = calloc(1, sizeof(pt_node_t));
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

void parse_multiplicative_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = MultiplicativeExpression;
  for (;;) {
    parse_cast_expression(node);
    if (token_ptr->kind == Star || token_ptr->kind == Slash ||
        token_ptr->kind == Percent) {
      match(token_ptr->kind, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_additive_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = AdditiveExpression;
  for (;;) {
    parse_multiplicative_expression(node);
    if (token_ptr->kind == Plus || token_ptr->kind == Minus) {
      match(token_ptr->kind, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_shift_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = ShiftExpression;
  for (;;) {
    parse_additive_expression(node);
    if (token_ptr->kind == LShft || token_ptr->kind == RShft) {
      match(token_ptr->kind, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_relational_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = RelationalExpression;
  for (;;) {
    parse_shift_expression(node);
    if (token_ptr->kind == Lt || token_ptr->kind == Gt ||
        token_ptr->kind == Leq || token_ptr->kind == Geq) {
      match(token_ptr->kind, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_equality_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = EqualityExpression;
  for (;;) {
    parse_relational_expression(node);
    if (token_ptr->kind == Eq || token_ptr->kind == Neq) {
      match(token_ptr->kind, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_and_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = AndExpression;

  for (;;) {
    parse_equality_expression(node);
    if (token_ptr->kind == Amp) {
      match(token_ptr->kind, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_exclusive_or_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = ExclusiveOrExpression;

  for (;;) {
    parse_and_expression(node);
    if (token_ptr->kind == Caret) {
      match(token_ptr->kind, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_inclusive_or_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = InclusiveOrExpression;

  for (;;) {
    parse_exclusive_or_expression(node);
    if (token_ptr->kind == Bar) {
      match(token_ptr->kind, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_logical_and_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = LogicalAndExpression;

  for (;;) {
    parse_inclusive_or_expression(node);

    if (token_ptr->kind == AmpAmp) {
      match(token_ptr->kind, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_logical_or_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = LogicalOrExpression;

  for (;;) {
    parse_logical_and_expression(node);

    if (token_ptr->kind == BarBar) {
      match(token_ptr->kind, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_conditional_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = ConditionalExpression;

  parse_logical_or_expression(node);

  if (token_ptr->kind == Question) {
    match(Question, node);
    parse_expression(node);
    match(Colon, node);
    parse_conditional_expression(node);
  }

  append_node(root, node);
}

void parse_assignment_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = AssignmentExpression;

  token_t *assignment_begin = token_ptr;
  parse_conditional_expression(node);

  if (token_ptr->kind == Assn || token_ptr->kind == StarAssn ||
      token_ptr->kind == SlashAssn || token_ptr->kind == PercentAssn ||
      token_ptr->kind == PlusAssn || token_ptr->kind == MinusAssn ||
      token_ptr->kind == LShftAssn || token_ptr->kind == RShftAssn ||
      token_ptr->kind == AmpAssn || token_ptr->kind == CaretAssn ||
      token_ptr->kind == BarAssn) { // assignment, reparse
    node = calloc(1, sizeof(pt_node_t));
    node->kind = AssignmentExpression;
    token_ptr = assignment_begin;

    parse_unary_expression(node);
    match(token_ptr->kind, node);
    parse_assignment_expression(node);
  }

  // node->child = NULL;

  append_node(root, node);
}

void parse_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Expression;

  for (;;) {
    parse_assignment_expression(node);
    if (token_ptr->kind == Comma) {
      match(Comma, node);
    } else {
      break;
    }
  }

  append_node(root, node);
}

void parse_constant_expression(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = ConstantExpression;
  parse_conditional_expression(node);
  append_node(node, root);
}

void parse_declaration(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Declaration;
  parse_declaration_specifiers(node);
  if (token_ptr->kind != Comma) {
    parse_init_declarator_list(node);
  }
  match(Semi, node);
  append_node(root, node);
}

void parse_declaration_specifiers(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = DeclarationSpecifiers;
  append_node(root, node);
  while (is_declaration_specifier(*token_ptr)) {
    if (is_storage_class_specifier(*token_ptr)) {
      parse_storage_class_specifier(node);
    } else if (is_type_specifier(*token_ptr)) {
      parse_type_specifier(node);
    } else if (is_type_qualifier(*token_ptr)) {
      parse_type_qualifier(node);
    } else if (token_ptr->kind == Inline) {
      parse_function_specifier(node);
    } else {
      throw();
    }
  }
}

bool is_declaration_specifier(token_t token) {
  if (is_storage_class_specifier(token) || is_type_specifier(token) ||
      is_type_qualifier(token) || token.kind == Inline) {
    return true;
  } else {
    return false;
  }
}

void parse_init_declarator_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = InitDeclaratorList;
  parse_init_declarator(node);
  if (token_ptr->kind == Comma) {
    match(Comma, node);
    parse_init_declarator_list(node);
  }
  append_node(root, node);
}

void parse_init_declarator(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = InitDeclarator;
  parse_declarator(node);
  if (token_ptr->kind == Assn) {
    match(Assn, node);
    parse_initializer(node);
  }
  append_node(root, node);
}

void parse_storage_class_specifier(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = StorageClassSpecifier;

  if (is_storage_class_specifier(*token_ptr)) {
    match(token_ptr->kind, node);
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

void parse_type_specifier(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = TypeSpecifier;

  if (is_type_specifier(*token_ptr)) {
    match(token_ptr->kind, node);
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
  // case Id:
  // return true;
  default:
    return false;
  }
}

void parse_struct_or_union_specifier(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = StructOrUnionSpecifier;

  if (token_ptr->kind == Struct || token_ptr->kind == Union) {
    match(token_ptr->kind, node);
    if (token_ptr->kind == LBrace) {
      match(LBrace, node);
      parse_struct_declaration_list(node);
      match(RBrace, node);
    } else {
      parse_identifier(node);
      if (token_ptr->kind == LBrace) {
        match(LBrace, node);
        parse_struct_declaration_list(node);
        match(RBrace, node);
      }
    }
  } else {
    throw();
  }
  append_node(root, node);
}

void parse_struct_declaration_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = StructDeclarationList;

  parse_struct_declaration(node);
  if (token_ptr->kind != RBrace) {
    parse_struct_declaration_list(node);
  }

  append_node(root, node);
}

void parse_struct_declaration(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = TypeQualifier;

  parse_specifier_qualifier_list(node);
  parse_struct_declarator_list(node);
  match(Semi, node);

  append_node(root, node);
}

void parse_specifier_qualifier_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = TypeQualifier;

  if (is_type_specifier(*token_ptr)) {
    parse_type_specifier(node);
  } else if (is_type_qualifier(*token_ptr)) {
    parse_type_qualifier(node);
  } else {
    throw();
  }
  for (; is_type_specifier(*token_ptr) || is_type_qualifier(*token_ptr);) {
    if (is_type_specifier(*token_ptr)) {
      parse_type_specifier(node);
    } else if (is_type_qualifier(*token_ptr)) {
      parse_type_qualifier(node);
    }
  }

  append_node(root, node);
}

void parse_struct_declarator_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = StructDeclaratorList;

  parse_struct_declarator(node);
  if (token_ptr->kind == Comma) {
    match(Comma, node);
    parse_struct_declarator_list(node);
  }

  append_node(root, node);
}

void parse_struct_declarator(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = StructDeclarator;

  if (token_ptr->kind == Colon) {
    match(Colon, node);
    parse_constant_expression(node);
  } else {
    parse_declarator(node);
    if (token_ptr->kind == Colon) {
      match(Colon, node);
      parse_constant_expression(node);
    }
  }

  append_node(root, node);
}

void parse_enum_specifier(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = EnumSpecifier;

  match(Enum, node);
  if (token_ptr->kind == LBrace) {
    match(LBrace, node);
    parse_enumerator_list(node);
    if (token_ptr->kind == Comma) {
      match(Comma, node);
    }
    match(RBrace, node);
  } else {
    parse_identifier(node);
    if (token_ptr->kind == LBrace) {
      match(LBrace, node);
      parse_enumerator_list(node);
      if (token_ptr->kind == Comma) {
        match(Comma, node);
      }
      match(RBrace, node);
    }
  }

  append_node(root, node);
}

void parse_enumerator_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = EnumeratorList;

  parse_enumerator(node);
  if (token_ptr->kind == Comma) {
    match(Comma, node);
    parse_enumerator_list(node);
  }

  append_node(root, node);
}

void parse_enumerator(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Enumerator;

  parse_identifier(node);
  if (token_ptr->kind == Assn) {
    match(Assn, node);
    parse_constant_expression(node);
  }

  append_node(root, node);
}

void parse_type_qualifier(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = TypeQualifier;

  if (is_type_qualifier(*token_ptr)) {
    match(token_ptr->kind, node);
  } else {
    throw();
  }
  append_node(root, node);
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

void parse_function_specifier(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = FunctionSpecifier;
  match(Inline, node);
  append_node(root, node);
}

void parse_declarator(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Declarator;

  if (token_ptr->kind == Star) {
    parse_pointer(node);
  }

  parse_direct_declarator(node);
  append_node(root, node);
}

void parse_direct_declarator(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = DirectDeclarator;

  if (token_ptr->kind == Id) {
    parse_identifier(node);
  } else if (token_ptr->kind == LParen) {
    match(LParen, node);
    parse_declarator(node);
    match(RParen, node);
  }

  for (; token_ptr->kind == LSquare || token_ptr->kind == LParen;) {
    if (token_ptr->kind == LSquare) {
      match(LSquare, node);

      if (is_type_qualifier(*token_ptr)) {
        parse_type_qualifier_list(node);
        if (token_ptr->kind == Static) {
          match(Static, node);
          parse_assignment_expression(node);
        } else if (token_ptr->kind == Star) {
          match(Star, node);
        } else if (token_ptr->kind != RSquare) {
          parse_assignment_expression(node);
        }
        match(RSquare, node);
      } else if (token_ptr->kind == Static) {
        match(Static, node);
        if (is_type_qualifier(*token_ptr)) {
          parse_type_qualifier_list(node);
        }
        parse_assignment_expression(node);
      } else if (token_ptr->kind == Star) {
        match(Star, node);
      } else if (token_ptr->kind != RSquare) {
        parse_assignment_expression(node);
      }
      match(RSquare, node);
    } else if (token_ptr->kind == LParen) {
      match(LParen, node);
      if (is_declaration_specifier(*token_ptr)) {
        parse_parameter_type_list(node);
      } else if (token_ptr->kind != RParen) {
        parse_identifier_list(node);
      }
      match(RParen, node);
    }
  }

  append_node(root, node);
}

void parse_pointer(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Pointer;

  match(Star, node);

  for (; token_ptr->kind == Star;) {
    parse_pointer(node);
  }

  append_node(root, node);
}

void parse_type_qualifier_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = TypeQualifierList;

  parse_type_qualifier(node);

  for (; is_type_qualifier(*token_ptr);) {
    parse_type_qualifier(node);
  }

  append_node(root, node);
}

void parse_parameter_type_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = ParameterTypeList;

  parse_parameter_list(node);
  if (token_ptr->kind == Comma) {
    match(Comma, node);
    // TODO: figure out ellipsis
  }

  append_node(root, node);
}

void parse_parameter_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = ParameterList;

  parse_parameter_declaration(node);
  for (; token_ptr->kind == Comma;) {
    match(Comma, node);
    parse_parameter_declaration(node);
  }

  append_node(root, node);
}

void parse_parameter_declaration(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = ParameterDeclaration;

  parse_declaration_specifiers(node);
  parse_declarator(node);

  append_node(root, node);
}

void parse_identifier_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = IdentifierList;

  parse_identifier(node);
  for (; token_ptr->kind == Comma;) {
    match(Comma, node);
    parse_identifier(node);
  }
  append_node(root, node);
}

bool is_type_name(token_t *token) {
  return is_type_specifier(*token) || token->kind == Const ||
         token->kind == Restrict || token->kind == Volatile;
}

void parse_type_name(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = TypeName;

  parse_specifier_qualifier_list(node);
  if (token_ptr->kind == Star || token_ptr->kind == LParen ||
      token_ptr->kind == LSquare) {
    parse_abstract_declarator(node);
  }

  append_node(root, node);
}

void parse_abstract_declarator(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = AbstractDeclarator;

  if (token_ptr->kind == Star) {
    parse_pointer(node);
  }
  if (token_ptr->kind == LParen || token_ptr->kind == LSquare) {
    parse_direct_abstract_declarator(node);
  }

  append_node(root, node);
}

void parse_direct_abstract_declarator(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = DirectAbstractDeclarator;

  if (token_ptr->kind == LParen) {
    match(LParen, node);
    if (is_declaration_specifier(*token_ptr)) {
      parse_parameter_type_list(node);
    } else {
      parse_abstract_declarator(node);
    }
    match(RParen, node);
  } else if (token_ptr->kind == LSquare) {
    match(LSquare, node);
    if (token_ptr->kind == Star) {
      match(Star, node);
    } else if (token_ptr->kind == Static) {
      match(Static, node);
      if (is_type_qualifier(*token_ptr)) {
        parse_type_qualifier_list(node);
      }
      parse_assignment_expression(node);
    } else if (is_type_qualifier(*token_ptr)) {
      parse_type_qualifier_list(node);
      if (token_ptr->kind == Static) {
        match(Static, node);
        parse_assignment_expression(node);
      } else if (token_ptr->kind != RSquare) {
        parse_assignment_expression(node);
      }
    } else if (token_ptr->kind != RSquare) {
      parse_assignment_expression(node);
    }

    match(RSquare, node);
  }

  append_node(root, node);
}

void parse_typedef_name(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = TypedefName;

  parse_identifier(node);

  append_node(root, node);
}

void parse_initializer(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Initializer;
  if (token_ptr->kind == LBrace) {
    match(LBrace, node);
    parse_initializer_list(node);
    match(RBrace, node);
    if (token_ptr->kind == Comma) {
      match(Comma, node);
    }
  } else {
    parse_assignment_expression(node);
  }
  append_node(root, node);
}

void parse_initializer_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = InitializerList;

  if (token_ptr->kind == LSquare || token_ptr->kind == Dot) { // designation
    parse_designation(node);
  }
  parse_initializer(node);
  if (token_ptr->kind == Comma) {
    match(Comma, node);
    parse_initializer_list(node);
  }
  append_node(root, node);
}

void parse_designation(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Designation;

  parse_designator_list(node);
  match(Assn, node);

  append_node(root, node);
}

void parse_designator_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = DesignatorList;

  for (; token_ptr->kind == LSquare || token_ptr->kind == Dot;) {
    parse_designator(node);
  }

  append_node(root, node);
}

void parse_designator(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Designator;
  if (token_ptr->kind == LSquare) {
    match(LSquare, node);
    parse_constant_expression(node);
    match(RSquare, node);
  } else if (token_ptr->kind == Dot) {
    match(Dot, node);
    parse_identifier(node);
  } else {
    throw();
  }
  append_node(root, node);
}

void parse_statement(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = Statement;

  if (token_ptr->kind == Case || token_ptr->kind == Default) {
    parse_labeled_statement(node);
  } else if (token_ptr->kind == LBrace) {
    parse_compound_statement(node);
  } else if (token_ptr->kind == If || token_ptr->kind == Switch) {
    parse_selection_statement(node);
  } else if (token_ptr->kind == While || token_ptr->kind == Do ||
             token_ptr->kind == For) {
    parse_iteration_statement(node);
  } else if (token_ptr->kind == Goto || token_ptr->kind == Continue ||
             token_ptr->kind == Break || token_ptr->kind == Return) {
    parse_jump_statement(node);
  } else {
    parse_expression_statement(node);
  }
  append_node(root, node);
}

void parse_labeled_statement(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = LabeledStatement;

  switch (token_ptr->kind) {
  case Case: {
    match(Case, node);
    parse_constant_expression(node);
    match(Colon, node);
    break;
  }
  case Default: {
    match(Default, node);
    match(Colon, node);
    break;
  }
  default: {
    parse_identifier(node);
    match(Colon, node);
  }
  }
  parse_statement(node);

  append_node(root, node);
}

void parse_compound_statement(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = CompoundStatement;
  match(LBrace, node);
  for (; token_ptr->kind != RBrace;) {
    parse_block_item_list(node);
  }
  match(RBrace, node);
  append_node(root, node);
}

void parse_block_item_list(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = BlockItemList;
  for (; token_ptr->kind != RBrace;) {
    parse_block_item(node);
  }
  append_node(root, node);
}

void parse_block_item(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = BlockItem;
  if (is_declaration_specifier(*token_ptr)) {
    parse_declaration(node);
  } else {
    parse_statement(node);
  }
  append_node(root, node);
}

void parse_expression_statement(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = ExpressionStatement;
  if (token_ptr->kind == Semi) {
    match(Semi, node);
  } else {
    parse_expression(node);
    match(Semi, node);
  }
  append_node(root, node);
}

void parse_selection_statement(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = SelectionStatement;

  switch (token_ptr->kind) {
  case If: {
    match(If, node);
    match(LParen, node);
    parse_expression(node);
    match(RParen, node);
    parse_statement(node);
    if (token_ptr->kind == Else) {
      match(Else, node);
      parse_statement(node);
    }
    break;
  }
  case Switch: {
    match(Switch, node);
    match(LParen, node);
    parse_expression(node);
    match(RParen, node);
    parse_statement(node);
    break;
  }
  default: {
    throw();
  }
  }

  append_node(root, node);
}

void parse_iteration_statement(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = IterationStatement;

  switch (token_ptr->kind) {
  case While: {
    match(While, node);
    match(LParen, node);
    parse_expression(node);
    match(RParen, node);
    parse_statement(node);
    break;
  }
  case Do: {
    match(Do, node);
    parse_statement(node);
    match(While, node);
    match(LParen, node);
    parse_expression(node);
    match(RParen, node);
    match(Semi, node);
    break;
  }
  case For: {
    match(For, node);
    match(LParen, node);
    if (is_declaration_specifier(*token_ptr)) {
      parse_declaration(node);
    } else if (token_ptr->kind != Semi) {
      parse_expression(node);
      match(Semi, node);
    } else {
      match(Semi, node);
    }
    if (token_ptr->kind != Semi) {
      parse_expression(node);
      match(Semi, node);
    } else {
      match(Semi, node);
    }
    if (token_ptr->kind != RParen) {
      parse_expression(node);
    }

    match(RParen, node);
    parse_statement(node);
    break;
  }

  default: {
    throw();
  }
  }

  append_node(root, node);
}

void parse_jump_statement(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = JumpStatement;

  switch (token_ptr->kind) {
  case Goto: {
    match(Goto, node);
    parse_identifier(node);
    break;
  }
  case Continue:
  case Break: {
    match(token_ptr->kind, node);
    break;
  }
  case Return: {
    match(Return, node);
    if (token_ptr->kind != Semi) {
      parse_expression(node);
    }
    break;
  }
  default: {
    throw();
  }
  }
  match(Semi, node);
  append_node(root, node);
}

void parse_translation_unit(token_t *tokens) {
  token_ptr = tokens;

  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = TranslationUnit;
  ast = node;
  for (; token_ptr->kind != Eof;) {
    parse_external_declaration(node);
  }
}

void parse_external_declaration(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = ExternalDeclaration;

  token_t *begin = token_ptr;
  for (; is_declaration_specifier(*token_ptr); token_ptr++) {
  }
  parse_declarator(NULL);
  if (token_ptr->kind == LBrace) {
    token_ptr = begin;
    parse_function_definition(node);
  } else {
    token_ptr = begin;
    parse_declaration(node);
  }

  append_node(root, node);
}

void parse_function_definition(pt_node_t *root) {
  pt_node_t *node = calloc(1, sizeof(pt_node_t));
  node->kind = FunctionDefinition;
  parse_declaration_specifiers(node);
  parse_declarator(node);
  // TODO: support K&R style function declarations
  parse_compound_statement(node);
  append_node(root, node);
}
