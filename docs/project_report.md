# Ankara — Mini Programming Language Compiler
## Project Report

**Course:** Compiler Construction Lab
**Institution:** Metropolitan University Bangladesh

**Team:**
- Arjun Das Shaishab
- Saittajit Paul Soumo
- Syed Jabedul Islam

---

## 1. Introduction

Ankara is a compiler for a small, statically-typed procedural programming
language, built to satisfy the requirements of the Compiler Construction Lab
term project. The project follows the classical multi-phase compiler
architecture taught in the course: a source program is first broken into
tokens by a **lexical analyzer**, those tokens are checked against the
language's grammar and assembled into a syntax tree by a **parser**, that
tree is checked for type and scope correctness by a **semantic analyzer**
using a **symbol table**, and finally a simplified, linear intermediate
representation — **Three Address Code (TAC)** — is generated from the
validated tree.

Rather than treating these phases as one large program, we implemented and
tested each phase independently, as its own milestone, before wiring it into
the next. This let us verify each stage in isolation (e.g., confirming the
lexer tokenizes correctly before ever handing its output to a parser) and
made debugging tractable when problems did occur, since a failure could
usually be traced to a single, recently-changed phase in the pipeline.

The implementation uses **Flex** for lexical analysis, **Bison** for parsing,
and **C++17** for the AST, symbol table, semantic analyzer, and TAC
generator — a standard, industry-representative toolchain for building
recursive-descent-free, LALR-based compilers.

## 2. Objectives

The project set out to:

1. Design and implement a **lexical analyzer** that correctly tokenizes all
   language constructs and reports lexical errors (invalid characters) with
   line numbers, without halting on the first error.
2. Design and implement a **Bison-based parser** that validates programs
   against a formal context-free grammar, correctly resolves operator
   precedence and associativity, and recovers from syntax errors well enough
   to continue checking the rest of the file.
3. Build an **Abstract Syntax Tree (AST)** representation that captures the
   full structure of a parsed program, independent of the concrete grammar
   used to parse it.
4. Implement a **scoped symbol table** supporting nested block scopes, used
   both during semantic analysis and (implicitly) during code generation.
5. Implement a **semantic analyzer** enforcing the language's static
   semantics: no undeclared-variable use, no redeclaration within a scope,
   scope-correct variable visibility, and type-correct assignments and
   expressions.
6. Implement a **Three Address Code generator** that lowers a semantically
   valid AST into a linear sequence of simple instructions with temporaries
   and labels, suitable as an intermediate step toward real code generation.
7. Build a **test suite** demonstrating each of the above in isolation, with
   documented, verified expected output for every test case.
8. Document the design honestly, including known limitations, in line with
   the course's AI-usage policy that every team member fully understand and
   be able to explain all submitted code.

## 3. Language Specification

### 3.1 Data Types

| Type | Description |
|---|---|
| `int` | Signed integer |
| `float` | Floating-point number |
| `bool` | Boolean (`true` / `false`) |

The language performs one implicit conversion: an `int` value may be
assigned to a `float` variable (widening). All other cross-type assignments
are rejected by the semantic analyzer.

### 3.2 Lexical Tokens

**Keywords:** `int`, `float`, `bool`, `if`, `else`, `while`, `print`, `true`, `false`

**Operators:**

| Category | Operators |
|---|---|
| Arithmetic | `+` `-` `*` `/` `%` |
| Relational | `<` `>` `<=` `>=` `==` `!=` |
| Logical | `&&` `\|\|` `!` |
| Assignment | `=` |

**Punctuation:** `;` `,` `(` `)` `{` `}`

**Literals:** integer constants (`[0-9]+`), floating-point constants
(`[0-9]+\.[0-9]+`), identifiers (`[a-zA-Z_][a-zA-Z0-9_]*`)

**Comments:** `// single-line` and `/* block */`, both discarded by the
lexer and never seen by the parser.

### 3.3 Formal Grammar (CFG)

Below is the context-free grammar exactly as implemented in
`src/parser/parser.y`, given in a simplified BNF-like notation (Bison's
actual `%left`/`%right` precedence declarations, which resolve ambiguity in
the `expr` rule without needing it to be written unambiguously, are noted
separately below).

