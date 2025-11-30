/**
 * @file lex_scanner.h
 * @author Petr Molík xmolikp00
 * @brief Lexikální analýza
 * 
 */

#ifndef LEX_SCANNER_H
#define LEX_SCANNER_H

/**
 * @brief Stavy konečného automatu
 * 
 */
typedef enum State
{
    Start,
    Letter,
    Zero,
    Number,
    Hex,
    Exponent,
    Float,
    Exponent_sign,
    Exponent_float,
    BuiltIn,
    BuiltIn_dot,
    BuiltIn_fun,
    Divide,
    Lesser,
    Greater,
    Assign,
    Comment_single,
    Comment_multi,
    Comment_multi_end,
    Comment_multi_end_check,
    Comment_multi_slash,
    String_single,
    String_detect,
    String_q_check1,
    String_q_check2,
    String_multi,
    Id_global0,
    Id_global,
    Not,
    Error
} State;

/**
 * @brief Všechny typy tokenů
 * 
 */
typedef enum TokenType
{
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
    COMMA,
    EOL,
    T_EOF
} TokenType;

/**
 * @brief Struktura tokenu s načteným řčetězcem a jeho hodnotou
 * 
 * Uvolnění paměti má na starosti parser.
 * 
 */
typedef struct Token
{
    TokenType type;
    char *lexeme;
    union value
    {
        int int_val;
        double float_val;
        char *string_val;
    } value;
} Token;

/**
 * @brief Vrátí další token ze vstupu.
 *
 * Čte znaky ze standardního vstupu a vrací postupně jednotlivé tokeny.
 * Po dosažení konce vstupu vrací token typu T_EOF.
 */
Token ifj_get_token(void);


/**
 * @brief Vrátí aktuální číslo řádku.
 *
 * Řádek se zvyšuje při čtení znaků konce řádku v lexikálním analyzátoru.
 */
int ifj_get_line(void);


/**
 * @brief pomocná funkce pro vypsání tokenů
 * 
 * 
 * @param type Typ tokenu
 * @return char* 
 */
static inline const char* convert(TokenType type)
{
    switch (type)
    {
    case KEYWORD_class:
        return "KEYWORD_class";
    case KEYWORD_if:
        return "KEYWORD_if";
    case KEYWORD_else:
        return "KEYWORD_else";
    case KEYWORD_is:
        return "KEYWORD_is";
    case KEYWORD_null:
        return "KEYWORD_null";
    case KEYWORD_return:
        return "KEYWORD_return";
    case KEYWORD_var:
        return "KEYWORD_var";
    case KEYWORD_while:
        return "KEYWORD_while";
    case KEYWORD_Ifj:
        return "KEYWORD_Ifj";
    case KEYWORD_static:
        return "KEYWORD_static";
    case KEYWORD_import:
        return "KEYWORD_import";
    case KEYWORD_for:
        return "KEYWORD_for";
    case KEYWORD_Num:
        return "KEYWORD_Num";
    case KEYWORD_String:
        return "KEYWORD_String";
    case KEYWORD_Null:
        return "KEYWORD_Null";
    case IDENTIFIER_LOCAL:
        return "IDENTIFIER_LOCAL";
    case IDENTIFIER_GLOBAL:
        return "IDENTIFIER_GLOBAL";
    case INT:
        return "INT";
    case FLOAT:
        return "FLOAT";
    case STRING:
        return "STRING";
    case TIMES:
        return "TIMES";
    case DIVIDE:
        return "DIVIDE";
    case PLUS:
        return "PLUS";
    case MINUS:
        return "MINUS";
    case LESSER:
        return "LESSER";
    case GREATER:
        return "GREATER";
    case LESSER_EQUAL:
        return "LESSER_EQUAL";
    case GREATER_EQUAL:
        return "GREATER_EQUAL";
    case EQUAL:
        return "EQUAL";
    case NOT_EQUAL:
        return "NOT_EQUAL";
    case ASSIGN:
        return "ASSIGN";
    case L_ROUND:
        return "L_ROUND";
    case R_ROUND:
        return "R_ROUND";
    case L_CURLY:
        return "L_CURLY";
    case R_CURLY:
        return "R_CURLY";
    case IFJ_READ_STR:
        return "IFJ_READ_STR";
    case IFJ_READ_NUM:
        return "IFJ_READ_NUM";
    case IFJ_WRITE:
        return "IFJ_WRITE";
    case IFJ_FLOOR:
        return "IFJ_FLOOR";
    case IFJ_STR:
        return "IFJ_STR";
    case IFJ_LENGTH:
        return "IFJ_LENGTH";
    case IFJ_SUBSTRING:
        return "IFJ_SUBSTRING";
    case IFJ_STRCMP:
        return "IFJ_STRCMP";
    case IFJ_ORD:
        return "IFJ_ORD";
    case IFJ_CHR:
        return "IFJ_CHR";
    case COMMA:
        return "COMMA";
    case EOL:
        return "EOL";
    case T_EOF:
        return "T_EOF";
    }
    return "UNKNOWN";
}

#endif