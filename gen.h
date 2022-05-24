#include "parser.h"

#ifndef GEN_H
#define GEN_H

static int clause_count = 0;

void free_var_list();

void compile(char* input_file, char* output_file);

void gen_code(FILE* fp, body* body);

void gen_constant(FILE* fp, body* body);
void gen_return(FILE* fp, body* body);
void gen_function(FILE* fp, body* body);
void gen_program(FILE* fp, body* body);
void gen_unaryops(FILE* fp, body* body);
void gen_expression(FILE* fp, body* body);
void gen_assign(FILE* fp, body* body);
void gen_list(FILE* fp, body* body);
void gen_declare(FILE* fp, body* body);
void gen_variable(FILE* fp, body* body);
void gen_if(FILE* fp, body* body);
void gen_while(FILE* fp, body* body);
void gen_for(FILE* fp, body* body);
void gen_statement(FILE* fp, body* body);
void gen_call(FILE* fp, body* body);
void gen_context(FILE* fp, body* body);

void get_clause_names(char* child, char* end);
void get_label_names(char* child, char* end, int label);

typedef void(*generator)(FILE*, body*);

const static generator gens[] = {
    gen_constant, 
    gen_return, 
    gen_function, 
    gen_program, 
    gen_unaryops, 
    gen_expression, 
    gen_assign,
    gen_list,
    gen_declare,
    gen_variable,
    gen_if,
    gen_while,
    gen_for,
    gen_statement,
    gen_statement,
    gen_call,
    gen_context
};

#endif