```
program      → stmt_list

stmt_list    → ε
             | stmt_list stmt

stmt         → decl_stmt
             | assign_stmt
             | if_stmt
             | while_stmt
             | print_stmt
             | block
             | error ';'                      (* syntax-error recovery *)

type         → 'int' | 'float' | 'bool'

decl_stmt    → type ID ';'

assign_stmt  → ID '=' expr ';'

if_stmt      → 'if' '(' expr ')' block
             | 'if' '(' expr ')' block 'else' block

while_stmt   → 'while' '(' expr ')' block

print_stmt   → 'print' expr ';'

block        → '{' stmt_list '}'              (* introduces a new scope *)

expr         → expr '||' expr
             | expr '&&' expr
             | '!' expr
             | expr '==' expr
             | expr '!=' expr
             | expr '<'  expr
             | expr '>'  expr
             | expr '<=' expr
             | expr '>=' expr
             | expr '+'  expr
             | expr '-'  expr
             | expr '*'  expr
             | expr '/'  expr
             | expr '%'  expr
             | '-' expr                        (* unary minus *)
             | '(' expr ')'
             | ID
             | INT_CONST
             | FLOAT_CONST
             | 'true'
             | 'false'
```

### 3.4 Operator Precedence and Associativity

Declared in `parser.y` from **lowest to highest** precedence:

| Level (low → high) | Operators | Associativity |
|---|---|---|
| 1 | `\|\|` | left |
| 2 | `&&` | left |
| 3 | `!` | right |
| 4 | `==` `!=` | left |
| 5 | `<` `>` `<=` `>=` | left |
| 6 | `+` `-` | left |
| 7 | `*` `/` `%` | left |
| 8 | unary `-` (`UMINUS`) | right |

This ordering matches conventional language semantics: `a || b && c` parses
as `a || (b && c)`, and `a + b * c` parses as `a + (b * c)`. Unary minus is
given the **highest** precedence via a synthetic `UMINUS` token and
`%prec UMINUS`, so that `-a + b` parses as `(-a) + b` rather than
`-(a + b)`.

### 3.5 Scoping Rules

Every `block` (`{ ... }`) introduces a new nested scope. A variable
declared inside a block is visible only within that block (and any nested
blocks within it) and ceases to exist once the block ends. The global
(top-level) statement list is itself scope level 0.

### 3.6 Example Program

```c
int x;
x = 5;
float y;
y = x + 2.5;

if (x > 0) {
    print x;
} else {
    print y;
}

int i;
i = 0;
while (i < 3) {
    print i;
    i = i + 1;
}
```
## 4. Compiler Architecture

Ankara follows a classical multi-phase pipeline. Each phase consumes the
output of the previous phase and produces input for the next; the phases
were also the project's milestones, developed and tested in this order.

```
 source.mc
     │
     ▼
 ┌─────────────┐   tokens    ┌─────────────┐   AST    ┌──────────────────┐
 │   Lexer     │────────────▶│   Parser    │─────────▶│  Semantic         │
 │  (lexer.l)  │             │ (parser.y)  │          │  Analyzer          │
 └─────────────┘             └─────────────┘          │ (+ Symbol Table)   │
                                                        └─────────┬─────────┘
                                                                  │ validated AST
                                                                  ▼
                                                        ┌────────────────────┐
                                                        │  TAC Generator      │
                                                        └────────────────────┘
                                                                  │
                                                                  ▼
                                                        Three Address Code
```

- **Lexer** (`src/lexer/lexer.l`): converts raw source text into a stream of
  tokens, reporting lexical errors (invalid characters) as they occur without
  stopping.
- **Parser** (`src/parser/parser.y`): consumes the token stream, validates it
  against the grammar in Section 3.3, and — as part of the same grammar
  actions — constructs an AST.
- **Symbol Table** (`src/symbol_table/`): a scoped table of declared
  variables, populated and queried during semantic analysis.
- **Semantic Analyzer** (`src/semantic/`): walks the AST, using the symbol
  table to check declarations, scoping, and types.
- **TAC Generator** (`src/codegen/`): walks the *validated* AST (semantic
  analysis must succeed first) and emits Three Address Code.

The driver (`main()` in `parser.y`) ties these together: it runs the parser,
and only if parsing succeeds does it run semantic analysis; only if
semantic analysis succeeds does it run TAC generation. This mirrors how
real compilers refuse to generate code for a program that isn't even
type-correct.

