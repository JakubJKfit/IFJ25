/**
 * @file    ast.c
 * @brief   Implementace funkcí pro tvorbu a uvolnění uzlů AST
 */

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// --- Helper pro alokaci ---
static AstNode *ast_node_alloc(AstNodeType type, size_t size)
{
    AstNode *node = calloc(1, size);
    if (!node)
    {
        fprintf(stderr, "FATAL: AST node allocation failed\n");
        exit(99);
    }
    node->type = type;
    node->line_number = ifj_get_line(); // Automaticky vezme aktuální řádek
    return node;
}

// --- Seznamy ---
AstNodeList *create_list(void)
{
    AstNodeList *list = (AstNodeList *)ast_node_alloc(AST_NODE_LIST, sizeof(AstNodeList));
    list->capacity = 8;
    list->count = 0;
    list->items = malloc(list->capacity * sizeof(AstNode *));
    if (!list->items)
    {
        fprintf(stderr, "FATAL: AST list allocation failed\n");
        exit(99);
    }
    return list;
}

void add_to_list(AstNodeList *list, AstNode *item)
{
    if (list->count >= list->capacity)
    {
        list->capacity *= 2;
        list->items = realloc(list->items, list->capacity * sizeof(AstNode *));
        if (!list->items)
        {
            fprintf(stderr, "FATAL: AST list realloc failed\n");
            exit(99);
        }
    }
    list->items[list->count++] = item;
}

// --- Výrazy ---
AstNode *create_literal(Token token)
{
    AstNodeLiteral *node = (AstNodeLiteral *)ast_node_alloc(AST_EXPR_LITERAL, sizeof(AstNodeLiteral));
    node->token = token; // Kopíruje strukturu, vč. lexeme (pokud byl alokován)
    return (AstNode *)node;
}

AstNode *create_variable(Token token)
{
    AstNodeVariable *node = (AstNodeVariable *)ast_node_alloc(AST_EXPR_VARIABLE, sizeof(AstNodeVariable));
    node->token = token; // Kopíruje strukturu

    node->stack_offset = 0;
    node->data_type = Undefined;
    return (AstNode *)node;
}

AstNode *create_binary_expr(TokenType op, AstNode *left, AstNode *right, int line)
{
    AstNodeBinaryExpr *node = (AstNodeBinaryExpr *)ast_node_alloc(AST_EXPR_BINARY, sizeof(AstNodeBinaryExpr));
    node->base.line_number = line;
    node->op = op;
    node->left = left;
    node->right = right;
    return (AstNode *)node;
}

AstNode *create_func_call(Token func_id, AstNodeList *args)
{
    AstNodeFuncCall *node = (AstNodeFuncCall *)ast_node_alloc(AST_FUNC_CALL, sizeof(AstNodeFuncCall));
    node->func_id = func_id;
    node->args = args;
    return (AstNode *)node;
}

// --- Příkazy ---
AstNodeBlock *create_block(int line)
{
    AstNodeBlock *node = (AstNodeBlock *)ast_node_alloc(AST_STMT_BLOCK, sizeof(AstNodeBlock));
    node->base.line_number = line;
    node->statements = create_list();
    return node;
}

void add_stmt_to_block(AstNodeBlock *block, AstNode *stmt)
{
    if (stmt)
    {
        add_to_list(block->statements, stmt);
    }
}

AstNode *create_if_stmt(AstNode *cond, AstNodeBlock *then_b, AstNodeBlock *else_b, int line)
{
    AstNodeIfStmt *node = (AstNodeIfStmt *)ast_node_alloc(AST_STMT_IF, sizeof(AstNodeIfStmt));
    node->base.line_number = line;
    node->condition = cond;
    node->then_block = then_b;
    node->else_block = else_b;
    return (AstNode *)node;
}

AstNode *create_while_stmt(AstNode *cond, AstNodeBlock *body, int line)
{
    AstNodeWhileStmt *node = (AstNodeWhileStmt *)ast_node_alloc(AST_STMT_WHILE, sizeof(AstNodeWhileStmt));
    node->base.line_number = line;
    node->condition = cond;
    node->body_block = body;
    return (AstNode *)node;
}

AstNode *create_return_stmt(AstNode *expr, int line)
{
    AstNodeReturnStmt *node = (AstNodeReturnStmt *)ast_node_alloc(AST_STMT_RETURN, sizeof(AstNodeReturnStmt));
    node->base.line_number = line;
    node->expr = expr; // Může být NULL
    return (AstNode *)node;
}

AstNode *create_vardecl(Token var_id)
{
    AstNodeVarDecl *node = (AstNodeVarDecl *)ast_node_alloc(AST_STMT_VAR_DECL, sizeof(AstNodeVarDecl));
    node->var_id = var_id;

    node->stack_offset = 0;
    node->data_type = Undefined;
    return (AstNode *)node;
}

