# Assembler

A two-pass assembler for a custom 24-bit instruction set architecture, written in ANSI C.

![C](https://img.shields.io/badge/C-90-blue.svg)
![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)

## Overview

This assembler translates assembly source files (`.as`) into machine code through a multi-stage pipeline: preprocessing, first pass (symbol resolution), and second pass (code generation). It features a macro system, comprehensive error reporting, and outputs object files with optional entry/external symbol tables.

## Features

- **Macro Preprocessor**
  - Define reusable code blocks with `mcro`/`mcroend`
  - Automatic macro expansion
  - Comment and whitespace normalization
  - Generates intermediate representation (`.am` files)

- **Two-Pass Assembly**
  - **First Pass**: Builds symbol table, calculates addresses (IC/DC)
  - **Second Pass**: Resolves symbols, encodes instructions to machine code

- **Instruction Set**
  - 16 instructions: `mov`, `cmp`, `add`, `sub`, `lea`, `clr`, `not`, `inc`, `dec`, `jmp`, `bne`, `jsr`, `red`, `prn`, `rts`, `stop`
  - 8 general-purpose registers (`r0`-`r7`)
  - 4 addressing modes: immediate, direct, relative, register

- **Directives**
  - `.data` – Define numeric data
  - `.string` – Define string literals
  - `.entry` – Export symbol
  - `.extern` – Import external symbol

- **Error Handling**
  - 30+ distinct error types with file/line information
  - Continues processing to report multiple errors
  - Clear, descriptive error messages

## Building

```bash
gcc -ansi -Wall -pedantic -o assembler \
    main/assembler.c \
    assembly/preprocessor.c \
    assembly/first_pass.c \
    assembly/second_pass.c \
    common/*.c \
    data_structures/*.c
```

Or with Make:

```bash
make
```

## Usage

```bash
./assembler <file1> [file2] [file3] ...
```

File extensions are added automatically – provide base names only:

```bash
./assembler program1 program2
# Processes program1.as and program2.as
```

## Output Files

| Extension | Description |
|-----------|-------------|
| `.am` | Preprocessed intermediate representation |
| `.ob` | Object file (machine code) |
| `.ent` | Entry symbols table (if `.entry` used) |
| `.ext` | External symbols table (if `.extern` used) |

### Object File Format

```
     IC DC
ADDRESS HEXCODE
ADDRESS HEXCODE
...
```

Example:
```
     7 3
0000100 040540
0000101 000003
0000102 040501
```

## Assembly Syntax

### Instructions

```asm
; Two-operand instructions
mov #5, r1          ; immediate to register
add r1, r2          ; register to register
cmp DATA, #10       ; direct to immediate

; One-operand instructions  
inc r3              ; increment register
jmp LOOP            ; jump to label
prn #-5             ; print immediate

; Zero-operand instructions
rts                 ; return from subroutine
stop                ; halt execution
```

### Labels and Directives

```asm
; Label definition
LOOP:   mov r1, r2

; Data directives
DATA:   .data 100, -5, 0x1F
STR:    .string "Hello"

; Symbol visibility
.entry  LOOP        ; export LOOP
.extern HELPER      ; import HELPER
```

### Macros

```asm
mcro print_r1
    prn r1
mcroend

; Usage - expands inline
print_r1
```

## Architecture

### Instruction Word Format (24-bit)

```
| ARE (3) | Funct (5) | Rd (3) | Rd Addr (2) | Rs (3) | Rs Addr (2) | Opcode (6) |
```

### Addressing Modes

| Mode | Code | Example | Description |
|------|------|---------|-------------|
| Immediate | 0 | `#5` | Literal value |
| Direct | 1 | `LABEL` | Memory address |
| Relative | 2 | `&LABEL` | PC-relative offset |
| Register | 3 | `r0` | Register direct |

## Project Structure

```
assembler/
├── main/
│   └── assembler.c         # Entry point, context management
├── assembly/
│   ├── preprocessor.c      # Macro expansion, comment removal
│   ├── first_pass.c        # Symbol table construction
│   └── second_pass.c       # Code generation
├── common/
│   ├── lexer.c             # Tokenization
│   ├── parser.c            # Syntax analysis
│   ├── code_gen.c          # Machine code encoding
│   ├── isa.c               # Instruction set definitions
│   ├── error.c             # Error reporting system
│   ├── file_io.c           # File operations
│   ├── string_view.c       # Non-owning string utilities
│   └── util.c              # Memory management wrappers
├── data_structures/
│   ├── array_list.c        # Dynamic array
│   └── hash_map.c          # Symbol table (separate chaining)
└── Makefile
```

## Error Examples

```
[Label Duplicate] test.am:15: Label 'LOOP' already defined
[Syntax Addressing Mode] test.am:22: Invalid addressing mode for instruction 'lea'
[Immediate Out of Bounds] test.am:30: Immediate value 2097152 exceeds allowed range
[Symbol Not Found] test.am:45: Symbol 'UNDEFINED' not found in symbol table
```

## Technical Notes

- Written in ANSI C (C90) for maximum portability
- No external dependencies beyond standard library
- Custom memory wrappers with allocation tracking
- StringView implementation for zero-copy parsing
- Hash map uses djb2 algorithm with separate chaining

## Limitations

- Maximum line length: 80 characters
- Maximum label length: 31 characters
- 21-bit signed immediate range: -1,048,576 to 1,048,575
- 24-bit address space

## License

MIT License – see [LICENSE](LICENSE) for details.
