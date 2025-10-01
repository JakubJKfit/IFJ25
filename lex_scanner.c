#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "lex_scanner.h"

typedef struct{
    char *buffer;
    size_t length;
    size_t capacity;
} LexemeBuffer;

Token createToken(TokenType type, LexemeBuffer* lexeme);

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

void removeLast(LexemeBuffer* lb){
    if (lb->length > 0){
        lb->buffer[--lb->length] = '\0';
    }
}

char * copyString(const char * str){
    char* copy = malloc(strlen(str) + 1);
    if(copy) strcpy(copy, str);
    return copy;
}

State builtInFunctions(LexemeBuffer* lexeme){
    if (strcmp(lexeme->buffer, "read_str") == 0) {
        createToken(IFJ_READ_STR, lexeme);
    } else if (strcmp(lexeme->buffer, "read_num") == 0) {
        createToken(IFJ_READ_NUM, lexeme);
    } else if (strcmp(lexeme->buffer, "write") == 0) {
        createToken(IFJ_WRITE, lexeme);
    } else if (strcmp(lexeme->buffer, "floor") == 0) {
        createToken(IFJ_FLOOR, lexeme);
    } else if (strcmp(lexeme->buffer, "str") == 0) {
        createToken(IFJ_STR, lexeme);
    } else if (strcmp(lexeme->buffer, "length") == 0) {
        createToken(IFJ_LENGTH, lexeme);
    } else if (strcmp(lexeme->buffer, "substring") == 0) {
        createToken(IFJ_SUBSTRING, lexeme);
    } else if (strcmp(lexeme->buffer, "strcmp") == 0) {
        createToken(IFJ_STRCMP, lexeme);
    } else if (strcmp(lexeme->buffer, "ord") == 0) {
        createToken(IFJ_ORD, lexeme);
    } else if (strcmp(lexeme->buffer, "chr") == 0) {
        createToken(IFJ_CHR, lexeme);
    } else {
        return BuiltIn_fun;
    }
    return Start;
}

State keywords(LexemeBuffer* lexeme){
    if (strcmp(lexeme->buffer, "class") == 0) {
        createToken(KEYWORD_class, lexeme);
    } else if (strcmp(lexeme->buffer, "if") == 0) {
        createToken(KEYWORD_if, lexeme);
    } else if (strcmp(lexeme->buffer, "else") == 0) {
        createToken(KEYWORD_else, lexeme);
    } else if (strcmp(lexeme->buffer, "is") == 0) {
        createToken(KEYWORD_is, lexeme);
    } else if (strcmp(lexeme->buffer, "null") == 0) {
        createToken(KEYWORD_null, lexeme);
    } else if (strcmp(lexeme->buffer, "return") == 0) {
        createToken(KEYWORD_return, lexeme);
    } else if (strcmp(lexeme->buffer, "var") == 0) {
        createToken(KEYWORD_var, lexeme);
    } else if (strcmp(lexeme->buffer, "while") == 0) {
        createToken(KEYWORD_while, lexeme);
    } else if (strcmp(lexeme->buffer, "static") == 0) {
        createToken(KEYWORD_static, lexeme);
    } else if (strcmp(lexeme->buffer, "import") == 0) {
        createToken(KEYWORD_import, lexeme);
    } else if (strcmp(lexeme->buffer, "for") == 0) {
        createToken(KEYWORD_for, lexeme);
    } else if (strcmp(lexeme->buffer, "Num") == 0) {
        createToken(KEYWORD_Num, lexeme);
    } else if (strcmp(lexeme->buffer, "String") == 0) {
        createToken(KEYWORD_String, lexeme);
    } else if (strcmp(lexeme->buffer, "Null") == 0) {
        createToken(KEYWORD_Null, lexeme);
    } else {
        return Letter;
    }
    return Start;
}

char escapeChar(){
    int c = getchar();
    if (c == EOF) return -1;

    switch(c){
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '\\': return '\\';
        case '"': return '"';
        case 'x': {
            int h1 = getchar();
            int h2 = getchar();
            if(h1 == EOF || h2 == EOF || !isxdigit(h1) || !isxdigit(h2)) return -1;
            char hex[3] = {h1, h2, '\0'};
            return (char) strtol(hex, NULL, 16);
        }
        default: return c;
    }
}