AstNode *create_assign_stmt(AstNodeVariable *lvalue, AstNode *rvalue, int line)
{
    AstNodeAssignStmt *node = (AstNodeAssignStmt *)ast_node_alloc(AST_STMT_ASSIGN, sizeof(AstNodeAssignStmt));
    node->base.line_number = line;
    node->lvalue = lvalue;
    node->rvalue = rvalue;
    return (AstNode *)node;
}

// --- Top Level ---
AstNodeFuncDef *create_func_def(Token func_id, AstNodeList *params, AstNodeBlock *body, AstFuncKind kind)
{
    AstNodeFuncDef *node = (AstNodeFuncDef *)ast_node_alloc(AST_FUNC_DEF, sizeof(AstNodeFuncDef));
    node->func_id = func_id;
    node->params = params;
    node->body = body;
    node->kind = kind;
    return node;
}

AstNodeProgram *create_program(void)
{
    AstNodeProgram *node = (AstNodeProgram *)ast_node_alloc(AST_PROGRAM, sizeof(AstNodeProgram));
    node->functions = create_list();
    return node;
}

void add_func_to_program(AstNodeProgram *prog, AstNodeFuncDef *func)
{
    if (prog && func)
    {
        add_to_list(prog->functions, (AstNode *)func);
    }
}

// Funkce pro uvolnění lexému
void free_token_lexeme(Token *t)
{
    if (t->lexeme)
    {
        free(t->lexeme);
        t->lexeme = NULL;
    }
    if (t->type == STRING && t->value.string_val)
    {
        free(t->value.string_val);
        t->value.string_val = NULL;
    }
}

// --- Uvolnění paměti (rekurzivní) ---

/**
 * @brief Uvolní POUZE OBSAH seznamu (položky a pole),
 * NE uvolnění samotné struktury seznamu (ta je uvolněna v free_ast_node),
 * aby se zabránilo double free chybám.
 */
static void free_list_contents(AstNodeList *list)
{
    if (!list)
        return;
    for (int i = 0; i < list->count; i++)
    {
        free_ast_node(list->items[i]);
    }
    free(list->items);
    // free(list); <-- TENTO ŘÁDEK JE ZÁMĚRNĚ SMAZÁN, ABY SE ZABRÁNILO DOUBLE FREE
}

void free_ast_node(AstNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case AST_PROGRAM:
        free_list_contents(((AstNodeProgram *)node)->functions);
        break;
    case AST_FUNC_DEF:
    {
        AstNodeFuncDef *n = (AstNodeFuncDef *)node;
        free_token_lexeme(&n->func_id);
        free_list_contents(n->params);
        free_ast_node((AstNode *)n->body);
        break;
    }
    case AST_FUNC_CALL:
    {
        AstNodeFuncCall *n = (AstNodeFuncCall *)node;
        free_token_lexeme(&n->func_id);
        free_list_contents(n->args);
        break;
    }
    case AST_STMT_BLOCK:
        free_list_contents(((AstNodeBlock *)node)->statements);
        break;
    case AST_STMT_VAR_DECL:
        free_token_lexeme(&((AstNodeVarDecl *)node)->var_id);
        break;
    case AST_STMT_ASSIGN:
        free_ast_node((AstNode *)((AstNodeAssignStmt *)node)->lvalue);
        free_ast_node(((AstNodeAssignStmt *)node)->rvalue);
        break;
    case AST_STMT_IF:
        free_ast_node(((AstNodeIfStmt *)node)->condition);
        free_ast_node((AstNode *)((AstNodeIfStmt *)node)->then_block);
        free_ast_node((AstNode *)((AstNodeIfStmt *)node)->else_block);
        break;
    case AST_STMT_WHILE:
        free_ast_node(((AstNodeWhileStmt *)node)->condition);
        free_ast_node((AstNode *)((AstNodeWhileStmt *)node)->body_block);
        break;
    case AST_STMT_RETURN:
        free_ast_node(((AstNodeReturnStmt *)node)->expr);
        break;
    case AST_EXPR_BINARY:
        free_ast_node(((AstNodeBinaryExpr *)node)->left);
        free_ast_node(((AstNodeBinaryExpr *)node)->right);
        break;
    case AST_EXPR_LITERAL:
        free_token_lexeme(&((AstNodeLiteral *)node)->token);
        break;
    case AST_EXPR_VARIABLE:
        free_token_lexeme(&((AstNodeVariable *)node)->token);
        break;
    case AST_NODE_LIST:
        free_list_contents((AstNodeList *)node);
        break;
    }
    // Nakonec uvolníme samotný uzel (včetně struktur jako AstNodeList)
    free(node);
}