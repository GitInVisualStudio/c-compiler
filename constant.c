#include "header/constant.h"

void parser_constant(lexer* lexer, body** body, context* context) {
    token current;
    lexer_next(lexer, &current);

    constant* cons;
    DEFAULT_BODY(cons, struct constant, CONSTANT);
    cons->print = __print_constant;
    cons->value = atoi(current.value);
    *body = (struct body*)cons;
}

void __print_constant(body* body, int depth) {
    printf("%s: %i\n", BODY_TYPES_NAMES[body->type], ((constant*)body)->value);
}