State transition(State state, int key, LexemeBuffer* lexeme){
    static int quote_count = 0;
    switch(state){
        case Start:
            switch(key){
                case '*':
                    createToken(TIMES, lexeme);
                    return Start;
                case '/':
                    return Divide;
                case '+':
                    createToken(PLUS, lexeme);
                    return Start;
                case '-':
                    createToken(MINUS, lexeme);
                    return Start;
                case '<':
                    return Lesser;
                case '>':
                    return Greater;
                case '=':
                    return Assign;
                case '(':
                    createToken(L_ROUND, lexeme);
                    return Start;
                case ')':
                    createToken(R_ROUND, lexeme);
                    return Start;
                case '{':
                    createToken(L_CURLY, lexeme);
                    return Start;
                case '}':
                    createToken(R_CURLY, lexeme);
                    return Start;  
                case '"':
                    quote_count++;
                    return String_detect; 
                case '_':
                    appendChar(lexeme, key);
                    return Id_global0;
                default:
                    if (isalpha(key)){
                        appendChar(lexeme, key);
                        return Letter;
                    }else if(isspace(key)){
                        return Start;
                    }else if(isdigit(key)){
                        appendChar(lexeme, key);
                        return Number;
                    }
                    ungetc(key, stdin);
                    return Error;         
            }

        case Number:
            appendChar(lexeme, key);
            if (isdigit(key)){
                return Number;
            }else if (key == '.'){
                return Float;
            }else if (key == 'e' || key == 'E'){
                return Exponent;
            }else if(strcmp(lexeme->buffer, "0x") == 0){
                return Hex;
            }
            removeLast(lexeme);
            createToken(INT, lexeme);
            ungetc(key, stdin);
            return Start;
        case Float:
            appendChar(lexeme, key);
            if (isdigit(key)){
                return Float;
            }else if (key == 'e' || key == 'E'){
                return Exponent;
            }
            removeLast(lexeme);
            createToken(FLOAT, lexeme);
            ungetc(key, stdin);
            ungetc(key, stdin);
            return Start;
        case Exponent:
            appendChar(lexeme, key);
            if (isdigit(key)){
                return Exponent_float;
            }else if (key == '+' || key == '-'){
                return Exponent_sign;
            }
            removeLast(lexeme);
            createToken(FLOAT, lexeme);
            ungetc(key, stdin);
            return Start;
        case Exponent_sign:
            appendChar(lexeme, key);
            if (isdigit(key)) return Exponent_float;
            removeLast(lexeme);
            createToken(FLOAT, lexeme);
            ungetc(key, stdin);
            return Start;
        case Exponent_float:
            appendChar(lexeme, key);
            if (isdigit(key)){
                return Exponent_float;
            }
            removeLast(lexeme);
            createToken(FLOAT, lexeme);
            ungetc(key, stdin);
            return Start;
        case Hex:
            appendChar(lexeme, key);
            if (isdigit(key) || (key >= 'a' && key <= 'f') || (key >= 'A' && key <= 'F')){
                return Hex;
            }
            removeLast(lexeme);
            createToken(INT, lexeme);
            ungetc(key, stdin);
            return Start;

        case Letter:
            if (isalnum(key) || key == '_'){
                appendChar(lexeme, key);
                return keywords(lexeme);
            }else if (strcmp(lexeme->buffer, "Ifj") == 0){
                ungetc(key, stdin);
                return BuiltIn;
            }
            createToken(IDENTIFIER_LOCAL, lexeme);
            ungetc(key, stdin);
            return Start;
        case Id_global0:
            if (key == '_') {
                appendChar(lexeme, key);
                return Id_global;
            }
            ungetc(key, stdin);
            return Error;
        case Id_global:
            if (isalnum(key) || key == '_'){
                appendChar(lexeme, key);
                return Id_global;
            }
            createToken(IDENTIFIER_GLOBAL, lexeme);
            return Start;

        case String_detect:
            if (key == '"'){
                quote_count++;
                if (quote_count == 3){
                    quote_count = 0;
                    return String_multi;
                }
                return String_detect;
            }else {
                if (quote_count == 2){
                    quote_count = 0;
                    createToken(STRING, lexeme);
                    ungetc(key, stdin);
                    return Start;
                }
                resetBuffer(lexeme);
                ungetc(key, stdin);
                quote_count = 0;
                return String_single;
            }
        case String_single:
            if (key == '"'){
                createToken(STRING, lexeme);
                return Start;
            }if (key == '\\'){
                int escaped = escapeChar();
                if (escaped == -1) {
                    createToken(ERROR, lexeme);
                    return Start;
                }
                appendChar(lexeme, (char)escaped);
                return String_single;
            }
            appendChar(lexeme, key);
            return String_single;
        case String_multi:
            if (key == '"'){
                quote_count++;
                if (quote_count == 3){
                    quote_count = 0;
                    createToken(STRING, lexeme);
                    return Start;
                }
                return String_multi;
            }
            quote_count = 0;
            appendChar(lexeme, key);
            return String_multi;

        // Built-in funkce
        case BuiltIn:
            if (isspace(key)){
                return BuiltIn;
            }else if(key == '.'){
                resetBuffer(lexeme);
                return BuiltIn_dot;
            }
            createToken(KEYWORD_Ifj, lexeme);
            ungetc(key, stdin);
            return Start;
        case BuiltIn_dot:
            if(isspace(key)){
                resetBuffer(lexeme);
                return BuiltIn_dot;
            }else if(isalpha(key)){
                appendChar(lexeme, key);
                return BuiltIn_fun;
            }else{
                ungetc(key, stdin);
                return Error;
            }
        case BuiltIn_fun:
            if (isalnum(key) || key == '_'){
                appendChar(lexeme, key);
                return builtInFunctions(lexeme);
            }
            ungetc(key, stdin);
            return Error;

        case Divide:
            if (key == '/'){
                return Comment_single;
            }else if (key == '*'){
                return Comment_multi;
            }
            createToken(DIVIDE, lexeme);
            ungetc(key, stdin);
            return Start;
        case Comment_single:
            if (key == '\n')return Start;
            return Comment_single;
        case Comment_multi:
            if (key == '*')return Comment_multi_end;
            return Comment_multi;
        case Comment_multi_end:
            if (key == '/')return Start;
            return Comment_multi;
        case Lesser:
            if (key == '='){
                createToken(LESSER_EQUAL, lexeme);
                return Start;
            }
            createToken(LESSER, lexeme);
            ungetc(key, stdin);
            return Start;
        case Greater:
            if (key == '=') {
                createToken(GREATER_EQUAL, lexeme);
                return Start;
            }
            createToken(GREATER, lexeme);
            ungetc(key, stdin);
            return Start;
        case Assign:
            if (key == '='){
                createToken(EQUAL, lexeme);
                return Start;
            }
            createToken(ASSIGN, lexeme);
            ungetc(key, stdin);
            return Start;
        case Error:
            createToken(ERROR, lexeme);
            return Start;
        default:
            ungetc(key, stdin);
            return Error;
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
        case ERROR:
            token.lexeme = copyString(lexeme->buffer);
            break;
        default:
            token.lexeme = NULL;
            break;
    }

    // Vypisovani tokenu na debuging
    /*if (token.lexeme) {
        printf("%s - %s\n", convert(token.type), token.lexeme);
        free(token.lexeme);
    }

    if (token.type == STRING && token.value.string_val) {
        free(token.value.string_val);
    }*/
    printf("%s - %s\n", convert(token.type), token.lexeme);
    resetBuffer(lexeme);
    return token;
}

int main(){
    LexemeBuffer* lb = initBuffer(16);
    State state = Start;
    int key;
    while ((key = getchar()) != EOF){
        state = transition(state, key, lb);
    }
    createToken(T_EOF, lb);
    freeBuffer(lb);
    return 0;
}