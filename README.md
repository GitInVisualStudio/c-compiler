# c-compiler
Yet another c compiler written in c, becuase why not.

Sub-compiles to x86-64 Assembly.

### Supported features:
  - local ```int``` variables
  - logical expressions ```!(1 && 3 || 0)```
  - arithmetic expressions ```5 + (3 - 2) * 8 / 2```
  - ```if``` statements
  - ```for``` & ```while``` loops
  - assignments ```a += b * 2```
  - functions with parameter
 
 ### Known issues:
  - Re-declaration of variables in a new scope is not possible. I just think it's a stupid idea, that's why.
  - Multiple definitions of functions won't be detected. Same as before.
  - Functions only support up to **6** parameter, that's because I am lazy and don't want to implement the full x86-64 System V function calling conventions. :)

# Setup
```bash
   git clone https://github.com/GitInVisualStudio/c-compiler.git
   cd ./c-compiler
   make run
```

This will automatically setup all the directories. 

The default example code that is compiled is in ```../code/```, but you can change the target in the Makefile.

The compiled compiler is ```../build/compiler```

Generated Assembler file will be stored in ```../build/output.s```

# Usage
  - ```make example``` to compile & execute the example (default: ```../code/example.c```)
  - ```make valgrind``` to check for memory leaks
  - ```make test``` to execute all tests.

# Credits
This whole project follows [Nora Sandlers](https://norasandler.com) blog posts about writing a C compiler. Her blog is easy to follow and very well written. I highly recommend her blog if you want to learn more about compilers.
