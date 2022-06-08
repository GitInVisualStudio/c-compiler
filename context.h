#ifndef CONTEXT_H
#define CONTEXT_H

#include "parser.h"

/**
 * @brief stores the context (scope) of the program at a given point
 * needs to save the current stack offset as well the stack offset difference to the previous scope
 * to allocate local variables at the right position, and de-allocate them after the scope has ended
 */
typedef struct context {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    char** variables;
    int stack_offset;
    int stack_offset_dif;
    int labels_count;
    char** function_names;
    int* function_params;
    int functions_length;
} context;

/**
 * @brief appends a variable to the context.
 * 
 * increases the stack_offset
 * 
 * @param context 
 * @param name 
 */
void append_variable(context* context, char* name);

context* copy_context(context* context);

/**
 * @brief returns the stack offset of the variable with the identifier name
 * 
 * @param context current context
 * @param name variable name
 * @return int -1 if the variable does not exist
 */
int get_offset(context* context, char* name);

/**
 * @brief appends a function to the context
 * 
 * increases the label_count by one
 * 
 * @param context current cotnext
 * @param name name of the fucntion
 * @param argc amount of parameter
 */
void append_function(context* context, char* name, int argc);

/**
 * @brief returns the argument count of a function name
 * 
 * @param context 
 * @param name 
 * @return int -1 if function is not found
 */
int get_argc(context* context, char* name);

void __print_context(body* body, int depth);
void __free_context(body* body);

#endif