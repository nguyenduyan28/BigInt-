# Infinite Signed Integer Calculator

A C++17 terminal calculator that evaluates one binary expression per line using a custom arbitrary-size signed integer (`BigInt`). No external big-integer libraries are used.

## Build

```bash
cmake -S . -B build
cmake --build build
```

Targets:
- `calc`: CLI calculator
- `parser_tests`: lexer/parser only (no BigInt)
- `bigint_tests`: BigInt core only (no parser)

## Run the calculator

```bash
./build/calc
```

Enter one expression per line, then press Enter. Examples:
```
123 + 456
-5 * 10
100000000000000004 % 100000000000000000
```

## Input format and operators

- Format: `<INT> <OP> <INT>` on a single line.
- `INT`: optional leading `+`/`-`, digits only, leading zeros allowed; spaces permitted between tokens.
- Supported operators: `+`, `-`, `*`, `%` (mod is included).

## Error handling

- Parse errors: `parse error: <message>` to stderr (with position info) and continue.
- Number parsing errors: `number error: <message>` to stderr and continue.
- Math errors (e.g., mod by zero): `math error: <message>` to stderr and continue.

## Modulo semantics

`%` follows C/C++ remainder rules: the result has the same sign as the dividend. Modulo by zero raises a math error.

## Architecture (frontend vs core)

- Frontend: lexer and parser produce `ParsedExpr { lhs string, op char, rhs string }` using `std::string_view` tokens, independent of BigInt.
- Core: BigInt parses individual integers, performs arithmetic, and prints results.
- Evaluator (CLI) glues them: parses, constructs BigInts from lexemes, applies the operator, and prints the outcome.

## Tests

From the build directory:
```bash
./parser_tests
./bigint_tests
```
