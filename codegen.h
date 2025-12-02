/**
 * @name IFJ25
 * 
 * @file codegen.h
 * @author Jáchym Turek xturekj01
 * @brief generování cílového kódu IFJcode25
 * 
 * Hlavičkový soubor pro generátor kódu.
 */
#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "parser.h" 


void generate_code(AstNode *root);

#endif 