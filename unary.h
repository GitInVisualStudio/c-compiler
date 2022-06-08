#ifndef UNARY_H
#define UNARY_H

#include "parser.h"

/**
 * @brief unary operator only have one child 
 */
typedef struct unaryOps {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    TOKENS op;
} unaryOps;

void parse_unary(lexer* lexer, body** body, context* context);

void __print_unary(body* body, int depth);

#endif