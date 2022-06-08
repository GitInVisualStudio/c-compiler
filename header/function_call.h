#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "parser.h"

/**
 * @brief function call, child is always NULL
 * each argument is an expression
 */
typedef struct function_call {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    char name[INPUT_SIZE];
    struct body** expressions;
    int argc;
} function_call;

void parse_call(lexer* lexer, body** body, context* context, token* prev);

void __print_call(body* body, int depth);
void __free_call(body* body);

#endif