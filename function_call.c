#include "header/function_call.h"

void parse_call(lexer* lexer, body** body, context* context, token* prev) {
    token current;
    
    int argc = get_argc(context, prev->value);

    if (argc == -1) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "Function (%s) not defined!", prev->value);
        fail_error(error_msg);
    }

    function_call* call;
    DEFAULT_BODY(call, struct function_call, CALL);
    call->free = __free_call;
    call->print = __print_call;
    strcpy(call->name, prev->value);

    check_valid(lexer, &current, O_PARENTHESIS);

    lexer_peak(lexer, &current);

    while(current.token_type != C_PARENTHESIS) {
        call->argc++;
        call->expressions = (struct body**)realloc(call->expressions, sizeof(struct body*) * call->argc);
        parse_expression(lexer, &call->expressions[call->argc - 1], context);

        lexer_peak(lexer, &current);
        if(current.token_type == COMMA) {
            lexer_next(lexer, &current);
        }
    }

    if (call->argc != argc) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "Wrong amount of arguments! Expected: %i Got: %i", argc, call->argc);
        fail_error(error_msg);
    }

    check_valid(lexer, &current, C_PARENTHESIS);

    *body = (struct body*)call;
}

void __print_call(body* body, int depth) {
    function_call* call = (function_call*)body;
    printf("%s (%s):\n", BODY_TYPES_NAMES[body->type], call->name);
    for (int i = 0; i < call->argc; i++) {
        PRINT_BODY(call->expressions[i], depth + 1);    
    }
}

void __free_call(body* body) {
    function_call* call = (function_call*)body;
    for (int i = 0; i < call->argc; i++) 
        FREE_BODY(call->expressions[i]);
    if (call->argc > 0)
        free(call->expressions);
}