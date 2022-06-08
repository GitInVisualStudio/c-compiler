#ifndef LIST_H
#define LIST_H

#include "parser.h"

/**
 * @brief linked list of bodys used to have multiple statements as one child
 */
typedef struct list {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    struct list* next;
} list;

void parse_list(lexer* lexer, body** body, context* context);

void __free_list(body* body);
void __print_list(body* body, int depth);

#endif