#ifndef PROGRAM_H
#define PROGRAM_H

#include "parser.h"

/**
 * @brief represents a program, one program can have multiple functions
 */
typedef struct program {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    struct function** funcs;
    int func_length;
} program;

void parse_program(lexer* lexer, body** program);

void __free_program(body* body);
void __print_program(body* body, int depth);

#endif