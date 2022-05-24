compile: lexer.o parser.o gen.o
	gcc -O2 -o ../build/main main.c lexer.o parser.o gen.o
	rm *.o

run: compile
	../build/main ../code/return_2.c ../build/output.s
	
valgrind: compile
	valgrind --track-origins=yes --leak-check=full ../build/main ../code/return_2.c ../build/output.s

test: run
	gcc ../build/output.s -o ../build/out
	../build/out

verify: 
	gcc -o ../build/out ../code/return_2.c
	../build/out
