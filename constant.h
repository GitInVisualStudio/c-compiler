#ifndef CONSTANT_H
#define CONSTANT_H

#include "parser.h"

/**
 * @brief constant integer literal 
 */
typedef struct constant {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    int value;
} constant;


void parser_constant(lexer* lexer, body** body, context* context);

void __print_constant(body* body, int depth);

#endif