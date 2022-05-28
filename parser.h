#include "lexer.h"

#ifndef PARSER_H
#define PARSER_H

/**
 * @brief offset size of the stack per push
 */
#define OFFSET_SIZE 8

/**
 * @brief used to tell what labels each element has -> needs to be done to tell continue & break where to go
 * 
 */
static int label_count = 0;

/**
 * @brief All different body types an AST can have, start at 0
 * 
 */
typedef enum BODY_TYPES {
    CONSTANT=0,
    RETURN,
    FUNCTION,
    PROGRAM,
    UNARY_OPS,
    EXPRESSION,
    ASSIGN,
    LIST,
    DECLARE,
    VARIABLE,
    IF,
    WHILE,
    FOR,
    CONTINUE,
    BREAK,
    CALL,
    CONTEXT,
} BODY_TYPES;

/**
 * @brief string representation of each body type
 * 
 */
static const char* BODY_TYPES_NAMES[] = {
    "CONSTANT",
    "RETURN",
    "FUNCTION",
    "PROGRAM",
    "UNARY_OPS",
    "EXPRESSION",
    "ASSIGN",
    "LIST",
    "DECLARE",
    "VARIABLE",
    "IF",
    "WHILE",
    "FOR",
    "CONTINUE",
    "BREAK",
    "CALL",
    "CONTEXT"
};

/**
 * @brief function call, child is always NULL
 * each argument is an expression
 */
typedef struct function_call {
    BODY_TYPES type;
    struct body* child;
    char name[INPUT_SIZE];
    struct body** expressions;
    int argc;
} function_call;

/**
 * @brief continue or break statements
 * needs the label to identifiy the loop
 */
typedef struct statement {
    BODY_TYPES type;
    struct body* child;
    int label;
} statement;

/**
 * @brief 
 * name for debug purposes only
 * offset tells the position where the variable is stored on the stack
 * NOTE: the offset is positive, but the stack grows downwards, meaning the actual stack offset is -offset
 */
typedef struct variable {
    BODY_TYPES type;
    struct body* child;
    char name[INPUT_SIZE];
    int offset;
} variable;

/**
 * @brief if condition, else body is optional and can be NULL
 * 
 */
typedef struct if_body {
    BODY_TYPES type;
    struct body* child;
    struct body* else_child;
    struct body* condition;
} if_body;

/**
 * @brief constant integer literal 
 */
typedef struct constant {
    BODY_TYPES type;
    struct body* child;
    int value;
} constant;

/**
 * @brief an expression has 2 children: child (op) child
 * for example: 1 + 2
 */
typedef struct expression {
    BODY_TYPES type;
    struct body* child;
    struct body* child2;
    TOKENS op;
} expression;

/**
 * @brief unary operator only have one child 
 */
typedef struct unaryOps {
    BODY_TYPES type;
    struct body* child;
    TOKENS op;
} unaryOps;

/**
 * @brief 
 * label count same as for
 */
typedef struct while_body {
    BODY_TYPES type;
    struct body* child;
    struct body* condition;
    int label;
} while_body;

/**
 * @brief for body
 * needs to store the label count to generate the right label names for break and continue statements
 */
typedef struct for_body {
    BODY_TYPES type;
    struct body* child;
    struct body* condition;
    struct body* init;
    struct body* expr;
    int label;
} for_body;

/**
 * @brief linked list of bodys used to have multiple statements as one child
 */
typedef struct list {
    BODY_TYPES type;
    struct body* child;
    struct list* next;
} list;

/**
 * @brief stores the context (scope) of the program at a given point
 * needs to save the current stack offset as well the stack offset difference to the previous scope
 * to allocate local variables at the right position, and de-allocate them after the scope has ended
 */
typedef struct context {
    BODY_TYPES type;
    struct body* child;
    char** variables;
    int stack_offset;
    int stack_offset_dif;
    int labels_count;
    char** function_names;
    int* function_params;
    int functions_length;
} context;

/**
 * @brief function has an identifier name and an amount of arguments
 * NOTE: argc stores the amount of arguments, but not their type, nor their name
 * we don't need to store the type, because we only support integers 
 * the individual identifiers of the arguments will be stored in the context (scope) of that function, which will be passed extra
 */
