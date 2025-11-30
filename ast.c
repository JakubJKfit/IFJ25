/**
 * @file    ast.c
 * @brief   Implementace funkcí pro tvorbu a uvolnění uzlů AST
 */

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * @brief Alokuje a inicializuje AST uzel daného typu.
 *
 * Při chybě alokace ukončí program s návratovým kódem 99.
 *
 * @param type Typ uzlu.
 * @param size Velikost alokované struktury v bajtech.
 * @return Ukazatel na nově alokovaný uzel.
 */
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

/**
 * @brief Vytvoří prázdný seznam AST uzlů.
 *
 * Seznam má počáteční kapacitu 8 položek a podle potřeby se dynamicky zvětšuje.
 */
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

/**
 * @brief Přidá položku na konec seznamu AST uzlů.
 *
 * Pokud je kapacita naplněna, seznam se zvětší pomocí realloc().
 *
 * @param list Seznam, do kterého se přidává.
 * @param item Přidávaný uzel AST.
 */
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

// Výrazy

/**
 * @brief Vytvoří AST uzel pro literál (číslo, řetězec, null).
 *
 * @param token Token reprezentující literál.
 * @return Ukazatel na nově vytvořený uzel.
 */
AstNode *create_literal(Token token)
{
    AstNodeLiteral *node = (AstNodeLiteral *)ast_node_alloc(AST_EXPR_LITERAL, sizeof(AstNodeLiteral));
    node->token = token;
    node->data_type = Undefined;
    return (AstNode *)node;
}

/**
 * @brief Vytvoří AST uzel pro proměnnou.
 *
 * @param token Token s identifikátorem proměnné.
 * @return Ukazatel na nově vytvořený uzel.
 */
AstNode *create_variable(Token token)
{
    AstNodeVariable *node = (AstNodeVariable *)ast_node_alloc(AST_EXPR_VARIABLE, sizeof(AstNodeVariable));
    node->token = token;
    node->stack_offset = 0;
    node->data_type = Undefined;
    return (AstNode *)node;
}

/**
 * @brief Vytvoří AST uzel pro binární výraz.
 *
 * @param op    Typ operátoru (PLUS, MINUS, atd.).
 * @param left  Levý operand.
 * @param right Pravý operand.
 * @param line  Číslo řádku, na kterém se výraz nachází.
 * @return Ukazatel na nově vytvořený uzel.
 */
AstNode *create_binary_expr(TokenType op, AstNode *left, AstNode *right, int line)
{
    AstNodeBinaryExpr *node = (AstNodeBinaryExpr *)ast_node_alloc(AST_EXPR_BINARY, sizeof(AstNodeBinaryExpr));
    node->base.line_number = line;
    node->op = op;
    node->left = left;
    node->right = right;
    node->data_type = Undefined;
    return (AstNode *)node;
}

/**
 * @brief Vytvoří AST uzel pro volání funkce.
 *
 * @param func_id Token s identifikátorem funkce (nebo vestavěné IFJ funkce).
 * @param args    Seznam argumentů volání.
 * @return Ukazatel na nově vytvořený uzel.
 */
AstNode *create_func_call(Token func_id, AstNodeList *args)
{
    AstNodeFuncCall *node = (AstNodeFuncCall *)ast_node_alloc(AST_FUNC_CALL, sizeof(AstNodeFuncCall));
    node->func_id = func_id;
    node->args = args;
    node->data_type = Undefined;
    return (AstNode *)node;
}

// Příkazy

/**
 * @brief Vytvoří AST uzel pro blok příkazů.
 *
 * @param line Číslo řádku, na kterém blok začíná.
 * @return Ukazatel na nově vytvořený blok.
 */
AstNodeBlock *create_block(int line)
{
    AstNodeBlock *node = (AstNodeBlock *)ast_node_alloc(AST_STMT_BLOCK, sizeof(AstNodeBlock));
    node->base.line_number = line;
    node->statements = create_list();
    return node;
}

/**
 * @brief Přidá příkaz do bloku.
 *
 * @param block Blok, do kterého se příkaz přidává.
 * @param stmt  Přidávaný příkaz (AST uzel).
 */
void add_stmt_to_block(AstNodeBlock *block, AstNode *stmt)
{
    if (stmt)
    {
        add_to_list(block->statements, stmt);
    }
}

/**
 * @brief Vytvoří AST uzel pro příkaz if.
 *
 * @param cond    Podmínka.
 * @param then_b  Větev then.
 * @param else_b  Větev else (může být NULL).
 * @param line    Číslo řádku, na kterém příkaz začíná.
 * @return Ukazatel na nově vytvořený uzel.
 */
AstNode *create_if_stmt(AstNode *cond, AstNodeBlock *then_b, AstNodeBlock *else_b, int line)
{
    AstNodeIfStmt *node = (AstNodeIfStmt *)ast_node_alloc(AST_STMT_IF, sizeof(AstNodeIfStmt));
    node->base.line_number = line;
    node->condition = cond;
    node->then_block = then_b;
    node->else_block = else_b;
    return (AstNode *)node;
}

