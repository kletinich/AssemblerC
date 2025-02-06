# Assembler
This project simulates an asembler. Given an assembly-like language, the code translates it to machine code.
The relevant file type for the assemble is **.as**, other file types won't assemble.

## The assembler includes the reserverd words: ##
### 1. Macros: ###

| Reserved Word | Description                                       | Example Usage                |
|---------------|---------------------------------------------------|------------------------------|
| `macr`      | Declares the beginning of a macro definition. This keyword indicates that the following lines define the macro's content until `endmacr` is encountered. | `macr MY_MACRO`              |
| `endmacr`   | Marks the end of a macro definition. This keyword signals the conclusion of the macro content that was defined by `macr`.   | `endmacr`                    |

### 2. Commands: ###

  | Command Number | Command | Description                       |
|----------------|---------|-----------------------------------|
| 0              | `mov`     | Move data from one location to another. |
| 1              | `cmp`     | Compare two values.              |
| 2              | `add`     | Add two values together.          |
| 3              | `sub`     | Subtract one value from another.  |
| 4              | `lea`     | Load the effective address (useful for pointers). |
| 5              | `clr`     | Clear a register or memory location (set it to zero). |
| 6              | `not`     | Perform a logical NOT operation (invert bits). |
| 7              | `inc`     | Increment a value by one.        |
| 8              | `dec`     | Decrement a value by one.        |
| 9              | `jmp`     | Jump to a specified address (unconditional jump). |
| 10             | `bne`     | Branch if not equal (conditional jump). |
| 11             | `red`     | Read input (e.g., from user).   |
| 12             | `prn`     | Print output (display a value).  |
| 13             | `jsr`     | Jump to subroutine (call a function). |
| 14             | `rts`     | Return from subroutine.          |
| 15             | `stop`    | Halt execution of the program.   |

### 3. Labels: ###
| Label  | Description                                                      | Example                       |
|-------------|------------------------------------------------------------------|-------------------------------|
| `.data`     | Defines a data segment for variables.                           | `.data`<br>`variable: .word 5` |
| `.string`   | Defines a null-terminated string.                               | `.string "Hello, World!"`     |
| `.entry`    | Marks the entry point of the program or function.              | `.entry main`                 |
| `.extern`   | Declares variables or functions defined in other files/modules. | `.extern externalVar`         |

### 4. Registers: ###
| Register | `r0` | `r1` | `r2` | `r3` | `r4` | `r5` | `r6` | `r7` |
|----------|------|------|------|------|------|------|------|------|


## How to run: ##
1. run makefile with the given c and h files.
2. include the assembly test files in the same folder for testing.

If the assemble procedure is successfull, [file name].ob of binary data, [file name].ent of entries, [file name].ext of externals.
If the assemble procedure isn't successfull, no file will be created.
