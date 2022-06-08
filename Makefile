COMPILER_NAME = compiler
BUILDDIR = ./
EXAMPLE_FILE = ../code/example.c
OUTPUT_FILE = ../build/output.s

CC	= gcc
CFLAGS	= -Wall -Wextra -D_FORTIFY_SOURCE=2 -O2 -pipe -Werror=format-security -Werror=implicit-function-declaration

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)

setup:
ifeq (,$(wildcard ../build))
	mkdir ../build
endif
ifeq (,$(wildcard ../code))
	mkdir ../code
	@echo "int main() {\n\treturn 5 * 2;\n}" > ../code/example.c
endif

compile: setup $(OBJS)
	$(CC) $(CFLAGS) -o ../build/$(COMPILER_NAME) $(OBJS)
	rm *.o

%.o: %.c
	$(CC) -c $< -o $@

run: compile
	../build/$(COMPILER_NAME) $(EXAMPLE_FILE) $(OUTPUT_FILE)
	
valgrind: compile
	valgrind --track-origins=yes --leak-check=full ../build/$(COMPILER_NAME) $(EXAMPLE_FILE) $(OUTPUT_FILE)

example: run
	gcc -o ../build/out $(OUTPUT_FILE)
	../build/out

verify: # verify example output with gcc compiled version
	gcc -o ../build/out $(EXAMPLE_FILE)
	../build/out

test: compile
	python3 ./tests/test.py ../build/$(COMPILER_NAME)	
