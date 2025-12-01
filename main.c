// main.c
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