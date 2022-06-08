#include "header/function.h"

void parse_parameter(lexer* lexer, context* context) {
    token current;
    check_valid(lexer, &current, INT_KEYWORD);

    check_valid(lexer, &current, IDENTIFIER);

    if (get_offset(context, current.value) != -1) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "Variable: (%s) already declared!", current.value);
        fail_error(error_msg);
    }

    // only add the variable to the context list of variables
    append_variable(context, current.value);
}

void parse_function(lexer* lexer, function** func, context* context) {
    function* new;
    DEFAULT_BODY(new, struct function, FUNCTION);
    new->print = __print_function;
    *func = new;

    token current;

    check_valid(lexer, &current, INT_KEYWORD);

    check_valid(lexer, &current, IDENTIFIER);

    // copy the function name into the AST 
    strcpy(new->name, current.value);

    check_valid(lexer, &current, O_PARENTHESIS);

    struct context* new_context = copy_context(context);
    new->child = (struct body*)new_context;
    
    lexer_peak(lexer, &current);

    while(current.token_type == INT_KEYWORD) {
        new->argc++;
        if (new->argc > 6) {
            fail_error("No functions supported with more than 6 arguments!");
        }
        parse_parameter(lexer, new_context);

        lexer_peak(lexer, &current);
        if (current.token_type == COMMA) {
            lexer_next(lexer, &current);
            lexer_peak(lexer, &current);
        }
    }

    new_context->stack_offset = OFFSET_SIZE * new->argc;

    check_valid(lexer, &current, C_PARENTHESIS);
    lexer_peak(lexer, &current);

    int context_argc = get_argc(context, new->name);
    if (context_argc != new->argc && context_argc != -1) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "new definition of function with same name: (%s)", new->name);
        fail_error(error_msg);
    }

    append_function(context, new->name, new->argc);

    if (current.token_type == SEMICOLON) {
        new->child = NULL;
        FREE_BODY((body*)new_context);
        lexer_next(lexer, &current);
        return;
    }

    append_function(new_context, new->name, new->argc);

    parse_list(lexer, &new_context->child, new_context);
}

void __print_function(body* body, int depth) {
    function* func = (function*)body;
    printf("%s (%s):\n", BODY_TYPES_NAMES[func->type], func->name);
    PRINT_BODY(func->child, depth + 1);
}
