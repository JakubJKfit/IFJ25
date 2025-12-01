/**
 * @name IFJ25
 * 
 * @file codegen.c
 * @author Jáchym Turek xturekj01
 * @brief generování cílového kódu IFJcode25
 * 
 * Soubor obsahuje funkce pro generování kódu z AST. Generovaný kód je vypsán na standardní výstup.
 * Prochází strom a pro každý uzel generuje odpovídající unstrukce IFJcode25.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <assert.h>

#include "codegen.h"
#include "ast.h"
#include "parser.h"
#include "err.h"  

/// Pole pro uložení globálních proměnných
char global_vars[128][256];
/// Počet globálních proměnných
int counter = 0;
/// Čítač pro generování unikátních návěstí (if, while...)
int if_label_counter = 0;


static void traverse_and_print_ast(AstNode *node);



/**
 * @brief Vypíše řetězec ve formátu pro IFJcode25
 *  Funkce projde vstupní řetězec a znaky, které nejsou tisknutelné nebo mají
 * IFJcode25 speciální význam (např. mezera, #, \), nahradí escape sekvencí \xyz.
 * @param str Vstupní řetězec
 */
static void print_string_literal_escaped(const char *str) {
    
    printf("string@");
    

    if (!str) {
        return;
    }


    for (int i = 0; str[i] != '\0'; i++) {

        unsigned char c = str[i];

        
        if (c <= 32 || c == 35 || c == 92) {

            printf("\\%03d", c);
        } else {

            putchar(c);
        }
    }
}


/**
 * @brief Generuje kontrolu na nil hodnoty pro binární operace
 */
void gen_nil_check() {
    int label_id = if_label_counter++;
    printf("POPS GF@_check_op_2\n"); 
    printf("POPS GF@_check_op_1\n"); 
    printf("TYPE GF@_check_type GF@_check_op_1\n");
    printf("JUMPIFEQ $nil_err_%d GF@_check_type string@nil\n", label_id);
    printf("TYPE GF@_check_type GF@_check_op_2\n");
    printf("JUMPIFEQ $nil_err_%d GF@_check_type string@nil\n", label_id);
    printf("PUSHS GF@_check_op_1\n");
    printf("PUSHS GF@_check_op_2\n");
    printf("JUMP $nil_ok_%d\n", label_id);
    printf("LABEL $nil_err_%d\n", label_id);
    printf("EXIT int@26\n");
    printf("LABEL $nil_ok_%d\n", label_id);
}



/**
 * @brief Rekurzivně projde strom a generuje kód.
 * Prochází AST a pro každý typ uzlu generuje odpovídající kód IFJcode25.
 * @param node Aktuální uzel AST
 */
