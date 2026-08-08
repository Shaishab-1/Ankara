# Ankara — Mini Programming Language Compiler

A compiler for a small, statically-typed procedural language, built as the term
project for the **Compiler Construction Lab** course (Metropolitan University
Bangladesh). Implemented in **Flex + Bison + C++17**.

The compiler performs full front-end and mid-end compilation:

```
Source (.mc) → Lexer → Parser → AST → Symbol Table → Semantic Analyzer → TAC
```

## Team

| Name | GitHub |
|---|---|
| Arjun Das Shaishab | [Shaishab-1](https://github.com/Shaishab-1) |
| Saittajit Paul Soumo | [saittajitsoumo] |(https://github.com/saittajitsoumo)
| Syed Jabedul Islam | [JABEDSYED] |(https://github.com/JABEDSYED)

## Language Features

- **Types:** `int`, `float`, `bool`
- **Statements:** variable declaration, assignment, `if` / `if-else`, `while`,
  `print`, nested blocks with proper scoping
- **Operators:**
  - Arithmetic: `+  -  *  /  %`
  - Relational: `<  >  <=  >=  ==  !=`
  - Logical: `&&  ||  !`

See `docs/` and `tests/README.md` for concrete example programs.

## Project Structure

```
Ankara/
├── docs/                    # Project report, diagrams, presentation notes
├── examples/                # Sample .mc programs
├── src/
│   ├── lexer/                lexer.l              — Flex specification
│   ├── parser/                parser.y             — Bison grammar + AST construction
│   ├── ast/                   ast.h                — AST node class hierarchy
│   ├── symbol_table/          symbol_table.h/.cpp  — Scoped symbol table
│   ├── semantic/               semantic_analyzer.h/.cpp — Semantic checks
│   └── codegen/                tac_generator.h/.cpp — Three Address Code generator
├── tests/                   # Test programs + tests/README.md (expected outputs)
├── Makefile
└── README.md
```

## Building

### Windows (MSYS2 UCRT64)

```bash
pacman -S make git mingw-w64-ucrt-x86_64-gcc flex bison
make parser-test
```

### Linux / macOS

Install `g++`, `flex`, and `bison` via your package manager
(e.g. `sudo apt install g++ flex bison` on Debian/Ubuntu, or
`brew install flex bison` on macOS), then:

```bash
make parser-test
```

This builds `build/parser_test.exe` (Windows) or `build/parser_test`
(Linux/macOS), linking the parser, lexer, symbol table, semantic analyzer,
and TAC generator into a single executable.

## Running

```bash
./build/parser_test.exe tests/valid_basic.mc      # Windows
./build/parser_test tests/valid_basic.mc           # Linux/macOS
```

For any input file, the compiler will:

1. Tokenize and parse the source, reporting **lexical** and **syntax** errors
   with line numbers.
2. Build and print the **AST**.
3. Run **semantic analysis** (type checking, scope checking, redeclaration
   checking) and report all errors found.
4. If semantic analysis succeeds, generate and print **Three Address Code**.

## Tests

All test programs live in `tests/`, covering valid compilation, lexical
errors, syntax errors, and all six semantic error categories from the
project manual. Full details and exact expected output for every test file
are documented in **[`tests/README.md`](tests/README.md)**.

## Known Limitations

These are documented design trade-offs, not bugs — explained in full in the
Project Report under *Challenges*:

- **Scope-violation vs. undeclared-variable errors** currently produce the
  same error message, since both are detected through the same symbol-table
  lookup failure. Once a block's scope is exited, its variables are removed
  from the table, so a lookup after that point is indistinguishable from the
  variable never having existed.
- **Some semantic errors report line `0`** instead of the actual source
  line. This happens for errors detected inside `BinaryOpNode` /
  `UnaryOpNode`, since these AST nodes do not currently store a line number.
  The error itself is still correctly detected — only the reported line is
  affected.
- **No short-circuit evaluation** in the generated TAC for `&&` / `||`;
  both operands are always evaluated.

## Documentation

- [`docs/project_report.md`](docs/project_report.md) — full project report
  (architecture, design decisions, grammar, challenges, conclusion)
- [`tests/README.md`](tests/README.md) — test suite index with expected
  outputs