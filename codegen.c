#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <assert.h>

#include "codegen.h"
#include "ast.h"
#include "parser.h" 
#include "err.h"    


void generate_code(AstNode *node){
    printf(".IFJcode25\n");
    printf("JUMP $main\n");
    printf("%d", node->type);
    return;
}