static void traverse_and_print_ast(AstNode *node) {

    if (!node) {
        return;
    }


    switch (node->type) {
        case AST_PROGRAM: {
            AstNodeProgram *prog = (AstNodeProgram *)node;
            traverse_and_print_ast((AstNode *)prog->functions);
            break;
        }
        case AST_FUNC_DEF: {
            AstNodeFuncDef *func = (AstNodeFuncDef *)node;
            
            
            if(strcmp(func->func_id.lexeme, "main") == 0){
                printf("LABEL main\n");
                printf("CREATEFRAME\n");
                printf("PUSHFRAME\n");
                

                traverse_and_print_ast((AstNode *)func->body);
                printf("POPFRAME\n");
                printf("EXIT int@0\n");
            }else{
                
                switch(func->kind){
                    case FUNC_IS_FUNC:
                        printf("LABEL %s\n", func->func_id.lexeme);
                        break;
                    case FUNC_IS_GETTER:
           
                        printf("LABEL %s_GET\n", func->func_id.lexeme);
                        break;
                    case FUNC_IS_SETTER:
                
                        printf("LABEL %s_SET\n", func->func_id.lexeme);
                        break;
                }
                printf("PUSHFRAME\n");
                printf("DEFVAR LF@%%retval\n");
                printf("MOVE LF@%%retval nil@nil\n");
                for (int i = 0; i < func->params->count; i++) {
      
                    AstNodeVariable *param = (AstNodeVariable *)func->params->items[i];
                    
      
                    printf("DEFVAR LF@%s_%d\n", param->token.lexeme, abs(param->stack_offset));
                    
                  
                    printf("MOVE LF@%s_%d LF@%%param_%d\n", param->token.lexeme, abs(param->stack_offset), i + 1);
                }
           
                traverse_and_print_ast((AstNode *)func->body);
                printf("POPFRAME\n");
                printf("RETURN\n");
            }

            
            break;
        }
        case AST_STMT_BLOCK: {
            AstNodeBlock *block = (AstNodeBlock *)node;
            traverse_and_print_ast((AstNode *)block->statements);
            break;
        }
        case AST_STMT_ASSIGN: {
            AstNodeAssignStmt *stmt = (AstNodeAssignStmt *)node;
     
            traverse_and_print_ast(stmt->rvalue);
            if (stmt->lvalue->stack_offset < -1) {
           
                printf("POPS LF@%s_%d\n", stmt->lvalue->token.lexeme, abs(stmt->lvalue->stack_offset));
                
            } else if (stmt->lvalue->stack_offset == -1) {
             
                printf("POPS GF@%s\n", stmt->lvalue->token.lexeme);
        
            }else if(stmt->lvalue->stack_offset == 0){
      
                
                printf("CREATEFRAME\n");
                printf("DEFVAR TF@%%param_1\n");
                printf("POPS TF@%%param_1\n");
                printf("CALL %s_SET\n", stmt->lvalue->token.lexeme);
            }
            break;
        }
        case AST_STMT_IF: {
            AstNodeIfStmt *stmt = (AstNodeIfStmt *)node;
            traverse_and_print_ast(stmt->condition);
            int current_if_label = if_label_counter;
            if_label_counter++;

            printf("POPS GF@_cond_result\n");

            printf("TYPE GF@_cond_type GF@_cond_result\n");

            printf("JUMPIFEQ $if_then_%d GF@_cond_type string@int\n", current_if_label);
            printf("JUMPIFEQ $if_then_%d GF@_cond_type string@float\n", current_if_label);
            printf("JUMPIFEQ $if_then_%d GF@_cond_type string@string\n", current_if_label);

            printf("JUMPIFNEQ $if_else_%d GF@_cond_type string@bool\n", current_if_label);
            

            printf("JUMPIFEQ $if_then_%d GF@_cond_result bool@true\n", current_if_label);

            printf("LABEL $if_else_%d\n", current_if_label);
            traverse_and_print_ast((AstNode *)stmt->else_block);
            printf("JUMP $if_end_%d\n", current_if_label);

      
            printf("LABEL $if_then_%d\n", current_if_label);
            traverse_and_print_ast((AstNode *)stmt->then_block);


            printf("LABEL $if_end_%d\n", current_if_label);
            
            break;
        }
        case AST_STMT_WHILE: {
            
            AstNodeWhileStmt *stmt = (AstNodeWhileStmt *)node;

            int current_while_id = if_label_counter++;

            printf("LABEL $while_cond_%d\n", current_while_id);

            traverse_and_print_ast(stmt->condition);

            printf("POPS GF@_cond_result\n");
            printf("TYPE GF@_cond_type GF@_cond_result\n");

            printf("JUMPIFEQ $while_body_%d GF@_cond_type string@int\n", current_while_id);
            printf("JUMPIFEQ $while_body_%d GF@_cond_type string@float\n", current_while_id);
            printf("JUMPIFEQ $while_body_%d GF@_cond_type string@string\n", current_while_id);

            printf("JUMPIFEQ $while_check_bool_%d GF@_cond_type string@bool\n", current_while_id);
            printf("JUMP $while_end_%d\n", current_while_id);
            printf("LABEL $while_check_bool_%d\n", current_while_id);
            
            printf("JUMPIFEQ $while_body_%d GF@_cond_result bool@true\n", current_while_id);

            printf("JUMP $while_end_%d\n", current_while_id);

            
            printf("LABEL $while_body_%d\n", current_while_id);
            traverse_and_print_ast((AstNode *)stmt->body_block);
         
            printf("JUMP $while_cond_%d\n", current_while_id);
            printf("LABEL $while_end_%d\n", current_while_id);
            
            break;
        }
        case AST_STMT_RETURN: {
            AstNodeReturnStmt *stmt = (AstNodeReturnStmt *)node;
            
            if(stmt->expr != NULL){
                traverse_and_print_ast(stmt->expr);
                printf("POPS LF@%%retval\n");
            }
            break;
        }
        case AST_FUNC_CALL: {
            AstNodeFuncCall *call = (AstNodeFuncCall *)node;

            traverse_and_print_ast((AstNode *)call->args);

            printf("CREATEFRAME\n");

            for(int i = call->args->count - 1; i >= 0; i--){
                printf("DEFVAR TF@%%param_%d\n", i + 1);
                printf("POPS TF@%%param_%d\n", i + 1);
                
            }
            
            
            if(call->func_id.type == IDENTIFIER_LOCAL){
                
                printf("CALL %s\n", call->func_id.lexeme);
                
            }else{
                
                printf("CALL %s\n", convert(call->func_id.type));
            }

            printf("PUSHS TF@%%retval\n");
            
            
            break;
        }
        case AST_EXPR_BINARY: {
            AstNodeBinaryExpr *expr = (AstNodeBinaryExpr *)node;
            
            
            traverse_and_print_ast(expr->left);
            traverse_and_print_ast(expr->right);
            switch (expr->op) {
        
                case PLUS:
                    gen_nil_check();
                    
                    if (expr->data_type == String) {
                        printf("POPS GF@_concat_help_2\n");
                        printf("POPS GF@_concat_help_1\n");
                        printf("CONCAT GF@_concat_help_1 GF@_concat_help_1 GF@_concat_help_2\n");
                        printf("PUSHS GF@_concat_help_1\n");
                    } else { 
                        printf("ADDS\n");
                    }
                    break;
                    
                case MINUS:
                gen_nil_check();
                    printf("SUBS\n");
                    break;
                    
                case TIMES: {

                    int uid = if_label_counter++; 


                    printf("POPS GF@_mul_r\n"); 
                    printf("POPS GF@_mul_l\n"); 

                    printf("TYPE GF@_mul_type_l GF@_mul_l\n");
                    printf("TYPE GF@_mul_type_r GF@_mul_r\n");

                    printf("JUMPIFEQ $mul_str_%d GF@_mul_type_l string@string\n", uid);

                    printf("JUMPIFEQ $mul_l_float_%d GF@_mul_type_l string@float\n", uid);
                    printf("JUMP $mul_err_%d\n", uid); 

                    printf("LABEL $mul_l_float_%d\n", uid);

                    printf("JUMPIFEQ $mul_r_float_%d GF@_mul_type_r string@float\n", uid);
                    printf("JUMP $mul_err_%d\n", uid);

                    printf("LABEL $mul_r_float_%d\n", uid);

                    printf("PUSHS GF@_mul_l\n");
                    printf("PUSHS GF@_mul_r\n");
                    printf("MULS\n");
                    printf("JUMP $mul_end_%d\n", uid);

                    printf("LABEL $mul_str_%d\n", uid);
      
                    printf("JUMPIFNEQ $mul_err_%d GF@_mul_type_r string@float\n", uid);

                    printf("ISINT GF@_mul_check_int GF@_mul_r\n");
                    printf("JUMPIFNEQ $mul_err_%d GF@_mul_check_int bool@true\n", uid);

                    
                    printf("LABEL $mul_str_int_ok_%d\n", uid);

                    printf("MOVE GF@_mul_acc string@\n");

                    printf("LABEL $mul_loop_%d\n", uid);
                    printf("PUSHS GF@_mul_r\n");
                    printf("PUSHS float@0x0p+0\n");
                    printf("GTS\n");
                    printf("PUSHS bool@true\n");
                    printf("JUMPIFNEQS $mul_loop_end_%d\n", uid); 
                    printf("CONCAT GF@_mul_acc GF@_mul_acc GF@_mul_l\n");
                    printf("SUB GF@_mul_r GF@_mul_r float@0x1p+0\n");
                    printf("JUMP $mul_loop_%d\n", uid);

                    printf("LABEL $mul_loop_end_%d\n", uid);
                    printf("PUSHS GF@_mul_acc\n");
                    printf("JUMP $mul_end_%d\n", uid);
                    printf("LABEL $mul_err_%d\n", uid);
                    printf("EXIT int@26\n");

                    printf("LABEL $mul_end_%d\n", uid);
                    break;
                }
                    
                case DIVIDE:
                    gen_nil_check();
                    printf("POPS GF@_div_r\n");
                    printf("POPS GF@_div_l\n");

                    
                    printf("TYPE GF@_div_type_l GF@_div_l\n");
                    printf("JUMPIFEQ $div_l_is_f GF@_div_type_l string@float\n");
                    printf("INT2FLOAT GF@_div_l_f GF@_div_l\n");
                    printf("JUMP $div_r_check\n");
                    printf("LABEL $div_l_is_f\n");
                    printf("MOVE GF@_div_l_f GF@_div_l\n");
                    printf("LABEL $div_r_check\n");

                   
                    printf("TYPE GF@_div_type_r GF@_div_r\n");
                    printf("JUMPIFEQ $div_r_is_f GF@_div_type_r string@float\n");
                    printf("INT2FLOAT GF@_div_r_f GF@_div_r\n");
                    printf("JUMP $div_op\n");
                    printf("LABEL $div_r_is_f\n");
                    printf("MOVE GF@_div_r_f GF@_div_r\n");
                    printf("LABEL $div_op\n");

                    
                    printf("PUSHS GF@_div_l_f\n");
                    printf("PUSHS GF@_div_r_f\n");
                    
                   
                    printf("DIVS\n");
                    
                    
                    break;

                
                case LESSER:
                    printf("LTS\n");
                    break;
                case GREATER:
                    printf("GTS\n");
                    break;
                case LESSER_EQUAL:
                    printf("GTS\n");
                    printf("NOTS\n");
                    break;
                case GREATER_EQUAL:
                    printf("LTS\n");
                    printf("NOTS\n");
                    break;
                case EQUAL:{
                    int current_eq_id = if_label_counter++;
            
                    
                    printf("POPS GF@_eq_help_1\n"); 
                    printf("POPS GF@_eq_help_2\n");       

                    
                    printf("TYPE GF@_eq_help_3 GF@_eq_help_2\n");       
                    printf("TYPE GF@_eq_help_4 GF@_eq_help_1\n"); 

                   
                    printf("JUMPIFEQ $eq_types_ok_%d GF@_eq_help_3 GF@_eq_help_4\n", current_eq_id);

                  
                    printf("JUMPIFEQ $eq_types_ok_%d GF@_eq_help_3 string@nil\n", current_eq_id);
                    printf("JUMPIFEQ $eq_types_ok_%d GF@_eq_help_4 string@nil\n", current_eq_id);

                    
                    printf("PUSHS bool@false\n");
                    printf("JUMP $eq_end_%d\n", current_eq_id);

                    
                    printf("LABEL $eq_types_ok_%d\n", current_eq_id);
                    printf("PUSHS GF@_eq_help_2\n");
                    printf("PUSHS GF@_eq_help_1\n");
                    printf("EQS\n");

                    printf("LABEL $eq_end_%d\n", current_eq_id);
                    break;
                }
                case NOT_EQUAL:{
                    int current_eq_id = if_label_counter++;
            
                    
                    printf("POPS GF@_eq_help_1\n");
                    printf("POPS GF@_eq_help_2\n");       

                    
                    printf("TYPE GF@_eq_help_3 GF@_eq_help_2\n");
                    printf("TYPE GF@_eq_help_4 GF@_eq_help_1\n"); 

                    
                    printf("JUMPIFEQ $eq_types_ok_%d GF@_eq_help_3 GF@_eq_help_4\n", current_eq_id);

                    
                    printf("JUMPIFEQ $eq_types_ok_%d GF@_eq_help_3 string@nil\n", current_eq_id);
                    printf("JUMPIFEQ $eq_types_ok_%d GF@_eq_help_4 string@nil\n", current_eq_id);

                    
                    printf("PUSHS bool@false\n");
                    printf("JUMP $eq_end_%d\n", current_eq_id);

                    
                    printf("LABEL $eq_types_ok_%d\n", current_eq_id);
                    printf("PUSHS GF@_eq_help_2\n");
                    printf("PUSHS GF@_eq_help_1\n");
                    printf("EQS\n");

                    printf("LABEL $eq_end_%d\n", current_eq_id);

                    printf("NOTS\n");
                    break;

                }
                    
                    

                
                case KEYWORD_is:{
                   
                    int current_is_id = if_label_counter;
                    if_label_counter++;
            
                    
                    
                    printf("POPS GF@_is_type_keyword\n"); 
                    printf("POPS GF@_is_val\n");          

                    
                    printf("TYPE GF@_is_actual_type GF@_is_val\n");

                    
                    printf("JUMPIFNEQ $is_check_string_%d GF@_is_type_keyword string@Num\n", current_is_id);
                        
                    printf("JUMPIFEQ $is_true_%d GF@_is_actual_type string@int\n", current_is_id);
                        
                    printf("JUMPIFEQ $is_true_%d GF@_is_actual_type string@float\n", current_is_id);
                        
                    printf("PUSHS bool@false\n");
                    printf("JUMP $is_end_%d\n", current_is_id);

                    
                    printf("LABEL $is_check_string_%d\n", current_is_id);
                    printf("JUMPIFNEQ $is_check_null_%d GF@_is_type_keyword string@String\n", current_is_id);
                        
                    printf("JUMPIFEQ $is_true_%d GF@_is_actual_type string@string\n", current_is_id);
                        
                    printf("PUSHS bool@false\n");
                    printf("JUMP $is_end_%d\n", current_is_id);

                    
                    printf("LABEL $is_check_null_%d\n", current_is_id);
                    
                    printf("JUMPIFEQ $is_true_%d GF@_is_actual_type string@nil\n", current_is_id);
                    
                    printf("PUSHS bool@false\n");
                    printf("JUMP $is_end_%d\n", current_is_id);
                   
                    printf("LABEL $is_true_%d\n", current_is_id);
                    printf("PUSHS bool@true\n");
                    
                    printf("LABEL $is_end_%d\n", current_is_id);
                    break;
                }

                default:
                    fprintf(stderr, "CodeGen: Neznámý binární operátor %d\n", expr->op);
                    ifjexit(ERR_INTERNAL);
    
            }
            break;
            
        }
            
        case AST_NODE_LIST: {
            AstNodeList *list = (AstNodeList *)node;
            
            for (int i = 0; i < list->count; i++) {
                
                traverse_and_print_ast(list->items[i]);
            }
            break;
        }

        
        case AST_STMT_VAR_DECL:{
            AstNodeVarDecl *var_decl = (AstNodeVarDecl *)node;
            
            if(var_decl->stack_offset < -1){
                printf("DEFVAR LF@%s_%d\n", var_decl->var_id.lexeme, abs(var_decl->stack_offset));
            }else if(var_decl->stack_offset == -1){
                printf("DEFVAR GF@%s\n", var_decl->var_id.lexeme);
            }
            
            break;
        }
            
        case AST_EXPR_LITERAL: {
            AstNodeLiteral *lit = (AstNodeLiteral *)node;
            
            if(lit->token.type == INT){
                printf("PUSHS float@%a\n", (double)lit->token.value.int_val);
            }else if(lit->token.type == FLOAT){
                printf("PUSHS float@%a\n", lit->token.value.float_val);
            }else if(lit->token.type == STRING){
                printf("PUSHS ");
                print_string_literal_escaped(lit->token.value.string_val);
                printf("\n");
            }else if(lit->token.type == KEYWORD_null){
                printf("PUSHS nil@nil\n");
            }else if(lit->token.type == KEYWORD_Num){
                
                printf("PUSHS string@Num\n");
                
            } else if(lit->token.type == KEYWORD_String){
                
                printf("PUSHS string@String\n");
                
            } else if(lit->token.type == KEYWORD_Null){
                
                printf("PUSHS string@Null\n");
            }

            break;
        }
            
        case AST_EXPR_VARIABLE: {
            AstNodeVariable *var = (AstNodeVariable *)node;
            if(var->stack_offset < -1){
                
                printf("PUSHS LF@%s_%d\n", var->token.lexeme, abs(var->stack_offset));
            }else if(var->stack_offset == -1){
                
                printf("PUSHS GF@%s\n", var->token.lexeme);
            }else if(var->stack_offset == 0){
                printf("CREATEFRAME\n");
                printf("CALL %s_GET\n", var->token.lexeme);
                printf("PUSHS TF@%%retval\n"); 
            }
            

            break;
        }
            
            
        
        default:
            
            break;
    }
}
/**
 * @brief První průchod AST pro vybrání globálních proměných
 * Prochází strom a hledá proměnné s offsetem -1.
 * @param node Aktuální uzel AST
 */
