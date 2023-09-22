# chocc

A C compiler from scratch 🍫

---

The goal is to write a self hosting ANSI C (C89/ISO C90) compiler in ANSI C.
Everything is hand written without generators.
The parser uses recursive descent.
Tree-walking interpretation and WebAssembly codegen backends are planned.

## Steps

1. Lex C
2. Parse C ← _currently here_
3. Implement preprocessor
4. Generate AST from parse tree
5. Create a tree-walking interpreter backend
6. Create a WebAssembly codegen backend
