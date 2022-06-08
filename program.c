#include "header/program.h"

void parse_program(lexer* lexer, body** prog) {
    program* new;
    DEFAULT_BODY(new, struct program, PROGRAM);
    new->free = __free_program;
    new->print = __print_program;
    *prog = (body*)new;
    
    struct context* context = (struct context*)calloc(sizeof(struct context), 1);
    context->type = CONTEXT;
    context->free = __free_context;
    token current;

    do {
        new->func_length++;
        new->funcs = (function**)realloc(new->funcs, sizeof(function*) * new->func_length);

        parse_function(lexer, &new->funcs[new->func_length - 1], context);

        lexer_peak(lexer, &current);
    } while (current.token_type != EOF_TOKEN);

    FREE_BODY((struct body*)context);
}

void __free_program(body* body) {
    program* prog = (program*)body;
    if (prog->func_length > 0) {
        for (int i = 0; i < prog->func_length; i++)
            FREE_BODY((struct body*)prog->funcs[i]);
        free(prog->funcs);
    }
}

void __print_program(body* body, int depth) {
    program* prog = (program*)body;
    printf("PROGRAM:\n");
    PRINT_BODY(body->child, depth + 1);
    for (int i = 0; i < prog->func_length; i++)
        PRINT_BODY((struct body*)prog->funcs[i], depth + 1);
}