static void first_travers(AstNode *node) {

    if (!node) {
        return;
    }

    


    switch (node->type) {
        case AST_PROGRAM: {
            AstNodeProgram *prog = (AstNodeProgram *)node;
            first_travers((AstNode *)prog->functions);
            break;
        }
        case AST_FUNC_DEF: {
            AstNodeFuncDef *func = (AstNodeFuncDef *)node;
            
            first_travers((AstNode *)func->params);
            first_travers((AstNode *)func->body);
        

            
            break;
        }
        case AST_STMT_BLOCK: {
            AstNodeBlock *block = (AstNodeBlock *)node;
            first_travers((AstNode *)block->statements);
            break;
        }
        case AST_STMT_ASSIGN: {
            AstNodeAssignStmt *stmt = (AstNodeAssignStmt *)node;
            first_travers((AstNode *)stmt->lvalue);
            first_travers((AstNode *)stmt->rvalue);
            
            break;
        }
        case AST_STMT_IF: {
            AstNodeIfStmt *stmt = (AstNodeIfStmt *)node;
            first_travers(stmt->condition);
            first_travers((AstNode *)stmt->then_block);
            first_travers((AstNode *)stmt->else_block);
            break;
        }
        case AST_STMT_WHILE: {
            AstNodeWhileStmt *stmt = (AstNodeWhileStmt *)node;
            first_travers(stmt->condition);
            first_travers((AstNode *)stmt->body_block);
            break;
        }
        case AST_STMT_RETURN: {
            AstNodeReturnStmt *stmt = (AstNodeReturnStmt *)node;
            first_travers(stmt->expr);
            break;
        }
        case AST_FUNC_CALL: {
            AstNodeFuncCall *call = (AstNodeFuncCall *)node;

            first_travers((AstNode *)call->args);

            

            
            
            
            break;
        }
        case AST_EXPR_BINARY: {
            AstNodeBinaryExpr *expr = (AstNodeBinaryExpr *)node;
            
            
            first_travers(expr->left);
            first_travers(expr->right);
            
            break;
            
        }
            
        case AST_NODE_LIST: {
            AstNodeList *list = (AstNodeList *)node;

            for (int i = 0; i < list->count; i++) {

                first_travers(list->items[i]);
            }
            break;
        }

        case AST_STMT_VAR_DECL:{

            
            
            
            break;
        }
            
        case AST_EXPR_LITERAL: {

            
            

            break;
        }
            
        case AST_EXPR_VARIABLE: {
            AstNodeVariable *var = (AstNodeVariable *)node;
            if(var->stack_offset == -1){
                int flag = 1;
                for(int i = 0; i < 128; i++){
                    if(strcmp(global_vars[i], var->token.lexeme) == 0){
                        flag = 0;
                        break;
                    }
                } 
                if(flag){
                    strcpy(global_vars[counter], var->token.lexeme);
                    counter++;
                }

            }
            

            break;
        }
            
        
        default:

            break;
    }
}

