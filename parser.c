#include "header/parser.h"

int label_count = 0;

void fail_error(char* error) {
    fprintf(stderr, "%s\n", error);
    exit(-1);
}

void fail(token* token, TOKENS expected) {
    fprintf(stderr, "Expected token: %s, got %s!\n", TOKEN_NAMES[expected], TOKEN_NAMES[token->token_type]);
    exit(-1);
}

void check_valid(lexer* lexer, token* token, TOKENS type) {
    lexer_next(lexer, token);
    if (token->token_type != type) fail(token, type);
}

void cpy_str_arry(char** dest, char** src, int length) {
    for (int i = 0; i < length; i++) {
        int str_length = strlen(src[i]);
        dest[i] = (char*)calloc(sizeof(char), str_length + 1);
        strcpy(dest[i], src[i]);
    }
}

bool contains_token(TOKENS* tokens, int tokens_length, TOKENS token) {
    for (int i = 0; i < tokens_length; i++)
        if (tokens[i] == token)
            return true;
    return false;
}

void __free_body(body* body) {
    if (body == NULL) return;
    if (body->free != NULL)
        body->free(body);
    __free_body(body->child);
    free(body);
}

void __print_body(body* body, int depth) {
    if (body == NULL) return;
    for (int i = 0; i < depth && body->type != LIST && body->type != CONTEXT; i++)
        printf("%s", SPACE);
    body->print(body, depth);
}

void __default_print_body(body* body, int depth) {
    printf("%s:\n", BODY_TYPES_NAMES[body->type]);
    PRINT_BODY(body->child, depth + 1);
}