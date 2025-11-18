#ifndef CODEGENHELP_H
#define CODEGENHELP_H

#include "ast.h"
#include "parser.h" 

/**
 * @brief Hlavní funkce pro generování kódu.
 * Projde AST a vygeneruje IFJcode25 na stdout.
 *
 * @param root Kořenový uzel AST (typu AST_PROGRAM).
 */
void generate_help(AstNode *root);

#endif 