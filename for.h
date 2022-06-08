#ifndef FOR_H
#define FOR_H

#include "parser.h"

/**
 * @brief for body
 * needs to store the label count to generate the right label names for break and continue statements
 */
typedef struct for_body {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    struct body* condition;
    struct body* init;
    struct body* expr;
    int label;
} for_body;

void parse_for(lexer* lexer, body** body, context* context);

void __free_for(body* body);
void __print_for(body* body, int depth);

#endif