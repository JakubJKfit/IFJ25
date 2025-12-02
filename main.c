/**
 * @name IFJ25
 * 
 * @file main.c
 * @author Jan Kostečka xkostej00
 * @author Jakub Jan Kupčík xkupcij00
 * @author Jáchym Turek xturekj01
 * @author Petr Molík xmolikp00
 * @brief Hlavičkový soubor pro tabulku symbolů
 */
#include <stdio.h>
#include <string.h>     
#include "parser.h"
#include "ast.h"
#include "codegen.h"


int main() 
{
    
    


    AstNode *ast_root = ifj_parse_program();

    if (ast_root != NULL)
    {
        

        generate_code(ast_root);


        free_ast_node(ast_root);
        return 0;
    }


    // Obecná syntaktická chyba
    return 2;
}