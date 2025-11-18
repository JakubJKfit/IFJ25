/**
 * @file    ast_printer.h
 * @brief   Hlavičkový soubor pro modul tisku AST stromu
 */

#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "ast.h"

/**
 * @brief Vytiskne přehlednou reprezentaci AST stromu na stdout.
 * @param node Kořenový uzel stromu (typu AST_PROGRAM).
 */
void ast_print_program(AstNode *node);

#endif // AST_PRINTER_H