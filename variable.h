#ifndef VARIABLE_H
#define VARIABLE_H

#include "parser.h"

/**
 * @brief 
 * name for debug purposes only
 * offset tells the position where the variable is stored on the stack
 * NOTE: the offset is positive, but the stack grows downwards, meaning the actual stack offset is -offset
 */
typedef struct variable {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    char name[INPUT_SIZE];
    int offset;
} variable;

void parse_assignment(lexer* lexer, body** body, context* context);
void parse_op_assign(lexer* lexer, variable* body, context* context, TOKENS op);
void parse_declaration(lexer* lexer, body** state, context* context);

void __print_variable(body* body, int depth);

#endif