Unlike a typical Bison project, our lexer and parser are compiled directly
into a single executable, `build/parser_test`, via a Makefile rule
(`make parser-test`) rather than being tested through separate binaries per
phase — with one exception: an early standalone lexer test target
(`make lexer-test`) was used only in Milestone 2, before the lexer was
integrated with Bison, to verify tokenization output in isolation.

## 5. Lexer Design

### 5.1 Responsibilities

The lexer's job is narrowly scoped: recognize the lexical tokens defined
in Section 3.2, skip whitespace and comments, track the current line
number for error reporting, and report an error (without halting) whenever
it encounters a character that doesn't belong to any valid token.

### 5.2 Implementation

The lexer is a single Flex specification, `src/lexer/lexer.l`, compiled by
Flex into `lex.yy.for_parser.cpp`. Its rules are checked top-to-bottom
against the longest possible match at each position (Flex's standard
maximal-munch behavior), so, for example, `<=` is matched by the two-
character rule rather than being tokenized as `<` followed by `=`.

Key design points:

- **Token codes come from Bison, not the lexer.** Rather than defining its
  own token-type enum, the lexer `#include`s `parser.tab.h` — the header
  Bison generates from `parser.y` — and returns the token constants defined
  there (`INT`, `PLUS`, `ID`, ...). This means the lexer has a hard build
  dependency on the parser: `parser.tab.h` must exist before `lexer.l` can
  even compile. The Makefile encodes this dependency explicitly.
- **`yylval` carries token values.** For tokens with a value — integer
  constants, float constants, and identifiers — the lexer writes into the
  `yylval` union (whose layout is declared by Bison's `%union` block in
  `parser.y`) before returning the token code. For example, on matching an
  identifier, the lexer does `yylval.sval = strdup(yytext); return ID;`.
- **Line tracking.** A global `int line_number`, defined in `lexer.l` and
  declared `extern` in `parser.y`, is incremented on every newline and used
  by both lexical and syntax error messages.
- **Comments and whitespace are discarded silently** — matched by their own
  rules with empty actions, so they never reach the parser as tokens.
- **Lexical error handling.** The catch-all rule `.` matches any single
  character not matched by an earlier, more specific rule. It prints
  `Lexical Error at line N: Invalid character 'c'`, increments a
  `lexical_error_count` counter, and — critically — does **not** call
  `exit()` or `return`. This lets scanning continue past the bad character,
  so a single invalid character doesn't prevent the rest of the file from
  being tokenized and checked.

### 5.3 Design Rationale

We chose to integrate the lexer directly with Bison's generated token codes
(rather than keep the standalone `TokenType`/`Token` design used briefly in
Milestone 2) because maintaining two independent definitions of "what a
token is" — one in the lexer, one in the parser — creates an easy source of
bugs if they drift out of sync. Making Bison the single source of truth for
token identity means the lexer and parser can never disagree about what a
given token means.

## 6. Parser Design

### 6.1 Responsibilities

The parser validates a token stream against the grammar in Section 3.3,
resolves expression ambiguity via precedence and associativity, reports
syntax errors with line numbers, and — via semantic actions attached to
grammar rules — builds an AST that represents the parsed program's
structure independent of the concrete grammar used to derive it.

### 6.2 Implementation

The parser is a single Bison grammar, `src/parser/parser.y`, built with
`bison -d`, which produces both `parser.tab.cpp` (the generated parser) and
`parser.tab.h` (token codes and the `%union` type, needed by the lexer).

- **`%union`** declares the C++ types a token or non-terminal's semantic
  value can hold: `int ival`, `double fval`, `char* sval`, and (once AST
  construction was added in Milestone 4) an `ASTNode*` pointer type. This
  is what allows, e.g., `%token <ival> INT_CONST` to tie the `INT_CONST`
  token to the `ival` field of `yylval`.
- **AST construction via semantic actions.** Each grammar rule that
  produces a meaningful language construct has an action (`{ $$ = new
  ...Node(...); }`) that allocates the corresponding AST node, wiring child
  nodes in via the numbered `$1`, `$2`, ... references to the values
  matched by earlier symbols in the rule. For example, the rule for a
  `while` statement builds a `WhileNode` whose children are the condition
  expression and the body block, both already built as AST nodes by the
  time the `while_stmt` rule's action runs (Bison actions run bottom-up, so
  children are always constructed before their parent).
