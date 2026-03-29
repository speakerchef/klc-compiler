# KLC: KNOB-Lang-Compiler

A from scratch compiler for **KNOB** (**K**ompiled **NOB**) — a statically typed, semicolon delimited language that I'm creating that compiles down to AArch64 assembly.

> *Why `.knv` and not `.knb`?*
> Everyone knows the true perfect language prioritizes ergonomics over sensible standards. `v` is easier to hit from the home row than `b`. You're welcome.

---

## Architecture

```
source.knv → Lexer → Pratt Parser → AST → Codegen → AArch64 ASSEMBLY → clang/ld → executable
```

- **Lexer/Tokenizer** — tokenizes `.knv` source into a stream of typed tokens
- **Parser** — Pratt parser with precedence climbing for expressions, producing an AST.
- **Codegen** — Currently direct AST emission to assembly targeting AArch64 (Apple Silicon / macOS Darwin ABI). (x86_64 support in the future).

No LLVM IR or other backends/deps.

---

## Language features

*KNOB is a fun project of mine and is still a work in progress.

### Keywords (so far)

| Keyword | What it does |
|---------|-------------|
| `let`   | Const-defaulted variable declaration |
| `mut`   | Mutable variable declaration |
| `exit`  | Exit with an exit code |
| `if`    | Conditional branching (WIP) |

### Supported Operations (so far)

- Integer arithmetic: `+`, `-`, `*`, `/` (with correct precedence)
- Parenthesization: `(1 + 2) * (3 + 4)`
- Variable declarations with expression assignment

### Example syntax for `.knv`. Try to run this!

```
let x = 5 * 6 / 2 - 5; // 10
let y = (1 + 2) * (3 + 4); // 21
exit y - x; //you should see 11
```

---

## Build & Run

> **Requires:** CMake, Clang/GCC with C++23 support, AArch64 target (Apple Silicon Mac)

```bash
# Build the compiler
cmake -S . -B build/
cmake --build ./build

# Compile a .knv file to assembly
./build/klc <FILE.knv>
```

### Assemble & Link the output

```bash
cd build/
clang -c -g -o a.o gen_asm.s \
  && ld -lSystem -syslibroot $(xcrun -sdk macosx --show-sdk-path) -e _main -o out a.o \
  && ./out

# Check exit code
echo $?
```

---