/**
 * @brief vypíše kód vestavěných funkcí
 * 
 */
void help_func() {
    printf("LABEL IFJ_WRITE\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("MOVE LF@%%retval nil@nil\n");
    printf("DEFVAR LF@type\n");
    printf("TYPE LF@type LF@%%param_1\n");
    printf("JUMPIFEQ $write_check_float LF@type string@float\n");
    printf("WRITE LF@%%param_1\n");
    printf("JUMP $write_end\n");
    printf("LABEL $write_check_float\n");
    printf("DEFVAR LF@is_int\n");
    printf("ISINT LF@is_int LF@%%param_1\n");
    printf("JUMPIFEQ $write_as_int LF@is_int bool@true\n");
    printf("WRITE LF@%%param_1\n");
    printf("JUMP $write_end\n");
    printf("LABEL $write_as_int\n");
    printf("DEFVAR LF@int_val\n");
    printf("FLOAT2INT LF@int_val LF@%%param_1\n");
    printf("WRITE LF@int_val\n");
    printf("LABEL $write_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("\n");
    printf("LABEL IFJ_STR\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("MOVE LF@%%retval nil@nil\n");
    printf("DEFVAR LF@type\n");
    printf("TYPE LF@type LF@%%param_1\n");
    printf("JUMPIFEQ $str_is_string LF@type string@string\n");
    printf("JUMPIFEQ $str_is_int LF@type string@int\n");
    printf("JUMPIFEQ $str_is_float LF@type string@float\n");
    printf("JUMP $str_end\n");
    printf("LABEL $str_is_string\n");
    printf("MOVE LF@%%retval LF@%%param_1\n");
    printf("JUMP $str_end\n");
    printf("LABEL $str_is_int\n");
    printf("INT2STR LF@%%retval LF@%%param_1\n");
    printf("JUMP $str_end\n");
    printf("LABEL $str_is_float\n");
    printf("FLOAT2STR LF@%%retval LF@%%param_1\n"); 
    printf("JUMP $str_end\n");
    printf("LABEL $str_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("\n");
    printf("LABEL IFJ_READ_STR\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("READ LF@%%retval string\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("\n");
    printf("LABEL IFJ_READ_NUM\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("READ LF@%%retval float\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("\n");
    printf("LABEL IFJ_FLOOR\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("DEFVAR LF@type\n");
    printf("TYPE LF@type LF@%%param_1\n");
    printf("JUMPIFEQ $floor_is_int LF@type string@int\n");
    printf("FLOAT2INT LF@%%retval LF@%%param_1\n");
    printf("INT2FLOAT LF@%%retval LF@%%retval\n");
    printf("JUMP $floor_end\n");
    printf("LABEL $floor_is_int\n");
    printf("INT2FLOAT LF@%%retval LF@%%param_1\n");
    printf("LABEL $floor_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("\n");
    printf("LABEL IFJ_LENGTH\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("MOVE LF@%%retval nil@nil\n");
    printf("DEFVAR LF@type\n");
    printf("TYPE LF@type LF@%%param_1\n");
    printf("JUMPIFEQ $len_ok LF@type string@string\n");
    printf("EXIT int@25\n");
    printf("LABEL $len_ok\n");
    printf("DEFVAR LF@len_int\n");
    printf("STRLEN LF@len_int LF@%%param_1\n");
    printf("INT2FLOAT LF@%%retval LF@len_int\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("\n");
    printf("LABEL IFJ_SUBSTRING\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("MOVE LF@%%retval nil@nil\n");
    printf("DEFVAR LF@type\n");
    printf("TYPE LF@type LF@%%param_1\n");
    printf("JUMPIFNEQ $sub_err_25 LF@type string@string\n");
    printf("TYPE LF@type LF@%%param_2\n");
    printf("JUMPIFNEQ $sub_err_25 LF@type string@float\n");
    printf("TYPE LF@type LF@%%param_3\n");
    printf("JUMPIFNEQ $sub_err_25 LF@type string@float\n");
    printf("DEFVAR LF@is_int\n");
    printf("ISINT LF@is_int LF@%%param_2\n");
    printf("JUMPIFNEQ $sub_err_26 LF@is_int bool@true\n");
    printf("ISINT LF@is_int LF@%%param_3\n");
    printf("JUMPIFNEQ $sub_err_26 LF@is_int bool@true\n");
    printf("DEFVAR LF@i\n");
    printf("FLOAT2INT LF@i LF@%%param_2\n");
    printf("DEFVAR LF@j\n");
    printf("FLOAT2INT LF@j LF@%%param_3\n");
    printf("DEFVAR LF@len\n");
    printf("STRLEN LF@len LF@%%param_1\n");
    printf("DEFVAR LF@cond\n");
    printf("LT LF@cond LF@i int@0\n");
    printf("JUMPIFEQ $sub_ret_null LF@cond bool@true\n");
    printf("LT LF@cond LF@j int@0\n");
    printf("JUMPIFEQ $sub_ret_null LF@cond bool@true\n");
    printf("GT LF@cond LF@i LF@j\n");
    printf("JUMPIFEQ $sub_ret_null LF@cond bool@true\n");
    printf("LT LF@cond LF@i LF@len\n");
    printf("JUMPIFNEQ $sub_ret_null LF@cond bool@true\n");
    printf("GT LF@cond LF@j LF@len\n");
    printf("JUMPIFEQ $sub_ret_null LF@cond bool@true\n");
    printf("MOVE LF@%%retval string@\n");
    printf("DEFVAR LF@k\n");
    printf("MOVE LF@k LF@i\n");
    printf("DEFVAR LF@char\n");
    printf("LABEL $sub_loop\n");
    printf("LT LF@cond LF@k LF@j\n");
    printf("JUMPIFNEQ $sub_done LF@cond bool@true\n");
    printf("GETCHAR LF@char LF@%%param_1 LF@k\n");
    printf("CONCAT LF@%%retval LF@%%retval LF@char\n");
    printf("ADD LF@k LF@k int@1\n");
    printf("JUMP $sub_loop\n");
    printf("LABEL $sub_done\n");
    printf("JUMP $sub_end\n");
    printf("LABEL $sub_err_25\n");
    printf("EXIT int@25\n");
    printf("LABEL $sub_err_26\n");
    printf("EXIT int@26\n");
    printf("LABEL $sub_ret_null\n");
    printf("MOVE LF@%%retval nil@nil\n");
    printf("LABEL $sub_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("\n");
    printf("LABEL IFJ_STRCMP\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("MOVE LF@%%retval nil@nil\n");
    printf("DEFVAR LF@type\n");
    printf("TYPE LF@type LF@%%param_1\n");
    printf("JUMPIFNEQ $strcmp_err_25 LF@type string@string\n");
    printf("TYPE LF@type LF@%%param_2\n");
    printf("JUMPIFNEQ $strcmp_err_25 LF@type string@string\n");
    printf("DEFVAR LF@cmp_res\n");
    printf("PUSHS LF@%%param_1\n");
    printf("PUSHS LF@%%param_2\n");
    printf("EQS\n");
    printf("POPS LF@cmp_res\n");
    printf("JUMPIFEQ $strcmp_eq LF@cmp_res bool@true\n");
    printf("PUSHS LF@%%param_1\n");
    printf("PUSHS LF@%%param_2\n");
    printf("LTS\n");
    printf("POPS LF@cmp_res\n");
    printf("JUMPIFEQ $strcmp_less LF@cmp_res bool@true\n");
    printf("MOVE LF@%%retval float@0x1p+0\n"); // 1.0
    printf("JUMP $strcmp_end\n");
    printf("LABEL $strcmp_eq\n");
    printf("MOVE LF@%%retval float@0x0p+0\n"); // 0.0
    printf("JUMP $strcmp_end\n");
    printf("LABEL $strcmp_less\n");
    printf("MOVE LF@%%retval float@-0x1p+0\n"); // -1.0
    printf("JUMP $strcmp_end\n");
    printf("LABEL $strcmp_err_25\n");
    printf("EXIT int@25\n");
    printf("LABEL $strcmp_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("\n");
    printf("LABEL IFJ_ORD\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("MOVE LF@%%retval nil@nil\n");


    printf("DEFVAR LF@type\n");
    printf("TYPE LF@type LF@%%param_1\n");
    printf("JUMPIFNEQ $ord_err_25 LF@type string@string\n");
    printf("TYPE LF@type LF@%%param_2\n");
    printf("JUMPIFNEQ $ord_err_25 LF@type string@float\n");

    printf("DEFVAR LF@is_int\n");
    printf("ISINT LF@is_int LF@%%param_2\n");
    printf("JUMPIFNEQ $ord_err_26 LF@is_int bool@true\n");
    printf("DEFVAR LF@idx\n");
    printf("FLOAT2INT LF@idx LF@%%param_2\n");

    printf("DEFVAR LF@len\n");
    printf("STRLEN LF@len LF@%%param_1\n");
    
    printf("DEFVAR LF@cond\n");
    printf("LT LF@cond LF@idx int@0\n");
    printf("JUMPIFEQ $ord_ret_0 LF@cond bool@true\n");
    printf("LT LF@cond LF@idx LF@len\n");
    printf("JUMPIFNEQ $ord_ret_0 LF@cond bool@true\n");
    printf("DEFVAR LF@res_int\n");
    printf("STRI2INT LF@res_int LF@%%param_1 LF@idx\n");
    printf("INT2FLOAT LF@%%retval LF@res_int\n");
    printf("JUMP $ord_end\n");
    printf("LABEL $ord_ret_0\n");
    printf("MOVE LF@%%retval float@0x0p+0\n");
    printf("JUMP $ord_end\n");

    printf("LABEL $ord_err_25\n");
    printf("EXIT int@25\n");

    printf("LABEL $ord_err_26\n");
    printf("EXIT int@26\n");

    printf("LABEL $ord_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("\n");
    printf("LABEL IFJ_CHR\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("MOVE LF@%%retval nil@nil\n");


    printf("DEFVAR LF@type\n");
    printf("TYPE LF@type LF@%%param_1\n");
    printf("JUMPIFNEQ $chr_err_25 LF@type string@float\n");


    printf("DEFVAR LF@is_int\n");
    printf("ISINT LF@is_int LF@%%param_1\n");
    printf("JUMPIFNEQ $chr_err_26 LF@is_int bool@true\n");

    printf("DEFVAR LF@char_code\n");
    printf("FLOAT2INT LF@char_code LF@%%param_1\n");
    printf("INT2CHAR LF@%%retval LF@char_code\n");
    printf("JUMP $chr_end\n");
    printf("LABEL $chr_err_25\n");
    printf("EXIT int@25\n");

    printf("LABEL $chr_err_26\n");
    printf("EXIT int@26\n");

    printf("LABEL $chr_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("\n");

    
}





/**
 * @brief Vstupní funkce pro generování kódu z AST
 * volání z main.c.
 * Nejprve spustí první průchod pro sběr globálních proměných, potom se vypíšou globální proměnné, pomocné proměnné a vestavěné funkce.
 * Nakonec se provede hlavní průchod AST pro generování kódu.
 * 
 * @param root Kořen AST
 */
void generate_code(AstNode *root) {
    if (!root) {
        fprintf(stderr, "AST je prázdný, nelze tisknout.\n");
        return;
    }

    first_travers(root);

    printf(".IFJcode25\n");
    printf("\n");
    for(int i = 0; i < counter; i++){
        printf("DEFVAR GF@%s\n", global_vars[i]);
        printf("MOVE GF@%s nil@nil\n", global_vars[i]);
    }
    printf("DEFVAR GF@_div_l\n");
    printf("DEFVAR GF@_div_r\n");
    printf("DEFVAR GF@_div_l_f\n");
    printf("DEFVAR GF@_div_r_f\n");
    printf("DEFVAR GF@_div_type_l\n");
    printf("DEFVAR GF@_div_type_r\n");
    printf("DEFVAR GF@_cond_result\n");
    printf("DEFVAR GF@_cond_type\n");
    printf("DEFVAR GF@_mul_n\n");
    printf("DEFVAR GF@_mul_s\n");
    printf("DEFVAR GF@_mul_acc\n");
    printf("DEFVAR GF@_mul_pod\n");
    printf("DEFVAR GF@_is_val\n");
    printf("DEFVAR GF@_is_type_keyword\n");
    printf("DEFVAR GF@_is_actual_type\n");
    printf("DEFVAR GF@_eq_help_1\n");
    printf("DEFVAR GF@_eq_help_2\n");
    printf("DEFVAR GF@_eq_help_3\n");
    printf("DEFVAR GF@_eq_help_4\n");
    printf("DEFVAR GF@_concat_help_1\n");
    printf("DEFVAR GF@_concat_help_2\n");
    printf("DEFVAR GF@_check_op_1\n");
    printf("DEFVAR GF@_check_op_2\n");
    printf("DEFVAR GF@_check_type\n");
    printf("DEFVAR GF@_check_res\n");
    printf("DEFVAR GF@_mul_l\n");
    printf("DEFVAR GF@_mul_r\n");
    printf("DEFVAR GF@_mul_type_l\n");
    printf("DEFVAR GF@_mul_type_r\n");
    printf("DEFVAR GF@_mul_check_int\n");

    
    
    printf("JUMP main\n");
    printf("\n");
    help_func();
    
    
    traverse_and_print_ast(root);
    
    
}