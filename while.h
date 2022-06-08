#ifndef WHILE_H
#define WHILE_H

#include "parser.h"

/**
 * @brief 
 * label count same as for
 */
typedef struct while_body {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    struct body* condition;
    int label;
} while_body;

void parse_while(lexer* lexer, body** body, context* context);

void __free_while(body* body);
void __print_while(body* body, int depth);

#endif