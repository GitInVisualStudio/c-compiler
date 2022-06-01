COMPILER_NAME = compiler
SRC_FILE = ../code/example.c
OUTPUT_FILE = ../build/output.s

setup:
ifeq (,$(wildcard ../build))
	mkdir ../build
endif
ifeq (,$(wildcard ../code))
	mkdir ../code
# print simple example c code
	@echo "int main() {\n\treturn 5 * 2;\n}" > ../code/example.c
endif

compile: setup lexer.o parser.o gen.o
	gcc -O2 -o ../build/$(COMPILER_NAME) main.c lexer.o parser.o gen.o
	rm *.o

run: compile
	../build/$(COMPILER_NAME) $(SRC_FILE) $(OUTPUT_FILE)
	
valgrind: compile
	valgrind --track-origins=yes --leak-check=full ../build/$(COMPILER_NAME) $(SRC_FILE) $(OUTPUT_FILE)

example: run
	gcc -o ../build/out $(OUTPUT_FILE)
	../build/out

verify: # verify example output with gcc compiled version
	gcc -o ../build/out $(SRC_FILE)
	../build/out

test: compile
	python3 ./tests/test.py ../build/$(COMPILER_NAME)	