- **Blocks and scope, via a mid-rule action.** The `block` rule is:
  ```
  block: LBRACE { symTab.enterScope(); }
         stmt_list
         RBRACE  { symTab.exitScope(); $$ = new BlockNode($3); }
  ```
  The mid-rule action after `LBRACE` calls `symTab.enterScope()` *before*
  any statement inside the block is parsed, and `exitScope()` runs after
  the closing `RBRACE`. One consequence worth noting for anyone reading the
  grammar: a mid-rule action counts as a symbol in Bison's `$N` numbering,
  so `stmt_list` becomes `$3` in the closing action, not `$2` as it would
  be without the mid-rule action.
- **Error recovery.** The rule `stmt: ... | error SEMICOLON` uses Bison's
  special `error` token. When a statement fails to parse, Bison discards
  input tokens until it finds a `;`, then resumes parsing the next
  statement as if nothing happened, rather than aborting on the first
  syntax error. This satisfies the project manual's requirement for basic
  error recovery (Section 4.2) — a single malformed statement doesn't
  prevent the rest of the file from being checked.
- **`yyerror()`** is Bison's required error-reporting hook; our
  implementation prints `Syntax Error at line N: <message>` using the
  shared `line_number` global, and increments `syntax_error_count`.

### 6.3 Design Rationale

**Why build the AST during parsing rather than as a separate pass?** Bison
naturally builds results bottom-up as it reduces; piggy-backing AST
construction onto that process avoids a second full pass over the token
stream and is the idiomatic way to use a Bison-generated parser. The
grammar itself only validates *syntax* — no type checking or scope
checking happens in `parser.y` at all; those are deliberately deferred to
the Semantic Analyzer (Section 8), which operates on the completed AST
after parsing succeeds. This separation keeps the parser focused on one
job and made it possible to test syntax validation (Milestone 3) completely
independently of semantic correctness (Milestone 6).

**Why `error SEMICOLON` rather than more granular recovery?** Recovering at
statement boundaries (marked by `;`) is coarse but predictable: it maps
directly onto how a human reads source code (as a sequence of statements),
and it avoids the significantly more complex grammar engineering needed for
finer-grained recovery, which was outside the scope of what Section 4.2
of the manual required.
## 7. AST Structure

### 7.1 Purpose

The Abstract Syntax Tree abstracts away the concrete syntax (keywords,
punctuation, precedence-driven parse structure) and retains only the
program's essential structure — the information the Semantic Analyzer and
TAC Generator actually need. As the project manual specifies, the AST is
the shared data structure passed to both later phases; neither phase
touches raw tokens or grammar rules again once the parser has finished.

### 7.2 Node Hierarchy

All AST nodes derive from a common abstract base, `ASTNode` (defined in
`src/ast/ast.h`), which declares a virtual `print(indent)` method used for
debug tree-printing. Concrete node types:

| Node | Represents | Key fields |
|---|---|---|
| `ProgramNode` | The whole program | list of top-level statements |
| `BlockNode` | A `{ ... }` block | list of statements |
| `DeclNode` | A variable declaration | type, variable name |
| `AssignNode` | An assignment | variable name, RHS expression |
| `IfNode` | `if` / `if-else` | condition, then-block, optional else-block |
| `WhileNode` | `while` loop | condition, body block |
| `PrintNode` | `print` statement | expression to print |
| `BinaryOpNode` | Binary operator expression | operator, left, right |
| `UnaryOpNode` | Unary operator expression (`-`, `!`) | operator, operand |
| `IdNode` | Variable reference | variable name |
| `IntLiteralNode` | Integer literal | value |
| `FloatLiteralNode` | Float literal | value |
| `BoolLiteralNode` | Boolean literal | value |

An enum `VarType` (`TYPE_INT`, `TYPE_FLOAT`, `TYPE_BOOL`, and later
`TYPE_ERROR`, added in Milestone 6) is used throughout the AST, symbol
table, and semantic analyzer to represent a value's static type.

### 7.3 Construction and Ownership

