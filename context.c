#include "header/context.h"

context* copy_context(context* context) {
    struct context* new;
    DEFAULT_BODY(new, struct context, CONTEXT);
    new->print = __print_context;
    new->free = __free_context;
    if (context == NULL) {
        return new;        
    }
    new->stack_offset = context->stack_offset;
    new->labels_count = context->labels_count;
    int length = (context->stack_offset/OFFSET_SIZE);
    if (length > 0) {
        new->variables = (char**)calloc(sizeof(char*), length);
        cpy_str_arry(new->variables, context->variables, length);
    }
    int func_length = context->functions_length;
    if (func_length > 0) {
        new->functions_length = context->functions_length;
        new->function_names = (char**)calloc(sizeof(char*), func_length);
        cpy_str_arry(new->function_names, context->function_names, func_length);
        new->function_params = (int*)calloc(sizeof(int), func_length);
        memcpy(new->function_params, context->function_params, sizeof(int) * func_length);
    }
    return new;
}

void append_variable(context* context, char* name) {
    context->stack_offset += OFFSET_SIZE;
    int stack_length = (context->stack_offset/OFFSET_SIZE);
    int name_length = strlen(name);
    context->variables = (char**)realloc(context->variables, sizeof(char*) * stack_length);
    context->variables[stack_length - 1] = (char*)calloc(name_length + 1, sizeof(char));
    strcpy(context->variables[stack_length - 1], name);
}

int get_offset(context* context, char* name) {
    int stack_length = (context->stack_offset/OFFSET_SIZE);
    for (int i = 0; i < stack_length; i++) 
        if (strcmp(name, context->variables[i]) == 0)
            return (i + 1) * OFFSET_SIZE;
    return -1;
}

void append_function(context* context, char* name, int argc) {
    int length = ++context->functions_length;
    context->function_names = (char**)realloc(context->function_names, sizeof(char*) * length);
    context->function_params = (int*)realloc(context->function_params, sizeof(int) * length);
    context->function_params[length - 1] = argc;

    int str_length = strlen(name);
    context->function_names[length - 1] = (char*)calloc(sizeof(char), str_length + 1);
    strcpy(context->function_names[length - 1], name);
}

int get_argc(context* context, char* name) {
    for (int i = 0; i < context->functions_length; i++) {
        if (strcmp(context->function_names[i], name) == 0)
            return context->function_params[i];
    }
    return -1;
}

void __print_context(body* body, int depth) {
    PRINT_BODY(body->child, depth + 1);
}

void __free_context(body* body) {
    struct context* c = (struct context*)body;
    //TODO free everything
    for (int i = 0; i < c->stack_offset/OFFSET_SIZE; i++) 
        free(c->variables[i]);

    if (c->stack_offset != 0)
        free(c->variables);

    for (int i = 0; i < c->functions_length; i++) 
        free(c->function_names[i]);
    
    if (c->functions_length != 0) {
        free(c->function_names);
        free(c->function_params);
    }
}