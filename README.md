# chocc

An ANSI C compiler in ANSI C.

---

The goal is a self hosting ANSI C (C89/ISO C90[^1]) compiler in ANSI C without external dependencies.
The main functionality of the frontend is complete.
Tree-walking interpretation and WebAssembly codegen backends are planned.
The design is as follows.

[A character stream](./io.c) is created for each source code file.
The physical location (line/column) of each character is stored together with the character as the physical location and logical location of characters may differ during translation (e.g. due to newline splicing).

[Tokenization and lexing](./lex.c) are performed character-by-character with a pseudo finite state machine. Some whitespace characters are lexed into tokens as they are significant during preprocessing.

[Preprocessing](./cpp.c) is performed on the lexer output.
After preprocessing, preprocessing directive tokens and whitespace tokens are removed.

[The parser](./parse.c) is ad-hoc with a recursive descent core.
Top down operator precendence ("Pratt") parsing[^2][^3] is used for expressions.
Naive backtracking is also used in some parts.
The parser outputs AST nodes represented as tagged unions.
Types are represented by a tree, and are constructed from declaration specifiers and declarators.

Above is the extent of the current implementation.
No efforts at optimization have been made.
Allocated memory is not freed.

## Todo

- Implement all preprocessor directives
- Lex/parse floating and non-decimal radix literals
- Symbol table and scoping
- Evaluate integer constant expressions at compile time
- Lvalue semantic analysis
- Type analysis
- Tree walking interpretation
- WebAssembly codegen
- WebAssembly runtime

## Example

Input:

```c
#define A 10 + 1

int (*fn)(i\
nt[128],
          char **, in\
t (*)[]);
int *(*(*(*foo)(const char))(double)) /* block comment */[3];
static const int **x[5];
struct vec2 {
  int x, y;
} v2 = {0, 1};
typedef enum Colors { Red = 1, Blue, Green } colors;
static int func(int a, int b, volatile int c);
int func(int x) {
  int i = 0, *j, k = 2;
  if (a == b)
    i = A ? 1 : -1;
  else if (1)
    i *= v2.x;

  while (1) {
    i++ + y;
  }

  return 1;
}
```

AST:

```
Decl fn: *((Int[128], **Char, *(Int[])) -> Int)
Decl foo: *(Const Char -> *(Double -> *((*Int)[3])))
Decl x: Static (** Const Int)[5]
Decl v2: Struct vec2 { x: Int, y: Int }
`-List (len 2)
  |-Lit: 0
  `-Lit: 1
Decl colors: Typedef Enum Colors { Red = ?, Blue, Green }
Decl func: Static (a: Int, b: Int, c: Volatile Int) -> Int
FnDefn funcx: Int -> Int
`-BlockStmt
  |-Decl i: Int
  | `-Lit: 0
  |-Decl j: *Int
  |-Decl k: Int
  | `-Lit: 2
  |-IfElseStmt
  | |-InfixExpr: Eq
  | | |-Ident: a
  | | `-Ident: b
  | |-ExprStmt
  | | `-InfixExpr: Assn
  | |   |-Ident: i
  | |   `-InfixExpr: Question
  | |     |-InfixExpr: Plus
  | |     | |-Lit: 10
  | |     | `-Lit: 1
  | |     |-Lit: 1
  | |     `-PrefixExpr: Minus
  | |       `-Lit: 1
  | `-IfStmt
  |   |-Lit: 1
  |   `-ExprStmt
  |     `-InfixExpr: StarAssn
  |       |-Ident: i
  |       `-PostfixExpr: Dot
  |         |-Ident: v2
  |         `-Ident: x
  |-WhileStmt
  | |-Lit: 1
  | `-BlockStmt
  |   `-ExprStmt
  |     `-InfixExpr: Plus
  |       |-PostfixExpr: PlusPlus
  |       | |-Ident: i
  |       `-Ident: y
  `-JumpStmt Return
    `-Lit: 1
```

[^1]: Based mostly on _The C Programming Language_ 2E (K&R). Actual ISO/ANSI standards are hard to find.
[^2]: Vaughn Pratt, _Top Down Operator Precendence_ (1973).
[^3]: <https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html>