Nodes are heap-allocated with raw pointers (`new ...Node(...)`) inside
Bison's grammar actions in `parser.y`, and wired together by their child
pointers (e.g., an `IfNode` holds pointers to its condition, then-block,
and else-block nodes). The parser's global `programRoot` (of static type
`ASTNode*`) is set to the root `ProgramNode` once parsing completes; `main()`
uses `dynamic_cast<ProgramNode*>(programRoot)` when passing the tree to the
Semantic Analyzer, since the analyzer's `analyze()` entry point expects the
concrete `ProgramNode*` type rather than the abstract base.

We use raw pointers rather than smart pointers for simplicity, given the
project's scope and the fact that the compiler is a short-lived,
single-pass process — allocated nodes live for the process's lifetime and
are not explicitly freed, which is an acceptable trade-off for a course
project of this size rather than a production compiler.

### 7.4 Example

For the input:
```c
if (x > 0) {
    print x;
}
```
the parser builds:
```
IfNode
├── condition: BinaryOpNode(>)
│   ├── left:  IdNode("x")
│   └── right: IntLiteralNode(0)
└── then-block: BlockNode
    └── PrintNode
        └── IdNode("x")
```
which is exactly what `ast_print.mc` was created to visually verify in
Milestone 4.

## 8. Symbol Table Design

### 8.1 Purpose

The symbol table tracks every declared variable's name, type, the scope it
was declared in, and the line it was declared on, and answers two
questions the Semantic Analyzer relies on constantly: *is this name
already declared in the current scope?* (for catching redeclaration) and
*is this name visible right now?* (for catching undeclared-variable use).

### 8.2 Implementation

`src/symbol_table/symbol_table.h` / `.cpp` implement:

- **`SymbolEntry`** — a plain struct: `name`, `type`, `scopeLevel`,
  `lineDeclared`.
- **`SymbolTable`** — internally, `std::vector<std::unordered_map<std::string,
  SymbolEntry>>`, i.e. a stack of hash maps, one map per currently-open
  scope. The vector's last element is always the *current* (innermost)
  scope.
  - **`enterScope()`** pushes a new, empty map onto the stack.
  - **`exitScope()`** pops the current (innermost) map off the stack,
    discarding every variable declared inside it.
  - **`insert(name, type, line)`** inserts into the *current* scope's map,
    returning `false` if the name already exists in that same map (i.e., a
    redeclaration within the same scope specifically — the same name in an
    *outer* scope is legal shadowing, not a redeclaration).
  - **`lookup(name)`** searches from the innermost scope outward, returning
    the first match found — implementing standard lexical scoping, where an
    inner declaration shadows an outer one of the same name.

### 8.3 Integration

`enterScope()` / `exitScope()` are called from the parser's `block` rule's
mid-rule action, described in Section 6.2, so the symbol table's scope
stack rises and falls in lockstep with the parser entering and leaving
`{ ... }` blocks. `insert()` is called from the `decl_stmt` rule. The
Semantic Analyzer (Section 9) later re-derives its *own* scope structure by
re-walking the AST with a fresh `SymbolTable` instance — it does not reuse
the parser's table directly, since semantic analysis is a distinct pass
that runs after parsing completes.

## 9. Semantic Analysis

### 9.1 Purpose

Semantic analysis catches errors that are syntactically well-formed but
meaningless or type-incorrect — the class of error a grammar alone cannot
detect. `src/semantic/semantic_analyzer.h` / `.cpp` implement a
`SemanticAnalyzer` class whose `analyze(ProgramNode*)` walks the entire AST
once, using its own `SymbolTable`, and returns `false` if any error was
found (with the count available via `errorCount()`).

### 9.2 The Six Checks

Per Section 4.5 of the manual, six categories of semantic error are
detected:

1. **Undeclared variable use** — an `IdNode` or assignment target whose
   name is not found by `symTab.lookup()`. Reported as
   `use of undeclared variable 'X'`.
2. **Redeclaration** — a `decl_stmt` whose `symTab.insert()` call returns
   `false` because the name already exists in the *current* scope.
   Reported as `redeclaration of variable 'X'`.
3. **Scope violation** — a variable used after the block that declared it
   has ended. Detected through the same mechanism as check 1: once
   `exitScope()` has popped a block's scope, a subsequent `lookup()` for a
   name declared only in that block fails exactly as if the variable had
   never been declared. (See Section 12, Challenges, for discussion of why
   this produces an identical message to check 1.)
