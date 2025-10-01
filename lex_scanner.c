#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "lex_scanner.h"

int key;
typedef struct{
    char *buffer;
    size_t length;
    size_t capacity;
} LexemeBuffer;

LexemeBuffer* initBuffer(size_t initial_size){
    LexemeBuffer* lb = malloc(sizeof(LexemeBuffer));
    lb->buffer = malloc(initial_size);
    lb->buffer[0] = '\0';
    lb->length = 0;
    lb->capacity = initial_size;
    return lb;
}

void freeBuffer(LexemeBuffer* lb){
    free(lb->buffer);
    free(lb);
} 

void resetBuffer(LexemeBuffer* lb){
    lb->length = 0;
    if (lb->buffer) lb->buffer[0] = '\0';
}

void appendChar(LexemeBuffer* lb, char c){
    if (lb->length + 1 >= lb->capacity){
        lb->capacity *= 2;
        lb->buffer = realloc(lb->buffer, lb->capacity);
        if (!lb->buffer){
            fprintf(stderr, "Nedostatek paměti při realloc!\n");
            exit(1);
        }
    }
    lb->buffer[lb->length++] = c;
    lb->buffer[lb->length] = '\0';
}

char * copyString(const char * str){
    char* copy = malloc(strlen(str) + 1);
    if(copy) strcpy(copy, str);
    return copy;
}

State transition(State state, int key, LexemeBuffer* lexeme){
    switch(state){
        case Start:
            switch(key){
                case '*':
                    createToken(TIMES, lexeme);
                    return Start;
            }
    }
}

Token createToken(TokenType type, LexemeBuffer* lexeme){
    Token token;
    token.type = type;

    switch(type){
        case IDENTIFIER_GLOBAL:
        case IDENTIFIER_LOCAL:
            token.lexeme = copyString(lexeme->buffer);
            break;
        case INT:
            token.lexeme = copyString(lexeme->buffer);
            token.value.int_val = atoi(lexeme->buffer);
            break;
        case FLOAT:
            token.lexeme = copyString(lexeme->buffer);
            token.value.float_val = atof(lexeme->buffer);
            break;
        case STRING:
            token.lexeme = copyString(lexeme->buffer);
            token.value.string_val = copyString(lexeme->buffer);
            break;
        default:
            token.lexeme = NULL;
            break;
    }

    resetBuffer(lexeme);
    return token;
}

int main(){
    LexemeBuffer* lb = initBuffer(16);
    State state = Start;
    while (key != EOF){
        key = getchar();
        appendChar(lb, key);
        state = transition(state, key, lb);
    }
    freeBuffer(lb);
    return 0;
}