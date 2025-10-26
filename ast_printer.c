/**
 * @file    ast_printer.c
 * @brief   Implementace funkce pro tisk AST stromu na standardní výstup
 */

#include "ast_printer.h"
#include <stdio.h> // Pro printf

// Dopředná deklarace hlavní rekurzivní funkce
static void print_node(AstNode *node, int level);

/**
 * @brief Pomocná funkce pro tisk odsazení.
 */
static void print_indent(int level)
{
    printf("%*s", level * 2, ""); // 2 mezery na úroveň
}

/**
 * @brief Vytiskne lexém tokenu, pokud existuje.
 */
static void print_lexeme(Token *t)
{
    if (t->lexeme)
    {
        printf(" (%s)", t->lexeme);
    }
}

/**
 * @brief Vytiskne uzel seznamu (argumenty, parametry, příkazy).
 */
static void print_list(AstNodeList *list, int level, const char *name)
{
    print_indent(level);
    printf("[%s] (count: %d)\n", name, list->count);
    for (int i = 0; i < list->count; i++)
    {
        print_node(list->items[i], level + 1);
    }
}

/**
 * @brief Hlavní rekurzivní tisková funkce.
 */
static void print_node(AstNode *node, int level)
{
    if (node == NULL)
    {
        print_indent(level);
        printf("[NULL Node]\n");
        return;
    }

    switch (node->type)
    {
    case AST_PROGRAM:
    {
        AstNodeProgram *prog = (AstNodeProgram *)node;
        print_indent(level);
        printf("[Program]\n");
        print_list(prog->functions, level + 1, "Functions");
        break;
    }
    case AST_FUNC_DEF:
    {
        AstNodeFuncDef *func = (AstNodeFuncDef *)node;
        print_indent(level);
        printf("[FuncDef]");
        print_lexeme(&func->func_id);
        printf("\n");
        print_list(func->params, level + 1, "Params");
        print_node((AstNode *)func->body, level + 1);
        break;
    }
    case AST_STMT_BLOCK:
    {
        AstNodeBlock *block = (AstNodeBlock *)node;
        print_list(block->statements, level, "Block");
        break;
    }
    case AST_STMT_VAR_DECL:
    {
        AstNodeVarDecl *decl = (AstNodeVarDecl *)node;
        print_indent(level);
        printf("[VarDecl]");
        print_lexeme(&decl->var_id);
        printf("\n");
        break;
    }
    case AST_STMT_ASSIGN:
    {
        AstNodeAssignStmt *assign = (AstNodeAssignStmt *)node;
        print_indent(level);
        printf("[AssignStmt]\n");
        print_node((AstNode *)assign->lvalue, level + 1);
        print_node(assign->rvalue, level + 1);
        break;
    }
    case AST_EXPR_LITERAL:
    {
        AstNodeLiteral *lit = (AstNodeLiteral *)node;
        print_indent(level);
        printf("[Literal]");
        print_lexeme(&lit->token);
        printf("\n");
        break;
    }
    case AST_EXPR_VARIABLE:
    {
        AstNodeVariable *var = (AstNodeVariable *)node;
        print_indent(level);
        printf("[Variable]");
        print_lexeme(&var->token);
        printf("\n");
        break;
    }
    case AST_EXPR_BINARY:
    {
        AstNodeBinaryExpr *bin = (AstNodeBinaryExpr *)node;
        print_indent(level);
        printf("[BinaryExpr: %s]\n", convert(bin->op));
        print_node(bin->left, level + 1);
        print_node(bin->right, level + 1);
        break;
    }
    case AST_STMT_IF:
    {
        AstNodeIfStmt *if_stmt = (AstNodeIfStmt *)node;
        print_indent(level);
        printf("[IfStmt]\n");
        print_indent(level + 1);
        printf("(Condition)\n");
        print_node(if_stmt->condition, level + 2);
        print_indent(level + 1);
        printf("(Then)\n");
        print_node((AstNode *)if_stmt->then_block, level + 2);
        print_indent(level + 1);
        printf("(Else)\n");
        print_node((AstNode *)if_stmt->else_block, level + 2);
        break;
    }
    case AST_STMT_WHILE:
    {
        AstNodeWhileStmt *whl = (AstNodeWhileStmt *)node;
        print_indent(level);
        printf("[WhileStmt]\n");
        print_indent(level + 1);
        printf("(Condition)\n");
        print_node(whl->condition, level + 2);
        print_indent(level + 1);
        printf("(Body)\n");
        print_node((AstNode *)whl->body_block, level + 2);
        break;
    }
    case AST_STMT_RETURN:
    {
        AstNodeReturnStmt *ret = (AstNodeReturnStmt *)node;
        print_indent(level);
        printf("[ReturnStmt]\n");
        if (ret->expr)
        {
            print_node(ret->expr, level + 1);
        }
        break;
    }
    case AST_FUNC_CALL:
    {
        AstNodeFuncCall *call = (AstNodeFuncCall *)node;
        print_indent(level);
        printf("[FuncCall]");
        print_lexeme(&call->func_id);
        printf("\n");
        print_list(call->args, level + 1, "Args");
        break;
    }

    default:
        print_indent(level);
        printf("[Neznámý uzel: %d]\n", node->type);
    }
}

// Veřejná funkce, která vše odstartuje
void ast_print_program(AstNode *node)
{
    if (node == NULL || node->type != AST_PROGRAM)
    {
        printf("[Chyba: Kořenový uzel není AST_PROGRAM nebo je NULL]\n");
        return;
    }
    printf("--- Začátek AST ---\n");
    print_node(node, 0);
    printf("--- Konec AST ---\n");
}