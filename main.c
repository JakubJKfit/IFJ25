// main.c
#include <stdio.h>
#include <string.h>     
#include "parser.h"
#include "ast.h"
#include "ast_printer.h" 
#include "codegen.h"
#include "codegenhelp.h"

int main(int argc, char *argv[]) 
{
    int dump_ast = 0;
    
    if (argc == 2 && strcmp(argv[1], "--dump-ast") == 0)
    {
        dump_ast = 1;
    }

    int help_ast = 0;
    if (argc == 2 && strcmp(argv[1], "--help-ast") == 0)
    {
        help_ast = 1;
    }


    AstNode *ast_root = ifj_parse_program();

    if (ast_root != NULL)
    {
        if (dump_ast)
        {
            ast_print_program(ast_root);
        }
        else if (help_ast)
        {
            generate_help(ast_root);
            
        }
        else
        {
            
            generate_code(ast_root);
            
        }

        free_ast_node(ast_root);
        return 0;
    }


    // Obecná syntaktická chyba
    return 2;
}