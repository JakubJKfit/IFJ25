#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "parser.h" 

/**
 * @brief Vstupní funkce pro generování kódu z AST
 * volání z main.c.
 * Nejprve spustí první průchod pro sběr globálních proměných, potom se vypíšou globální proměnné, pomocné proměnné a vestavěné funkce.
 * Nakonec se provede hlavní průchod AST pro generování kódu.
 * 
 * @param root Kořen AST
 */
void generate_code(AstNode *root);

#endif 