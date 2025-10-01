#ifndef LEX_SCANNER_H
#define LEX_SCANNER_H

typedef enum State{
    Start,
    Divide,
    Lesser,
    Greater,
    Assign,
    Comment,
    Error
}State;

typedef enum TokenType{
    KEYWORD_class,
    KEYWORD_if,
    KEYWORD_else,
    KEYWORD_is,
    KEYWORD_null,
    KEYWORD_return,
    KEYWORD_var,
    KEYWORD_while,
    KEYWORD_Ifj,
    KEYWORD_static,
    KEYWORD_import,
    KEYWORD_for,
    KEYWORD_Num,
    KEYWORD_String,
    KEYWORD_Null,
    IDENTIFIER_LOCAL,
    IDENTIFIER_GLOBAL,
    INT,
    FLOAT,
    STRING,
    PROLOG,
    TIMES,
    DIVIDE,
    PLUS,
    MINUS,
    LESSER,
    GREATER,
    LESSER_EQUAL,
    GREATER_EQUAL,
    EQUAL,
    NOT_EQUAL,
    ASSIGN,
    L_ROUND,
    R_ROUND,
    L_CURLY,
    R_CURLY,
    IFJ_READ_STR,
    IFJ_READ_NUM,
    IFJ_WRITE,
    IFJ_FLOOR,
    IFJ_STR,
    IFJ_LENGTH,
    IFJ_SUBSTRING,
    IFJ_STRCMP,
    IFJ_ORD,
    IFJ_CHR,
    ERROR,
    T_EOF
}TokenType;

typedef struct Token{
    TokenType type;
    char *lexeme;
    union value{
        int int_val;
        double float_val;
        char *string_val;
    }value;
}Token;

#endif