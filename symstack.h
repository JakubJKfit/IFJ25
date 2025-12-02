/**
 * @name IFJ25
 * 
 * @file symstack.h
 * @author Jakub Jan Kupčík xkupcij00
 * @brief Hlavičkový soubor pro zásobník tabulek
 */

#ifndef SYMSTACK_H
#define SYMSTACK_H

#include "symtable.h"

/**
 * @brief uzel v zásobníku
 */
typedef struct stack_node{
    tree_node *symtable; // tabulka uzlu
    char *identifier; // identifikátor, pod kterým je uložená
    struct stack_node *next; // ukazatel na další uzel
}stack_node;

/**
 * @brief zásobník, ukazatel na vrchol zásobníku
 */
typedef struct{
    stack_node *top;
}symstack;

void stack_init(symstack *stack);
void stack_push(symstack *stack, char *identifier);
void stack_pop(symstack *stack);
tree_node **stack_top(symstack *stack);
void stack_dispose(symstack *stack);

#endif // SYMSTACK_H