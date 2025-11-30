/**
 * @file    parser.h
 * @brief   Hlavičkový soubor pro syntaktický analyzátor (parser)
 */

#ifndef IFJ_PARSER_H
#define IFJ_PARSER_H

#include <stdbool.h>
#include "ast.h" // Přidán include AST

/**
 * @brief Vstupní bod parseru pro jazyk IFJ25.
 *
 * Načte vstupní program, provede syntaktickou a sémantickou analýzu a
 * vytvoří strom AST. Při chybě volá ifjexit() a ukončí program.
 *
 * @return Ukazatel na kořenový uzel AST (AST_PROGRAM).
 */
AstNode *ifj_parse_program(void);

/**
 * @brief Globální kód chyby nastavovaný funkcí ifjexit().
 */
extern int ifj_error_code;

#endif // IFJ_PARSER_H