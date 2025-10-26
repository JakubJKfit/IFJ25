// stack tabulek symbolu pro kontrolu urovne vnoreni

#include "symtable.h"
#include "symstack.h"
#include <stdlib.h>
#include <stdio.h>

static void alloc_identif(stack_node *node, char *identifier){
    node->identifier = malloc(strlen(identifier) + 1);
    if(node->identifier == NULL){
        fprintf(stderr, "Stack indentifier malloc error\n");
        exit(ERR_INTERNAL);
    }
    strcpy(node->identifier, identifier);
}

void stack_init(symstack *stack){
    stack->top = NULL;
}

void stack_push(symstack *stack, char *identifier){
    stack_node *new_node = (stack_node *)malloc(sizeof(stack_node));
    if(new_node == NULL){
        fprintf(stderr, "Stack malloc error\n");
        exit(ERR_INTERNAL);
    }

    new_node->symtable = NULL;
    new_node->next = stack->top;
    if(identifier != NULL) alloc_identif(new_node, identifier);
    else new_node->identifier = NULL;

    stack->top = new_node;
}

void stack_pop(symstack *stack){
    if(stack->top != NULL){
        stack_node *top_node = stack->top;
        stack->top = top_node->next;
        tree_dispose(&(top_node->symtable));
        if(top_node->identifier != NULL) free(top_node->identifier);
        free(top_node);
    }
}

tree_node **stack_top(symstack *stack){
    if(stack->top == NULL){
        fprintf(stderr, "Stack is empty!\n");
        exit(ERR_INTERNAL);
    }
    return &(stack->top->symtable);
}

void stack_dispose(symstack *stack){
    while(stack->top != NULL){
        stack_pop(stack);
    }
}