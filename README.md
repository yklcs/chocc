# chocc

A C compiler from scratch 🍫

---

The goal is to write a self hosting C99 compiler in C99.
Everything is hand written without generators.
The parser uses recursive descent based on the C99 syntax.
Tree-walking interpretation and WebAssembly codegen backends are planned.

## Lexer

## C99 standard compliance

C features with currently no plans for _initial_ support:

- VLAs
- Bit fields

## Steps

1. Lex C99
2. Parse C99 ← _currently here_
3. Implement preprocessor
4. Generate AST from parse tree
5. Create a tree-walking interpreter backend
6. Create a WebAssembly codegen backend