4. **Type mismatch / invalid assignment** — an `AssignNode` whose RHS
   expression type does not match the declared type of the LHS variable.
   The one allowed implicit conversion is `int → float` widening; all other
   mismatches (e.g. `bool` assigned to `int`) are rejected. Reported as
   `type mismatch: cannot assign TYPE to variable 'X' of type TYPE`.
5. **Invalid expression** — operator/operand type mismatches within an
   expression: arithmetic operators (`+ - * / %`) require numeric
   (`int`/`float`) operands; logical operators (`&& || !`) require `bool`
   operands; equality operators (`== !=`) require both operands to be the
   same type category. Reported with an operator-specific message, e.g.
   `logical operator '&&' requires bool operands`.
6. **General invalid assignment** — overlaps with check 4; both are
   implemented by the same `resolveExprType()` / assignment-checking logic
   in `visitAssign()`.

### 9.3 Implementation Notes

- **`resolveExprType(ASTNode*)`** is the workhorse: given any expression
  subtree, it recursively determines (and returns) that subtree's
  `VarType`, checking operand types as it goes and reporting an error the
  moment it finds an invalid combination.
- **`TYPE_ERROR` as a sentinel.** When `resolveExprType()` detects an
  invalid expression, it reports the error once and returns `TYPE_ERROR`
  rather than a real type. Any *outer* expression that uses this result as
  an operand sees `TYPE_ERROR` and — by design — does not report a second,
  cascading error about the same root problem. This keeps error output
  focused on the actual mistake rather than a flood of downstream noise.
- **`reportError(line, message)`** centralizes the `Semantic Error at line
  N: message` output format and increments the analyzer's internal error
  counter, which `errorCount()` exposes.
- The analyzer visits statements via a family of `visitStmt` / `visitBlock`
  / `visitDecl` / `visitAssign` / `visitIf` / `visitWhile` / `visitPrint`
  methods that mirror the AST node hierarchy from Section 7.2, dispatching
  on the concrete node type.

### 9.4 Integration

`main()` in `parser.y` runs the Semantic Analyzer only after parsing
completes with zero syntax errors:
```c
SemanticAnalyzer analyzer;
bool semanticOk = analyzer.analyze(dynamic_cast<ProgramNode*>(programRoot));
if (!semanticOk) {
    // report analyzer.errorCount() and stop — TAC generation is skipped
    return 1;
}
```
This gating — no TAC without semantic correctness — mirrors how a real
compiler refuses to generate code for a program it cannot prove
type-correct.
## 10. Intermediate Code (TAC) Strategy

### 10.1 Purpose

Three Address Code is a linear, low-level intermediate representation
where every instruction has at most one operator and at most three
operands (hence the name) — a form much closer to machine code than the
tree-shaped AST, and a standard target for the "code generation" phase of
a teaching compiler.

### 10.2 Implementation

`src/codegen/tac_generator.h` / `.cpp` implement a `TACGenerator` class.
`generate(ASTNode* root)` returns a `vector<string>`, each string one TAC
instruction. Two counters drive fresh-name generation:

- **`newTemp()`** — returns a new temporary name (`t1`, `t2`, ...) for
  holding intermediate expression results.
- **`newLabel()`** — returns a new label name (`L1`, `L2`, ...) for branch
  targets.

Generation proceeds by walking the (already semantically-validated) AST:

| AST node | TAC emitted |
|---|---|
| `DeclNode` | nothing (declarations have no runtime effect in TAC) |
| `AssignNode` | `name = result` (where `result` is the temp/value the RHS expression evaluated to) |
| `BinaryOpNode` | evaluates both operands, then `tN = left op right` |
| `UnaryOpNode` | evaluates the operand, then `tN = op operand` |
| `IdNode` / literals | used directly by name/value — no instruction needed |
| `PrintNode` | `print result` |
| `IfNode` (no else) | condition eval, `if_false cond goto L1`, then-block, `L1:` |
| `IfNode` (with else) | condition eval, `if_false cond goto L1`, then-block, `goto L2`, `L1:`, else-block, `L2:` |
| `WhileNode` | `L_start:`, condition eval, `if_false cond goto L_end`, body, `goto L_start`, `L_end:` |

### 10.3 Example

For:
```c
int a; a = 3;
int b; b = 2;
int c; c = a + b * 2;
```
the generator correctly respects operator precedence at the AST level
(the AST already has `*` bound tighter than `+`, from parsing — TAC
generation just linearizes it) and emits:
```
a = 3
b = 2
t1 = b * 2
t2 = a + t1
c = t2
```

