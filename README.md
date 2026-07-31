# Mini Programming Language Compiler

A compiler front-end for a custom mini programming language, built with
Flex (lexical analysis) and Bison (syntax analysis), implemented in C++.

## Project Status
🚧 In development — Milestone 1 (project setup) complete.

## Pipeline
Source Code → Lexer (Flex) → Parser (Bison) → AST → Semantic Analyzer
(Symbol Table + Type Checking) → Three Address Code (TAC)

## Build Requirements
- GCC / g++ (C++17 or later)
- Flex
- Bison
- GNU Make

## Building
```bash
make
```

## Running
```bash
./build/compiler <path-to-source-file>
```

## Team
- Saittajit Paul Soumo
- Arjun Das Shaishab
- Syed Jabedul Islam

## License
Academic project — Compiler Construction Lab, Metropolitan University Bangladesh.