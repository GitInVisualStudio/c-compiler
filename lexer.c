#include "lexer.h"

lexer* lexer_open(char* filename) {
    lexer* new_lexer = (lexer*)malloc(sizeof(struct lexer));

    new_lexer->fp = fopen(filename, "r");

    if (new_lexer->fp == NULL) { //FILE DOES NOT EXIST/PERMISSION
        free(new_lexer);
        return NULL;
    }

    new_lexer->filename = (char*)malloc(sizeof(char) * (strlen(filename)  + 1));
    strcpy(new_lexer->filename, filename);
    
    return new_lexer;
}

void lexer_free(lexer* lexer) {
    free(lexer->filename);
    fclose(lexer->fp);
    free(lexer);
}

bool lexer_next(lexer* lexer, token* token) {

    char input[1024];

    if (fscanf(lexer->fp, "%s", input) == EOF) {
        token->token_type = EOF_TOKEN;
        return false;
    }

    int input_length = strlen(input);

    for (int i = 0; i < TOKENS_LENGTH; i++) {
        const char* symbol = TOKEN_SYMBOLS[i];
        if (symbol == NULL) continue;
        int length = strlen(symbol);
        
        /**
         * we need to compare N letters of the input, 
         * because tokens can be folled diretly on another without and leading whitespace 
         */
        if (strncmp(input, symbol, length) == 0) {

            /**
             * when the token is only substring of the input read,
             * we need to reset the seek position of the FP to the end of that token
             * otherwise we'd skip the next token
             */
            if (length != input_length) {
                fflush(lexer->fp);
                int offset = ftell(lexer->fp);
                fseek(lexer->fp, offset - (input_length - length), SEEK_SET);
            }
            strncpy(token->value, input, length);
            token->value[length] = '\0';
            token->token_type = i;
            return true;
        }
    }
    /**
     * int literals and identifiers don't have a static symbol, they need a custom implementation
     */
    if (__identifiy_token(lexer, token, input, __int_literal)) {
        token->token_type = INT_LITERAL;
        return true;
    }

    if (__identifiy_token(lexer, token, input, __identifier)) {
        token->token_type = IDENTIFIER;
        return true;
    }

    return false;
}

bool lexer_peak(lexer* lexer, token* token) {
    fflush(lexer->fp);
    int offset = ftell(lexer->fp);

    bool _return = lexer_next(lexer, token);
    fseek(lexer->fp, offset, SEEK_SET);

    return _return;
}

bool __identifiy_token(lexer* lexer, token* token, char input[INPUT_SIZE], token_condition condition) {
    int length = strlen(input);
    char* buffer = (char*)calloc(sizeof(char), (length + 1));    
    for (int i = 0; i < length; i++) {
        if (condition(input, i)) {
            buffer[i] = input[i];
        } else {
            /**
             * Look at lexer_next, tokens can have no leading whitespace ->
             * the input can consist of multiple tokens. If so we need to re-set the seek position
             */
            if (i > 0) {
                fflush(lexer->fp);
                int offset = ftell(lexer->fp);
                fseek(lexer->fp, offset - (length - i), SEEK_SET);
            }
            buffer[i] = '\0';
            break;
        }
    }

    if (strlen(buffer) == 0) {
        free(buffer);
        return false;
    }
    
    strcpy(token->value, buffer);

    free(buffer);

    return true;
}

bool __int_literal(char input[INPUT_SIZE], int index) {
    return input[index] >= '0' && input[index] <= '9';
}

bool __identifier(char input[INPUT_SIZE], int index) {
    return (input[index] >= 'a' && input[index] <= 'z') || (input[index] >= '0' && input[index] <= '9') || input[index] == '_';
}
