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

KNOB is a fun project of mine still under active construction.

### Keywords (so far)

| Keyword | What it does |
|---------|-------------|
| `let`   | Const-defaulted variable declaration |
| `mut`   | Mutable variable declaration |
| `exit`  | Exit with an exit code |
| `if`    | Conditional entry |
| `elif`  | Alternate branch |
| `else`  | Fallback branch |

### Operators (so far)

| Category | Operators | Notes |
|----------|-----------|-------|
| Arithmetic | `+` `-` `*` `/` | Standard integer arithmetic (fp later)|
| Power | `**` | Right associative, eg. 2**3 = 8 |
| Comparison | `==` `!=` `<` `>` `<=` `>=` | 1 or 0 |
| Logical | `&&` `\|\|` | Boolean logic on truthy/falsy values |
| Bitwise | `&` `\|` `^` | AND, OR, XOR |

### Other Features

- Parenthesized expressions with correct grouping: `(a + b) * (c - d)`
- Scoping with nested blocks and proper variable resolution
- Variables from outer scopes are visible in inner scopes
- Local variables are inaccessible outside their scope
- Nested if/elif/else with arbitrary depth

### Example syntax for `.knv`. Try to run this! `echo $?` should give you 13 :D

```
let a = 2 ** 4;
let b = (a - 1) * 3;
let c = b / 9;
let d = a ^ c;
let e = (d & 7) | 4;
let f = a > c;
let g = c == 5;
if (f && g) {
    let h = (a + b + c) / 4;
    if ((h > 10) && (e != 0)) {
        let k = a - c;
        let l = k ** 2;
        if (l > 100) {
            let m = (l / 11) ^ (e + 1);
            let n = m & 15;
            let o = n > 0;
            let p = l != 121;
            if (o && p) {
                exit n;
            } else {
                exit m;
            }
        }
        exit l;
    }
    exit h;
}
exit 0;
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

## Roadmap

- [ ] For/While loops
- [ ] Variable reassignment
- [ ] Functions with
- [ ] String literals + syscall `write`
- [ ] Loop optimizations
- [ ] Register allocation pass
- [ ] x86_64 backend
- [ ] ...and many more!
