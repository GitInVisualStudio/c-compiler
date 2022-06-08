#include "while.h"

void parse_while(lexer* lexer, body** body, context* context) {
    token current;

    check_valid(lexer, &current, WHILE_KEYWORD);
    check_valid(lexer, &current, O_PARENTHESIS);
    
    while_body* _while;
    DEFAULT_BODY(_while, struct while_body, WHILE);
    _while->label = ++label_count;
    _while->print = __print_while;
    _while->free = __free_while;

    struct context* new_context = copy_context(context);
    new_context->labels_count++;
    _while->child = (struct body*)new_context;

    parse_expression(lexer, &_while->condition, new_context);

    check_valid(lexer, &current, C_PARENTHESIS);

    parse_list(lexer, &new_context->child, new_context);

    *body = (struct body*)_while;
}

void __free_while(body* body) {
    while_body* _while = (while_body*)body;
    FREE_BODY(_while->condition);
}

void __print_while(body* body, int depth) {
    while_body* _while = (while_body*)body;
    printf("%s%s-CONDITION: \n", SPACE, BODY_TYPES_NAMES[body->type]);
    PRINT_BODY(_while->condition, depth + 2);
    PRINT_BODY(_while->child, depth + 1);
}