### 10.4 Design Rationale and Simplifications

- **No short-circuit evaluation.** For `&&` and `||`, our TAC generator
  always evaluates both operands unconditionally, rather than generating
  the branch-based code a production compiler would use to skip evaluating
  the right operand when the left already determines the result. This
  keeps the generator's control-flow logic simpler and was judged an
  acceptable simplification for this project's scope, since it does not
  affect correctness for any of our test programs (none rely on
  short-circuit side effects). This is documented further as a known
  limitation in Section 12.
- **TAC generation only runs after semantic analysis succeeds.** Since the
  generator assumes a type-correct AST (it never itself checks types), this
  ordering is not optional — generating TAC for an already-invalid program
  would be meaningless.

## 11. Testing Summary

Testing was treated as its own milestone (Milestone 8) rather than an
afterthought, following a *documented-and-verified* rather than
*assumed-correct* approach: every test file's expected output in
`tests/README.md` was captured from an actual `make parser-test` run, not
predicted in advance and left unchecked.

Coverage spans, per Section 15 of the manual:

- **Valid compilation** (`valid_basic.mc`, `tac_test.mc`) — full pipeline
  success, including correct AST printing and correct TAC output with
  proper precedence and branch labeling.
- **Lexical errors** (`lexical_error.mc`) — an invalid character is
  reported with a line number, and scanning continues afterward.
- **Syntax errors** (`syntax_error.mc`) — a malformed statement is
  reported with a line number, and the `error SEMICOLON` recovery rule
  lets parsing continue.
- **All six semantic error categories** from Section 9.2, each with its own
  dedicated test file and a captured, exact expected error message.
- **Supporting/demonstration files** (`ast_print.mc`, `scope_test.mc`) used
  during development (Milestones 4 and 5) to visually verify the AST and
  symbol table before the Semantic Analyzer existed to check them
  automatically.

A gap analysis conducted at the start of Milestone 8 found that three of
the six semantic sub-rules from Section 4.5 (scope violation, invalid
assignment, invalid expression) had no dedicated test file yet, despite
being implemented — a distinction between "logic exists" and "logic is
demonstrated," which the new tests were written specifically to close.
Full details for every test file live in `tests/README.md`.

## 12. Challenges

Building each phase independently surfaced problems whose root causes were
often only visible once two previously-separate pieces were connected —
consistent with how compiler bugs tend to surface at phase boundaries.
Below are the most significant ones, together with two design limitations
we chose to accept and document rather than "fix" in ways that would have
added disproportionate complexity for this project's scope.

### 12.1 Build and Toolchain Issues

- **`bison -d` header naming.** By default, `bison -d` on a `.cpp` output
  file generates `parser.tab.hpp`, not `parser.tab.h` — but our lexer
  `#include`s the latter. Fixed by passing `--defines=build/parser.tab.h`
  explicitly in the Makefile rather than relying on Bison's default.
- **Duplicate global definitions.** Both `lexer.l` and `parser.y` declared
  `int line_number` and `int lexical_error_count` without either being
  marked `extern`, so the linker saw two separate definitions of each and
  failed with "multiple definition" errors. Fixed by making the copies in
  `parser.y` `extern` declarations, leaving `lexer.l` as the single place
  that actually allocates the variables.
- **Missing `main()`.** Early in Milestone 3, `parser.y` had no `main()`
  function at all, producing a linker error about a missing `WinMain` entry
  point on Windows (MinGW's runtime expects `main` to exist even though the
  error message references `WinMain`). Fixed by adding a proper `main()`
  that opens the input file, calls `yyparse()`, and reports the error
  counts.
