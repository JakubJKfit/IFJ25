/**
 * @file symstack.c
 * @author Jakub Jan Kupčík xkupcij00
 * @brief Zásobník tabulek určený k vytvoření nové tabulky symbolů pro vnořené bloky
 */

#include "symtable.h"
#include "symstack.h"
#include "err.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Alokování paměti pro identifiátor
 * 
 * @param node ukazatel na uzel
 * @param identifier identifikátor
 * @return void
 */
static void alloc_identif(stack_node *node, char *identifier){
    node->identifier = malloc(strlen(identifier) + 1);
    if(node->identifier == NULL){
        fprintf(stderr, "Stack indentifier malloc error\n");
        ifjexit(ERR_INTERNAL);
    }
    strcpy(node->identifier, identifier);
}

/**
 * @brief Inicializace zásobníku
 * @param stack ukazatel na zásobník
 * @return void
 */
void stack_init(symstack *stack){
    stack->top = NULL;
}

/**
 * @brief Vložení nového uzlu do zásobníku
 * @param stack ukazatel na zásobník
 * @param identifier identifikátor
 * @return void
 */
void stack_push(symstack *stack, char *identifier){
    stack_node *new_node = (stack_node *)malloc(sizeof(stack_node));
    if(new_node == NULL){
        fprintf(stderr, "Stack malloc error\n");
        ifjexit(ERR_INTERNAL);
    }

    new_node->symtable = NULL; // nová prázdná tabulka
    new_node->next = stack->top; // starý vrchol bude následovat po nově uloženém uzlu
    if(identifier != NULL) alloc_identif(new_node, identifier);
    else new_node->identifier = NULL;

    stack->top = new_node; // nový uzel je vrchol zásobníku
}

/**
 * @brief Odstranění vrcholu zásobníku a určení nového vrcholu
 * 
 * @param stack ukazatel na stack
 * @return void
 */
void stack_pop(symstack *stack){
    if(stack->top != NULL){ // zasobnik nesmi byt prazdny
        stack_node *top_node = stack->top;
        stack->top = top_node->next; // druhy z vrcholu bude novy vrchol
        tree_dispose(&(top_node->symtable)); // uklizeni tabulky smazaneho vrcholu
        if(top_node->identifier != NULL) free(top_node->identifier);
        free(top_node);
    }
}

/**
 * @brief Vracení ukazatele na kořen tabulky vrcholu zásobníku.
 * @param stack ukazatel na zásobník
 * @return tree_node adresa ukazatele na kořen tabulky vrcholu zásobníku
 */
tree_node **stack_top(symstack *stack){
    if(stack->top == NULL){
        fprintf(stderr, "Stack is empty!\n");
        ifjexit(ERR_INTERNAL);
    }
    return &(stack->top->symtable);
}

/**
 * @brief Smazání zásobníku
 * @param stack ukazatel na zásobník
 * @return void
 */
void stack_dispose(symstack *stack){
    while(stack->top != NULL){
        stack_pop(stack);
    }
}