

#ifndef SYMSTACK_H
#define SYMSTACK_H

#include "symtable.h"

typedef struct stack_node{
    tree_node *symtable;
    char *identifier;
    struct stack_node *next;
}stack_node;

typedef struct{
    stack_node *top;
}symstack;

void stack_init(symstack *stack);
void stack_push(symstack *stack, char *identifier);
void stack_pop(symstack *stack);
tree_node **stack_top(symstack *stack);
void stack_dispose(symstack *stack);

#endif // SYMSTACK_H