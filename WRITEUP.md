# Infinite Signed Integer Calculator — Detailed Task Writeup

## 1) Code Structure 
- **Frontend module:** `src/parser/Lexer.{h,cpp}` and `src/parser/Parser.{h,cpp}`. Turns raw text into tokens, enforces the strict grammar `<INT> <OP> <INT> END`, and produces a `ParsedExpr` with two lexeme strings and an operator char. No numeric code is referenced here.
- **Core module:** `src/bigint/BigInt.{h,cpp}`. Implements the arbitrary-size signed integer with its own parsing of a single number, arithmetic, and formatting. It never includes parser headers.
- **Glue (CLI):** `src/cli/main.cpp` wires frontend to core; kept minimal and omitted from detail.
- **Build:** `CMakeLists.txt` produces static libs (`parser`, `bigint`) and executables (`calc`, `parser_tests`, `bigint_tests`) to keep layering explicit.

The detailed sections below focus on Task 1 (Frontend) and Task 2 (BigInt), in that order, with an emphasis on techniques, data structures, and algorithms used in the code.

## Task 1: Frontend (Lexer + Parser)
### Approach
Model the frontend like a small compiler front-end: a lexer that emits lightweight tokens with positions, and a parser that consumes exactly four tokens according to a fixed grammar. The parser is responsible for precise, position-rich errors and returns raw lexeme strings so later layers can decide how to interpret numbers.

### Lexer techniques (tokenization without allocations)
- **State:** Holds `std::string_view input_` and `size_t pos_`. Tokens are represented by the `Token` struct (`TokenType type`, `string_view lexeme`, `size_t position`, `char op`, `std::string error`).
- **Whitespace skipping:** `skipSpaces()` advances `pos_` using `std::isspace` to ignore arbitrary spaces between tokens without allocating or copying.
- **Integer recognition rule:** Start an integer if the current char is a digit, or if it is `+`/`-` **and** the next char is a digit (prevents lone signs from masquerading as numbers). Once started, advance while digits continue. The lexeme slice is a `string_view` into the original buffer—no copies during lexing.
- **Operators and invalids:** Recognizes single-character operators `+`, `-`, `*`, `%`. Any other character produces `TokenType::Invalid` with `error="invalid character"` and the offending slice/position preserved.
- **End-of-input token:** When `pos_` reaches `input_.size()`, emit `TokenType::End` carrying the current index so the parser can distinguish a true end from premature termination.
- **Why this design:** `string_view` minimizes allocations; storing positions at lex time enables precise parser diagnostics; the sign-followed-by-digit rule prevents accidental acceptance of stray signs.

### Parser techniques (fixed grammar, position-aware errors)
- **Grammar shape:** Exactly: `INT`, `OP`, `INT`, then `END`. No recursion or precedence—linear, predictable control flow that is easy to audit.
- **Error reporting:** Uses `unexpectedMessage` to tailor messages:
  - For `Invalid` tokens: forwards lexer’s `error` with `"at pos N"`.
  - For early `End`: `"unexpected end of input at pos N"`.
  - For wrong token kinds: `"expected <kind> at pos N"`.
  Tests assert the presence of `pos N`, so positions are tracked meticulously.
- **Success path:** Only after all four checks pass does it materialize `ParsedExpr` by copying `lhs` and `rhs` lexemes into `std::string` and storing the operator char. Copying is deferred to avoid allocations on failed parses.
- **Separation from BigInt:** Parser includes only `Lexer.h` and standard headers. It never constructs BigInt, keeping the module purely about syntax.
- **Why this design:** Fixed arity simplifies reasoning and testing; position-rich errors aid debugging; deferring string copies balances efficiency with clarity.

## Task 2: Core BigInt
### Approach
Represent integers as a sign plus base-`1e9` limbs in little-endian order. Separate magnitude math from sign handling, normalize once per operation to enforce invariants, and keep parsing/printing of a single integer self-contained (distinct from expression parsing).

### Representation and invariants
- **Data structures:** `int sign_` in `{-1,0,+1}`; `std::vector<uint32_t> limbs_` where each limb stores 9 decimal digits (`BASE = 1,000,000,000`), least significant limb at index 0.
- **Normalization:** `normalize()` strips trailing zero limbs and sets `sign_` to zero if the vector empties. Invariants: zero ⇒ empty limbs and `sign_==0`; non-zero ⇒ top limb non-zero; negative zero is impossible.
- **Why base 1e9:** Fits neatly in 32 bits; products of two limbs plus carry fit in `uint64_t`; reduces limb count versus base 10 but keeps conversion simple.

### Parsing and printing a single integer
- **fromString(std::string_view):** Trims outer spaces, reads optional leading `+`/`-`, then requires all remaining chars to be digits. Leading zeros collapse to canonical zero. Digits are chunked from the right in 9-digit groups (`std::stoul`) and appended as limbs. Any non-digit throws `std::invalid_argument`.
- **toString():** If zero, return `"0"`. Otherwise, prepend `'-'` when `sign_<0`, print the most significant limb without padding, then each lower limb zero-padded to width 9 to preserve interior zeros.

### Magnitude helpers and signed orchestration
- **Comparison:** `cmpAbs` compares limb vectors by size then lexicographically from most significant limb; `cmp` wraps sign logic around it.
- **Addition/Subtraction:** 
  - `addAbs` iterates over `max(len(a), len(b))` with `uint64_t carry`, appending a final carry if needed.
  - `subAbs` assumes `|a| >= |b|` and uses `int64_t diff` plus a borrow flag, adding `BASE` when negative.
  - Signed `operator+`/`operator-` decide between `addAbs` and `subAbs` based on operand signs and `cmpAbs`. Equal magnitudes short-circuit to zero. This keeps inner loops free from sign branching.
- **Multiplication (schoolbook):** Pre-sizes the result to `a.size + b.size + 1`, then for each limb pair accumulates `uint64_t cur = existing + a[i]*b[j] + carry`, writing `cur % BASE` back and carrying `cur / BASE`. After each row, leftover carry ripples forward. A single `normalize()` trims over-allocation. Complexity `O(n*m)` fits the <100-digit target.
- **Modulo (absolute long division with binary search per digit):**
  - Operate on absolute values; throw `runtime_error` on divisor zero. If `|a| < |b|`, return `a`.
  - Shift the divisor left in base-`BASE` units by inserting leading zeros to match the dividend length.
  - For each shift position (most to least significant), binary-search a scalar `k` in `[0, BASE-1]` such that `shifted_divisor * k <= remainder`. Scalar multiply uses `mulAbsByUint` to avoid duplicating loops. Subtract the best product, then shift the divisor right by erasing one leading zero, and continue.
  - After absolute remainder is found, if the original dividend was negative and remainder non-zero, set `sign_` to `-1` to match C/C++ remainder semantics (remainder keeps dividend’s sign). Finish with `normalize()`.
  - **Why this method:** Clear to read, leverages existing helpers, and is efficient enough for the problem scale without the complexity of full Knuth division.
