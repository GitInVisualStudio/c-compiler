#ifndef GEN_H
#define GEN_H

#include "parser.h"

/**
 * @brief used to generate unique labels
 * 
 */
extern int clause_count;

/**
 * @brief compiles input_file into x86 asm and stores the code in output_file
 * 
 * @param input_file 
 * @param output_file 
 */
void compile(char* input_file, char* output_file);

/**
 * @brief generates the code for a body of the AST
 * 
 * executes the right generator of the body based on its type
 * 
 * @param fp 
 * @param body 
 */
void gen_code(FILE* fp, body* body);

/**
 * each body type has its own generator function
 */

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

/**
 * @brief writes clause names for example for if statemetns, all arguments must be allocated
 * 
 * @param child 
 * @param end 
 */
void get_clause_names(char* child, char* end);

/**
 * @brief writes label names of a loop, all arguments must be allocated
 * 
 * @param child 
 * @param end 
 * @param label 
 */
void get_label_names(char* child, char* end, int label);

/**
 * @brief generic generator function
 * 
 */
typedef void(*generator)(FILE*, body*);

/**
 * @brief each generator corresponds to one body type, sorted based on the body types index
 * 
 */
extern const generator gens[];

#endif