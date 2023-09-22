# chocc

A C compiler from scratch 🍫

---

The goal is to write a self hosting ANSI C (C89/ISO C90) compiler in ANSI C.
Everything is hand written without generators.
The parser uses recursive descent.
Tree-walking interpretation and WebAssembly codegen backends are planned.

## Status

In:

```c
int (*fn)(int[128], char **, int (*)[]);
int *(*(*(*foo)(char))(double))[3];
char *f(int, char *[], void (*)(int, char[]));
char(*(*x[3])());
int *function(int x, char y) {
  int *arr[5];
  char(*(*z[3])())[5];
  char((*k)());
}
```

AST:

```
Decl fn: *((Int[128], **Char, *(Int[])) -> Int)
Decl foo: *((Char) -> *((Double) -> *(*Int[3])))
Decl f: (Int, *Char[], *((Int, Char[]) -> Void)) -> *Char
Decl x: *((Void) -> *Char)[3]
FnDefn function(x: Int, y: Char) -> *Int
 `-Decl arr: *Int[5]
 `-Decl z: *((Void) -> *(Char[5]))[3]
 `-Decl k: *((Void) -> Char)
```

## Steps

1. Lex C
2. Parse C ← _currently here_
3. Implement preprocessor
4. Generate AST from parse tree
5. Create a tree-walking interpreter backend
6. Create a WebAssembly codegen backend
