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
int (*fn)(int[128], char **, int (*)[]);
int *(*(*(*foo)(char))(double))[3];
char *f(int, char *[], void (*)(int, char[]));
char(*(*x[3])());
int func(int a, int b, int c);
int xx(char c) {
  int *arr[5];
  c = a > b ? (5 % 2) && 1 : x[1];
  func(0, 1 * 0, *ptr);
  strct.mem = 0;
}
```

AST:

```
Decl fn: *((Int[128], **Char, *(Int[])) -> Int)
Decl foo: *((Char) -> *((Double) -> *(*Int[3])))
Decl f: (Int, *Char[], *((Int, Char[]) -> Void)) -> *Char
Decl x: *((Void) -> *Char)[3]
Decl func: (a: Int, b: Int, c: Int) -> Int
FnDefn xx(c: Char) -> Int
 `-Decl arr: *Int[5]
 `-InfixExpr: Assn
  `-Ident: c
  `-InfixExpr: Question
   `-InfixExpr: Gt
    `-Ident: a
    `-Ident: b
   `-InfixExpr: AmpAmp
    `-InfixExpr: Percent
     `-Lit: 5
     `-Lit: 2
    `-Lit: 1
   `-PostfixExpr: LBrack
    `-Ident: x
 `-CallExpr
  `-Ident: func
  `-CommaExpr
   `-Lit: 0
   `-InfixExpr: Star
    `-Lit: 1
    `-Lit: 0
   `-PrefixExpr: Star
    `-Ident: ptr
 `-InfixExpr: Assn
  `-PostfixExpr: Dot
   `-Ident: strct
  `-Lit: 0
```

## Steps

1. Lex C
2. Parse C ← _currently here_
3. Implement preprocessor
4. Generate AST from parse tree
5. Create a tree-walking interpreter backend
6. Create a WebAssembly codegen backend
