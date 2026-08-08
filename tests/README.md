# Test Suite Index

This document lists every test program under `tests/`, what it
demonstrates, and its exact expected output, per Section 15 of the
project manual.

## How to run any test

```bash
make parser-test
./build/parser_test.exe tests/<filename>.mc
```

## 1. Valid Compilation

### `valid_basic.mc`
Full-featured valid program (declarations, assignment, while,
if-else, print) matching the manual's Section 5.5 sample.
**Expected:** Parses successfully, no semantic errors, and prints
its AST + TAC.

### `tac_test.mc`
Focused test for Three Address Code generation: arithmetic
precedence, if-else, while.
**Expected:** `No semantic errors found.` followed by TAC with
temporaries (`t1`, `t2`...) and labels (`L1`, `L2`...).

## 2. Lexical Errors

### `lexical_error.mc`
Contains the invalid character `@`.
**Expected:**

Lexical Error at line 2: unexpected character '@'

Compilation continues past the bad character; the rest of the
tokens on the line are still recognized correctly.

## 3. Syntax Errors

### `syntax_error.mc`
Incomplete expression: `x = 5 +;`
**Expected:**

Syntax Error at line 2: syntax error

The parser recovers via the `error SEMICOLON` rule instead of
crashing or stopping immediately.

## 4. Semantic Errors (Section 4.5, one test per rule)

### `semantic_undeclared.mc` — Undeclared variable use
**Expected:**

Semantic Error at line 3: use of undeclared variable 'y'
Compilation failed: 1 semantic error(s)


### `semantic_redeclare.mc` — Redeclaration
**Expected:**

Semantic Error at line 2: redeclaration of variable 'x'
Compilation failed: 1 semantic error(s)


### `semantic_scope_violation.mc` — Scope violation
`y` is declared inside a nested block and used after the block ends.
**Expected:**

Semantic Error at line 6: use of undeclared variable 'y'
Compilation failed: 1 semantic error(s)

**Design note:** our analyzer detects scope violations and
undeclared-variable use through the same symbol-table lookup
mechanism, so both produce this message. Once a block's scope is
exited, its variables are removed from the table — a lookup
failure afterward is indistinguishable from the variable never
having been declared at all. This is documented behavior, not a bug.

### `semantic_type_mismatch.mc` — Type mismatch
**Expected:**

Semantic Error at line 2: type mismatch: cannot assign int to variable 'flag' of type bool
Compilation failed: 1 semantic error(s)


### `semantic_invalid_assignment.mc` — Invalid assignment
Assigning a `bool` value to an `int` variable.
**Expected:**

Semantic Error at line 4: type mismatch: cannot assign bool to variable 'x' of type int
Compilation failed: 1 semantic error(s)


### `semantic_invalid_expression.mc` — Invalid expression
Using `&&` on an `int` operand.
**Expected:**

Semantic Error at line 0: logical operator '&&' requires bool operands
Compilation failed: 1 semantic error(s)

**Known limitation:** `BinaryOpNode` does not currently store a
line number, so expression-level errors report line `0` instead of
the real source line. This is a documented limitation, not
incorrect detection — the error itself is still correctly caught.

## 5. Supporting / Demonstration Files

### `ast_print.mc`
Small if-else program used during Milestone 4 to visually verify
AST tree-printing.

### `scope_test.mc`
Demonstrates the parser's own Symbol Table entering and exiting a
nested scope (Milestone 5); not a semantic-error test.