typedef struct function {
    BODY_TYPES type;
    struct body* child;
    char name[INPUT_SIZE];
    int argc;
} function;

/**
 * @brief represents a program, one program can have multiple functions
 */
typedef struct program {
    BODY_TYPES type;
    struct body* child;
    struct function** funcs;
    int func_length;
} program;

/**
 * @brief generic body type to represent an AST (abstract syntax tree)
 * stores the type and any further children of the AST in child
 */
typedef struct body {
    BODY_TYPES type;
    struct body* child;
} body;

/**
 * @brief each parse function corresponds to one body type of the AST, points the original pointer towards the new parsed element
 * 
 * @param lexer current lexer
 * @param body pointer to the pointer of a body, not an array of pointer
 * @param context current scope of the program in that point
 */

void parse_program(lexer* lexer, body** program);
void parse_function(lexer* lexer, function** function, context* context);
void parse_statement(lexer* lexer, body** statement, context* context);
void parse_expression(lexer* lexer, body** expression, context* context);
void parse_logical_or_exp(lexer* lexer, body** expression, context* context);
void parse_logical_and_exp(lexer* lexer, body** expression, context* context);
void parse_equality_exp(lexer* lexer, body** expression, context* context);
void parse_relational_exp(lexer* lexer, body** expression, context* context);
void parse_additive_exp(lexer* lexer, body** expression, context* context);
void parse_term(lexer* lexer, body** term, context* context);
void parse_factor(lexer* lexer, body** factor, context* context);
void parse_return(lexer* lexer, body** body, context* context);
void parse_declaration(lexer* lexer, body** state, context* context);
void parse_if(lexer* lexer, body** body, context* context);
void parse_list(lexer* lexer, body** body, context* context);
void parse_else(lexer* lexer, body** body, context* context);
void parse_while(lexer* lexer, body** body, context* context);
void parse_for(lexer* lexer, body** body, context* context);
void parse_continue(lexer* lexer, body** body, context* context);
void parse_break(lexer* lexer, body** body, context* context);
void parse_assignment(lexer* lexer, body** body, context* context);
void parse_call(lexer* lexer, body** body, context* context, token* prev);
void parse_parameter(lexer* lexer, context* context);
void parse_op_assign(lexer* lexer, variable* body, context* context, TOKENS op);

/**
 * @brief copies the strings of src into dest, dest needs to be allocated
 * 
 * @param dest string array
 * @param src string array
 * @param length how many strings to copy
 */
void cpy_str_arry(char** dest, char** src, int length);

/**
 * @brief generic parser function
 */
typedef void(*parser)(lexer*, body**, context*);

/**
 * @brief parses an expression
 * 
 * because every operator does not have the same priority, we need to parse expressions with higher priority first
 * the structur of each expression stays the same, only their priority is different. 
 * This function is the generic function to parse an expression with unknown prioriy
 * 
 * @param lexer lexer to read from
 * @param body child that needs to be set
 * @param context current context of that expression
 * @param p parser to the next higher priority expression
 * @param tokens tokens of equal priority
 * @param tokens_length 
 */
void parse_expressions(lexer* lexer, body** body, context* context, parser p, TOKENS* tokens, int tokens_length);

/**
 * @brief returns whether token is in tokens
 * 
 * @param tokens 
 * @param tokens_length 
 * @param token 
 * @return true 
 * @return false 
 */
bool contains_token(TOKENS* tokens, int tokens_length, TOKENS token);

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

/**
 * @brief scans the next token, safes the content into token and checks whether the token equals type, invokes fail if not
 * 
 * @param lexer current lexer
 * @param token token struct to be filled with content
 * @param type expected token type
 */
void check_valid(lexer* lexer, token* token, TOKENS type);

/**
 * @brief prints the error msg and exits the program
 * 
 * @param token 
 * @param expected 
 */
void fail(token* token, TOKENS expected);
void fail_error(char* error);

/**
 * @brief prints the AST of a program
 * 
 * @param program 
 * @param depth 
 */
void print_program(body* program, int depth);

/**
 * @brief frees all the allocated memory of the AST
 * 
 * @param program 
 */
void parse_free(body* program);

#endif