- **POSIX extensions on Windows/MSYS2.** `strdup()` and `fileno()` (used
  internally by Flex's generated buffer-handling code) are POSIX
  extensions not visible under strict `-std=c++17` on some MSYS2/MinGW
  toolchain versions. Depending on the machine, this required either
  switching to `-std=gnu++17` or otherwise ensuring GNU extensions were
  enabled — a good example of a portability issue specific to the
  Windows+MSYS2 development environment, not the code's logic.
- **UTF-8 in terminal output.** An em dash (`—`) character typed directly
  into a `printf` string in `parser.y` displayed as garbled characters in
  some Windows terminals, because the terminal's encoding didn't match the
  source file's UTF-8 encoding. Fixed by using a plain ASCII hyphen
  instead — a reminder to prefer ASCII in code that must run identically
  across different teammates' terminal configurations.

### 12.2 Grammar and Semantic-Logic Issues

- **Mid-rule action shifts `$N` numbering.** Adding the mid-rule action
  `{ symTab.enterScope(); }` to the `block` rule (Section 6.2) meant the
  following `stmt_list` symbol became `$3`, not `$2`, in the rule's final
  action — a subtlety of how Bison numbers mid-rule actions as their own
  pseudo-symbol that is easy to get wrong on a first attempt.
- **Signature mismatches after refactoring.** When adding the Semantic
  Analyzer (Milestone 6) and again when adding the TAC Generator
  (Milestone 7), `main()` in `parser.y` was initially written against an
  outdated function signature (mismatched parameter and return types),
  producing compile errors that were only resolved by re-checking the
  actual header file rather than assuming its shape from memory — a
  reminder that cross-file consistency has to be actively verified, not
  assumed, especially in a multi-person project.

### 12.3 Accepted Design Limitations

Two limitations remain in the final implementation. Both are understood
design trade-offs, not bugs we failed to find:

1. **Scope violation and undeclared-variable errors are
   indistinguishable.** Both check 1 and check 3 from Section 9.2 are
   detected via the same `symTab.lookup()` failure. Because `exitScope()`
   fully discards a block's variables when the block ends, a lookup after
   that point cannot tell the difference between "this variable was never
   declared anywhere" and "this variable was declared, but only inside a
   now-closed scope." Distinguishing them would require the symbol table
   to retain a history of closed scopes (or at least their variable names)
   purely for more precise error messages — additional bookkeeping we
   judged not worth the complexity for a project of this scope, given that
   both cases are still correctly *rejected*, just with the same wording.

2. **Some semantic errors report line 0.** `BinaryOpNode` and `UnaryOpNode`
   do not currently store the source line they came from (unlike
   statement-level nodes, which do). When `resolveExprType()` detects an
   invalid-expression error (check 5, e.g. `&&` used on a non-`bool`
   operand) purely from within an expression subtree, it has no line number
   to report and falls back to `0`. The error is still correctly *detected*
   — only the reported line number is affected. A complete fix would mean
   threading a line number through every expression-level AST node
   constructor and every `parser.y` action that builds one; we chose to
   document this rather than implement it, given the limited time
   available and that it does not affect detection correctness, only
   diagnostic precision.

Both limitations are also noted in the root `README.md` and in
`tests/README.md` next to the specific test files that exhibit them
(`semantic_scope_violation.mc` and `semantic_invalid_expression.mc`
respectively), so anyone reading the test output understands these are
expected, explained behaviors.

## 13. Conclusion

Ankara implements a complete, working front-end-through-intermediate-code
compiler pipeline for a small procedural language: lexing, parsing with
AST construction, scoped symbol table management, six categories of
semantic checking, and Three Address Code generation, each independently
tested and documented. Developing the project milestone-by-milestone — 
verifying each phase in isolation before connecting it to the next —
proved to be the single most useful practice for managing complexity
across a three-person team working across multiple machines and multiple
git identities; nearly every serious bug we hit (linker errors, mid-rule
`$N` numbering, signature drift) surfaced specifically *at* a phase
boundary, right where two previously-independent, previously-working
pieces were joined for the first time.

The two accepted limitations documented in Section 12.3 — rather than
being hidden or left unexplained — are, we think, evidence of genuinely
understanding the codebase's behavior well enough to know precisely where
and why it falls short of a production compiler, which is the standard the
manual's Section 10 AI-usage policy holds every submission to.

## 14. References

1. Compiler Construction Lab Project Manual, Metropolitan University
   Bangladesh (course-provided specification for language grammar,
   required modules, and milestone deliverables).
2. Flex — The Fast Lexical Analyzer, GNU Project.
3. Bison — GNU Parser Generator, GNU Project.
4. Aho, A. V., Lam, M. S., Sethi, R., Ullman, J. D. — *Compilers:
   Principles, Techniques, and Tools* (2nd ed.) — standard reference for
   the multi-phase compiler architecture, AST design, and Three Address
   Code, on which the general structure of this project is based.
