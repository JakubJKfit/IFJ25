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

State transition(State state, int key){
    switch(state){
        case Start:
            switch(key){
                case '*':
                    createToken(TIMES, NULL);
                    return Start;
            }
    }
}

Token createToken(TokenType type, char* lexeme){
    Token token;
    token.type = type;
    token.lexeme = copyString(lexeme);

    return token;
}

int main(){
    LexemeBuffer* lb = initBuffer(16);
    State state = Start;
    while (key != EOF){
        key = getchar();
        appendChar(lb, key);
        state = transition(state, key);
    }
    freeBuffer(lb);
    return 0;
}