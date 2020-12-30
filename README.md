# Generic Compiler Scaffolding

This repository is the scaffolding for an end-to-end compiler. It is a useful
starting point when developing a new language — taking care of the workflow
and wiring typical in most of my personal programming language projects.

To build, check out the repository and execute the following commands:

	$ git submodule init
	$ git submodule update
	$ cmake -H. -Bbuild
    $ cmake --build build --target compiler
    $ ./build/compiler

We include [LLVM](https://llvm.org) as a submodule.

## Features and Dependencies

The compiler is split into four primary directories representing the logical
parts of the compiler workflow:

  1. `parser/` - A parser based on [Bison](https://www.gnu.org/software/bison/) and [Flex](https://www.gnu.org/software/flex/).
  2. `checker/` - Placeholder module to check the program for semantic correctness.
  3. `emitter/` - Backend code generation to [LLVM IR](https://llvm.org/docs/LangRef.html).
  4. `commands/` - A lightweight framework for supporting different compiler commands. Out of the box, the compiler supports the following commands:
     - `compiler run` - Execute a program using just-in-time compilation
     - `compiler build` - Generates a binary (optionally cross-compiling for different architectures)
     - `compiler check` - Checks a program for semantic correctness
     - `compiler parse` - Checks a program for syntactic correctness
     - `compiler ir` - Emits the LLVM IR code for a program 

## Starting Point

The project builds a compiler for a minimal languge that prints the results of
simple integer expressions, e.g.,

    1 * 3 + (12 % 5)
    (2 << 3) / 2

prints

    5
    8

## Author

This scaffolding is used exclusively for personal projects by Bret Taylor
([btaylor@gmail.com](btaylor@gmail.com)).
