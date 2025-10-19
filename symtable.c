// tabulka symbolu

#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int maximum(int a, int b){
    return (a > b ? a : b);
}

void tree_init(tree_node **tree){
    *tree = NULL;
}

int get_height(tree_node *node){
    if(node != NULL){
        return node->height;
    }
    return 0;
}

void put_height(tree_node *node){
    if(node != NULL){
        int left_h = get_height(node->left);
        int right_h = get_height(node->right);
        node->height = maximum(left_h, right_h) + 1;
    }
}

int get_balance(tree_node *node){
    if(node != NULL){
        return get_height(node->left) - get_height(node->right);
    }
    return 0;
}

void rotate_right(tree_node **tree){
    if(*tree == NULL || (*tree)->left == NULL) return;
    
    tree_node *old_root = *tree;
    tree_node *new_root = old_root->left;
    tree_node *temp = new_root->right;

    new_root->right = old_root;
    old_root->left = temp;

    put_height(old_root);
    put_height(new_root);

    *tree = new_root;
}
