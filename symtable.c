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

void rotate_left(tree_node **tree){
    if(*tree == NULL || (*tree)->right == NULL) return;
    
    tree_node *old_root = *tree;
    tree_node *new_root = old_root->right;
    tree_node *temp = new_root->left;

    new_root->left = old_root;
    old_root->right = temp;

    put_height(old_root);
    put_height(new_root);

    *tree = new_root;
}

void balance_tree(tree_node **tree){
    if(*tree == NULL) return;

    put_height(*tree);
    int balance = get_balance(*tree);

    if(balance > 1){
        if(get_balance((*tree)->left) < 0){
            rotate_right(&(*tree)->left);
        }
        rotate_right(tree);
    }
    else if(balance < -1){
        if(get_balance((*tree)->right) > 0){
            rotate_right(&(*tree)->right);
        }
        rotate_left(tree);
    }
}

bool search_symbol(tree_node *node, char *identifier,symbol_data **found){
    if(node != NULL){
        if(strcmp(node->data.identifier, identifier) == 0){ // found
            *found = &node->data;
            return true;
        }

        int cmp = strcmp(node->data.identifier, identifier); // pruchod
        if (cmp > 0) {
            return search_symbol(node->left, identifier, found);
        }
        else
            return search_symbol(node->right, identifier, found);
    }

    return false;
}

void insert_symbol(tree_node **tree, symbol_data *data){
    if(*tree == NULL){
        *tree = (tree_node *)malloc(sizeof(tree_node));
        if(*tree == NULL){
            fprintf(stderr, "Malloc error for inserting symbol");
            exit(EXIT_FAILURE);
        }

        (*tree)->data.identifier = malloc(strlen(data->identifier) + 1);
        if ((*tree)->data.identifier == NULL){
            fprintf(stderr, "Malloc error for identifier for inserting symbol");
            free(*tree);
            exit(EXIT_FAILURE);
        }
        (*tree)->left = NULL;
        (*tree)->right = NULL;
        (*tree)->height = 1;

        strcpy((*tree)->data.identifier, data->identifier);

        return;
    }

    int cmp = strcmp(data->identifier, (*tree)->data.identifier);
    if(cmp < 0){
        insert_symbol(&(*tree)->left, data);
    }else if(cmp > 0){
        insert_symbol(&(*tree)->right, data);
    }
    else // jiz existuje
        return;
    put_height(*tree);
    balance_tree(tree);
}

void replace_by_rightmost(tree_node *target, tree_node **tree){
    if (*tree != NULL){
        if ((*tree)->right == NULL){
            free(target->data.identifier);
            target->data.identifier = malloc(strlen((*tree)->data.identifier) + 1);
            if(target->data.identifier == NULL){
                fprintf(stderr, "Malloc error for replace by rightmost");
                exit(EXIT_FAILURE);
            }
            strcpy(target->data.identifier, (*tree)->data.identifier);

            tree_node *to_delete = *tree;
            *tree = (*tree)->left;
            free(to_delete);
        }
        else
            replace_by_rightmost(target, &(*tree)->right);
    }
}

void delete_symbol(tree_node **tree, char *identifier){
    if (*tree == NULL) return;

    int cmp = strcmp(identifier, (*tree)->data.identifier);
    if(cmp < 0){
        delete_symbol(&(*tree)->left, identifier);
    }else if(cmp > 0){
        delete_symbol(&(*tree)->right, identifier);
    }else{
        tree_node *to_delete = *tree;
        if ((*tree)->left == NULL){
            *tree = (*tree)->right;
            free(to_delete->data.identifier);
            free(to_delete);
        }else if((*tree)->right == NULL){
            *tree = (*tree)->left;
            free(to_delete->data.identifier);
            free(to_delete);
        }else{
            replace_by_rightmost(*tree, &(*tree)->left);
        }
    }
    if(*tree != NULL){
        put_height(*tree);
        balance_tree(tree);
    }
}

void tree_dispose(tree_node **tree){
    if(*tree != NULL){
        tree_dispose(&(*tree)->left);
        tree_dispose(&(*tree)->right);
        free((*tree)->data.identifier);
        free(*tree);
        *tree = NULL;
    }
}