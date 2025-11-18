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
    // Zjistíme, jestli chceme tisknout AST
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
            // Pokud je přepínač aktivní, vytiskneme AST
            ast_print_program(ast_root);
        }
        else if (help_ast)
        {
            // Pokud je přepínač aktivní, vytiskneme pomocnou informaci o AST
            generate_help(ast_root);
            
        }
        else
        {
            // Jinak vypíšeme jen tichý úspěch (pro test.py)
            // Můžeš smazat i ten printf, aby byl program úplně tichý
            generate_code(ast_root);
            
        }

        free_ast_node(ast_root);
        return 0;
    }


    // Obecná syntaktická chyba
    return 2;
}