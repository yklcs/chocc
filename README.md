# chocc

A C compiler from scratch 🍫

---

The goal is to write a self hosting ANSI C (C89/ISO C90) compiler in ANSI C.
Everything is hand written without generators.
The parser uses recursive descent.
Tree-walking interpretation and WebAssembly codegen backends are planned.

## Status

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
Decl foo: *((Const Char) -> *((Double) -> *(*Int[3])))
Decl x: Static Const **Int[5]
Decl v2: Struct vec2 { x: Int, y: Int }
 `-List (len 2)
   |-Lit: 0
   `-Lit: 1
Decl colors: Typedef Enum Colors { Red = ?, Blue, Green }
Decl func: Static (a: Int, b: Int, c: Volatile Int) -> Int
FnDefn func(x: Int) -> Int
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
   |     `-InfixExpr: Sx  tarAssn
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

## Steps

1. Lex C
2. Parse C ← _currently here_
3. Implement preprocessor
4. Generate AST from parse tree
5. Create a tree-walking interpreter backend
6. Create a WebAssembly codegen backend
