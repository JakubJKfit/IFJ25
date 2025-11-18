#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <assert.h>

#include "codegenhelp.h"
#include "ast.h"
#include "parser.h" 
#include "err.h"    


static void traverse_and_print_ast(AstNode *node, int indent);



/** @brief Vytiskne odsazení pro hezkou strukturu stromu */
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  "); 
    }
}

/** @brief Převede enum AstNodeType na čitelný řetězec */
static const char* get_node_type_name(AstNodeType type) {
    switch (type) {
        case AST_PROGRAM:       return "AST_PROGRAM";
        case AST_FUNC_DEF:      return "AST_FUNC_DEF";
        case AST_STMT_BLOCK:    return "AST_STMT_BLOCK";
        case AST_STMT_VAR_DECL: return "AST_STMT_VAR_DECL";
        case AST_STMT_ASSIGN:   return "AST_STMT_ASSIGN";
        case AST_STMT_IF:       return "AST_STMT_IF";
        case AST_STMT_WHILE:    return "AST_STMT_WHILE";
        case AST_STMT_RETURN:   return "AST_STMT_RETURN";
        case AST_FUNC_CALL:     return "AST_FUNC_CALL";
        case AST_EXPR_BINARY:   return "AST_EXPR_BINARY";
        case AST_EXPR_LITERAL:  return "AST_EXPR_LITERAL";
        case AST_EXPR_VARIABLE: return "AST_EXPR_VARIABLE";
        case AST_NODE_LIST:     return "AST_NODE_LIST";
        default:                return "AST_UNKNOWN";
    }
}


/**
 * @brief Rekurzivně projde strom a vypíše typ každého uzlu.
 */
static void traverse_and_print_ast(AstNode *node, int indent) {
    if (!node) {
        return;
    }


    print_indent(indent);
    printf("[%s]\n", get_node_type_name(node->type));


    switch (node->type) {
        case AST_PROGRAM: {
            AstNodeProgram *prog = (AstNodeProgram *)node;
            traverse_and_print_ast((AstNode *)prog->functions, indent + 1);
            break;
        }
        case AST_FUNC_DEF: {
            AstNodeFuncDef *func = (AstNodeFuncDef *)node;
            traverse_and_print_ast((AstNode *)func->params, indent + 1);
            traverse_and_print_ast((AstNode *)func->body, indent + 1);
            break;
        }
        case AST_STMT_BLOCK: {
            AstNodeBlock *block = (AstNodeBlock *)node;
            traverse_and_print_ast((AstNode *)block->statements, indent + 1);
            break;
        }
        case AST_STMT_ASSIGN: {
            AstNodeAssignStmt *stmt = (AstNodeAssignStmt *)node;
            traverse_and_print_ast((AstNode *)stmt->lvalue, indent + 1);
            traverse_and_print_ast(stmt->rvalue, indent + 1);
            break;
        }
        case AST_STMT_IF: {
            AstNodeIfStmt *stmt = (AstNodeIfStmt *)node;
            traverse_and_print_ast(stmt->condition, indent + 1);
            traverse_and_print_ast((AstNode *)stmt->then_block, indent + 1);
            traverse_and_print_ast((AstNode *)stmt->else_block, indent + 1); 
            break;
        }
        case AST_STMT_WHILE: {
            AstNodeWhileStmt *stmt = (AstNodeWhileStmt *)node;
            traverse_and_print_ast(stmt->condition, indent + 1);
            traverse_and_print_ast((AstNode *)stmt->body_block, indent + 1);
            break;
        }
        case AST_STMT_RETURN: {
            AstNodeReturnStmt *stmt = (AstNodeReturnStmt *)node;
            traverse_and_print_ast(stmt->expr, indent + 1); 
            break;
        }
        case AST_FUNC_CALL: {
            AstNodeFuncCall *call = (AstNodeFuncCall *)node;
            traverse_and_print_ast((AstNode *)call->args, indent + 1);
            break;
        }
        case AST_EXPR_BINARY: {
            AstNodeBinaryExpr *expr = (AstNodeBinaryExpr *)node;
            traverse_and_print_ast(expr->left, indent + 1);
            traverse_and_print_ast(expr->right, indent + 1);
            break;
        }
        case AST_NODE_LIST: {
            AstNodeList *list = (AstNodeList *)node;

            for (int i = 0; i < list->count; i++) {
          
                traverse_and_print_ast(list->items[i], indent + 1);
            }
            break;
        }

        
        case AST_STMT_VAR_DECL:
        case AST_EXPR_LITERAL:{
            
            break;
        }
        case AST_EXPR_VARIABLE:
            
            break;
        
        default:
           
            break;
    }
}


/**
 * @brief Hlavní funkce pro generování kódu.
 * Nyní jen spustí rekurzivní tisk.
 */
void generate_help(AstNode *root) {
    if (!root) {
        fprintf(stderr, "AST je prázdný, nelze tisknout.\n");
        return;
    }

    printf("--- 🌳 Začátek AST Traversal 🌳 ---\n");

    traverse_and_print_ast(root, 0);
    
    printf("--- 🌳 Konec AST Traversal 🌳 ---\n");
}