/**
 * @brief Vytvoří AST uzel pro příkaz while.
 *
 * @param cond Podmínka cyklu.
 * @param body Tělo cyklu.
 * @param line Číslo řádku, na kterém příkaz začíná.
 * @return Ukazatel na nově vytvořený uzel.
 */
AstNode *create_while_stmt(AstNode *cond, AstNodeBlock *body, int line)
{
    AstNodeWhileStmt *node = (AstNodeWhileStmt *)ast_node_alloc(AST_STMT_WHILE, sizeof(AstNodeWhileStmt));
    node->base.line_number = line;
    node->condition = cond;
    node->body_block = body;
    return (AstNode *)node;
}

/**
 * @brief Vytvoří AST uzel pro příkaz return.
 *
 * @param expr Vrácený výraz (může být NULL).
 * @param line Číslo řádku, na kterém příkaz začíná.
 * @return Ukazatel na nově vytvořený uzel.
 */
AstNode *create_return_stmt(AstNode *expr, int line)
{
    AstNodeReturnStmt *node = (AstNodeReturnStmt *)ast_node_alloc(AST_STMT_RETURN, sizeof(AstNodeReturnStmt));
    node->base.line_number = line;
    node->expr = expr;
    return (AstNode *)node;
}

/**
 * @brief Vytvoří AST uzel pro deklaraci proměnné.
 *
 * @param var_id Token s identifikátorem proměnné.
 * @return Ukazatel na nově vytvořený uzel.
 */
AstNode *create_vardecl(Token var_id)
{
    AstNodeVarDecl *node = (AstNodeVarDecl *)ast_node_alloc(AST_STMT_VAR_DECL, sizeof(AstNodeVarDecl));
    node->var_id = var_id;
    node->stack_offset = 0;
    node->data_type = Undefined;
    return (AstNode *)node;
}

/**
 * @brief Vytvoří AST uzel pro přiřazení.
 *
 * @param lvalue Levá strana (proměnná).
 * @param rvalue Pravá strana (výraz).
 * @param line   Číslo řádku, na kterém příkaz začíná.
 * @return Ukazatel na nově vytvořený uzel.
 */
AstNode *create_assign_stmt(AstNodeVariable *lvalue, AstNode *rvalue, int line)
{
    AstNodeAssignStmt *node = (AstNodeAssignStmt *)ast_node_alloc(AST_STMT_ASSIGN, sizeof(AstNodeAssignStmt));
    node->base.line_number = line;
    node->lvalue = lvalue;
    node->rvalue = rvalue;
    return (AstNode *)node;
}

// Top Level

/**
 * @brief Vytvoří AST uzel pro definici funkce.
 *
 * @param func_id Identifikátor funkce.
 * @param params  Seznam parametrů.
 * @param body    Tělo funkce.
 * @param kind    Druh funkce (běžná, getter, setter).
 * @return Ukazatel na nově vytvořený uzel.
 */
AstNodeFuncDef *create_func_def(Token func_id, AstNodeList *params, AstNodeBlock *body, AstFuncKind kind)
{
    AstNodeFuncDef *node = (AstNodeFuncDef *)ast_node_alloc(AST_FUNC_DEF, sizeof(AstNodeFuncDef));
    node->func_id = func_id;
    node->params = params;
    node->body = body;
    node->kind = kind;
    return node;
}

/**
 * @brief Vytvoří kořenový uzel programu.
 *
 * Kořen obsahuje seznam definic funkcí.
 *
 * @return Ukazatel na nově vytvořený uzel programu.
 */
AstNodeProgram *create_program(void)
{
    AstNodeProgram *node = (AstNodeProgram *)ast_node_alloc(AST_PROGRAM, sizeof(AstNodeProgram));
    node->functions = create_list();
    return node;
}

/**
 * @brief Přidá definici funkce do kořenového uzlu programu.
 *
 * @param prog Kořenový uzel programu.
 * @param func Definice funkce, která se má přidat.
 */
void add_func_to_program(AstNodeProgram *prog, AstNodeFuncDef *func)
{
    if (prog && func)
    {
        add_to_list(prog->functions, (AstNode *)func);
    }
}

/**
 * @brief Uvolní dynamicky alokované části tokenu (lexeme, string_val).
 *
 * @param t Ukazatel na token, jehož interní data se mají uvolnit.
 */
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

/**
 * @brief Uvolní položky seznamu a jeho dynamické pole.
 *
 * Samotná struktura AstNodeList je uvolněna ve free_ast_node(), aby se
 * zabránilo dvojímu uvolnění.
 *
 * @param list Seznam, jehož obsah se má uvolnit.
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
}

/**
 * @brief Rekurzivně uvolní uzel AST a všechny jeho potomky.
 *
 * Funkce korektně uvolní i vnořené struktury (bloky, seznamy, výrazy,
 * definice funkcí atd.) a všechny dynamicky alokované lexémy.
 *
 * @param node Kořen uvolňovaného podstromu (může být NULL).
 */
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

    free(node);
}
