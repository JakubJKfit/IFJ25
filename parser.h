/**
 * @file    parser.h
 * @brief   Hlavičkový soubor pro syntaktický analyzátor (parser)
 */

#ifndef IFJ_PARSER_H
#define IFJ_PARSER_H

#include <stdbool.h>
#include "ast.h" // Přidán include AST

// Entry point nyní vrací kořen AST nebo NULL při chybě
AstNode *ifj_parse_program(void);

// globální kód chyby
extern int ifj_error_code;

#endif // IFJ_PARSER_H