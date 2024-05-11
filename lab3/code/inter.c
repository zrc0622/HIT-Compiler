#include "inter.h"

bool inter_error = false;
pInterCodeList intercode_list;  // 定义中间代码双向链表

/*operand*/
pOperand new_operand(int kind, ...){
    pOperand operand = (pOperand)malloc(sizeof(Operand));
    operand->kind = kind;
    va_list vaList;
    va_start(vaList, kind);
    switch(kind){
        case OP_CONSTANT:   // 常数
            operand->u.value = va_arg(vaList, int);
            break;
        
        case OP_VARIABLE:   // 变量、地址、标签、函数、关系运算符
        case OP_ADDRESS:
        case OP_ADDRESS_STRUCT:
        case OP_LABEL:
        case OP_FUNCTION:
        case OP_RELOP:
            operand->u.name = va_arg(vaList, char*);
            break;
    }
    return operand;
}

void delete_operand(pOperand operand){
    if(operand == NULL) return;
    switch(operand->kind){
        case OP_CONSTANT:
            break;
        
        case OP_VARIABLE:
        case OP_ADDRESS:
        case OP_ADDRESS_STRUCT:
        case OP_LABEL:
        case OP_FUNCTION:
        case OP_RELOP:
            if(operand->u.name){
                free(operand->u.name);
                operand->u.name = NULL;
            }
            break;
    }
    free(operand);
}

void set_operand(pOperand operand, int kind, ...){
    operand->kind = kind;
    va_list vaList;
    va_start(vaList, kind);
    switch(kind) {
        case OP_CONSTANT:
            operand->u.value = va_arg(vaList, int);
            break;
        
        case OP_VARIABLE:
        case OP_ADDRESS:
        case OP_ADDRESS_STRUCT:
        case OP_LABEL:
        case OP_FUNCTION:
        case OP_RELOP:
            if(operand->u.name) free(operand->u.name);
            operand->u.name = va_arg(vaList, char*);
            break;
    }
}

void print_operand(FILE* fp, pOperand operand){
    if(fp == NULL){
        switch (operand->kind) {
            case OP_CONSTANT:
                printf("#%d", operand->u.value);    // 常量加上#
                break;

            case OP_VARIABLE:
            case OP_ADDRESS:
            case OP_LABEL:
            case OP_FUNCTION:
            case OP_RELOP:
                printf("%s", operand->u.name);      // 其它直接打印
                break;
            
            case OP_ADDRESS_STRUCT:
                printf("&%s", operand->u.name);
                break;
        }
    } 
    else{
        switch (operand->kind) {
            case OP_CONSTANT:
                fprintf(fp, "#%d", operand->u.value);
                break;
            case OP_VARIABLE:
            case OP_ADDRESS:
            case OP_LABEL:
            case OP_FUNCTION:
            case OP_RELOP:
                fprintf(fp, "%s", operand->u.name);
                break;

            case OP_ADDRESS_STRUCT:
                fprintf(fp,"&%s", operand->u.name);
                break;
        }
    }
}

/*intercode list*/
pInterCode new_intercode(int kind, ...){
    pInterCode intercode = (pInterCode)malloc(sizeof(InterCode));
    intercode->kind = kind;
    va_list vaList;
    va_start(vaList, kind);
    switch (kind) {
        case IR_LABEL:  // 标签、函数、无条件跳转、返回、参数传递、参数定义、写入: LABEL op
        case IR_FUNCTION:
        case IR_GOTO:
        case IR_RETURN:
        case IR_ARG:
        case IR_PARAM:
        case IR_READ:
        case IR_WRITE:
            intercode->u.OneOp.op = va_arg(vaList, pOperand);
            break;

        case IR_ASSIGN: // 赋值、取地址、读数据（地址）、写数据（地址）、函数调用: left ASSIGN right
        case IR_GET_ADDR:
        case IR_READ_ADDR:
        case IR_WRITE_ADDR:
        case IR_CALL:
            intercode->u.Assign.left = va_arg(vaList, pOperand);
            intercode->u.Assign.right = va_arg(vaList, pOperand);
            break;

        case IR_ADD:    // 加减乘除: result ASSIGN op1 ADD op2
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            intercode->u.BinOp.result = va_arg(vaList, pOperand);
            intercode->u.BinOp.op1 = va_arg(vaList, pOperand);
            intercode->u.BinOp.op2 = va_arg(vaList, pOperand);
            break;

        case IR_DEC:    // 声明: int op
            intercode->u.Dec.op = va_arg(vaList, pOperand);
            intercode->u.Dec.size = va_arg(vaList, int);
            break;

        case IR_IF_GOTO:    // 条件跳转: x relop y, z
            intercode->u.IfGoto.x = va_arg(vaList, pOperand);
            intercode->u.IfGoto.relop = va_arg(vaList, pOperand);
            intercode->u.IfGoto.y = va_arg(vaList, pOperand);
            intercode->u.IfGoto.z = va_arg(vaList, pOperand);
    }
    return intercode;
}

void delete_intercode(pInterCode intercode){
    switch (intercode->kind) {
        case IR_LABEL:
        case IR_FUNCTION:
        case IR_GOTO:
        case IR_RETURN:
        case IR_ARG:
        case IR_PARAM:
        case IR_READ:
        case IR_WRITE:
            delete_operand(intercode->u.OneOp.op);
            break;

        case IR_ASSIGN:
        case IR_GET_ADDR:
        case IR_READ_ADDR:
        case IR_WRITE_ADDR:
        case IR_CALL:
            delete_operand(intercode->u.Assign.left);
            delete_operand(intercode->u.Assign.right);
            break;

        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            delete_operand(intercode->u.BinOp.result);
            delete_operand(intercode->u.BinOp.op1);
            delete_operand(intercode->u.BinOp.op2);
            break;

        case IR_DEC:
            delete_operand(intercode->u.Dec.op);
            break;

        case IR_IF_GOTO:
            delete_operand(intercode->u.IfGoto.x);
            delete_operand(intercode->u.IfGoto.relop);
            delete_operand(intercode->u.IfGoto.y);
            delete_operand(intercode->u.IfGoto.z);
    }
    free(intercode);
}

void print_intercode_list(FILE* fp, pInterCodeList intercode_list){
    for(pInterCodeHead cur = intercode_list->head; cur != NULL; cur = cur->next){
        if(fp == NULL) {
            switch (cur->intercode->kind) {
                case IR_LABEL:
                    printf("LABEL ");   // LABEL L1 :
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    printf(" :");
                    break;
                case IR_FUNCTION:
                    printf("FUNCTION ");    // FUNCTION main :
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    printf(" :");
                    break;
                case IR_ASSIGN: // a := b
                    print_operand(fp, cur->intercode->u.Assign.left);
                    printf(" := ");
                    print_operand(fp, cur->intercode->u.Assign.right);
                    break;
                case IR_ADD:    // a := b + c
                    print_operand(fp, cur->intercode->u.BinOp.result);
                    printf(" := ");
                    print_operand(fp, cur->intercode->u.BinOp.op1);
                    printf(" + ");
                    print_operand(fp, cur->intercode->u.BinOp.op2);
                    break;
                case IR_SUB:
                    print_operand(fp, cur->intercode->u.BinOp.result);
                    printf(" := ");
                    print_operand(fp, cur->intercode->u.BinOp.op1);
                    printf(" - ");
                    print_operand(fp, cur->intercode->u.BinOp.op2);
                    break;
                case IR_MUL:
                    print_operand(fp, cur->intercode->u.BinOp.result);
                    printf(" := ");
                    print_operand(fp, cur->intercode->u.BinOp.op1);
                    printf(" * ");
                    print_operand(fp, cur->intercode->u.BinOp.op2);
                    break;
                case IR_DIV:
                    print_operand(fp, cur->intercode->u.BinOp.result);
                    printf(" := ");
                    print_operand(fp, cur->intercode->u.BinOp.op1);
                    printf(" / ");
                    print_operand(fp, cur->intercode->u.BinOp.op2);
                    break;
                case IR_GET_ADDR:   // a := &b
                    print_operand(fp, cur->intercode->u.Assign.left);
                    printf(" := &");
                    print_operand(fp, cur->intercode->u.Assign.right);
                    break;
                case IR_READ_ADDR:  // a := *b
                    print_operand(fp, cur->intercode->u.Assign.left);
                    printf(" := *");
                    print_operand(fp, cur->intercode->u.Assign.right);
                    break;
                case IR_WRITE_ADDR: // *a := b
                    printf("*");
                    print_operand(fp, cur->intercode->u.Assign.left);
                    printf(" := ");
                    print_operand(fp, cur->intercode->u.Assign.right);
                    break;
                case IR_GOTO:   // GOTO L1
                    printf("GOTO ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
                case IR_IF_GOTO:    // IF a > b GOTO L1
                    printf("IF ");
                    print_operand(fp, cur->intercode->u.IfGoto.x);
                    printf(" ");
                    print_operand(fp, cur->intercode->u.IfGoto.relop);
                    printf(" ");
                    print_operand(fp, cur->intercode->u.IfGoto.y);
                    printf(" GOTO ");
                    print_operand(fp, cur->intercode->u.IfGoto.z);
                    break;
                case IR_RETURN: // RETURN a
                    printf("RETURN ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
                case IR_DEC:    // DEC a 8
                    printf("DEC ");
                    print_operand(fp, cur->intercode->u.Dec.op);
                    printf(" ");
                    printf("%d", cur->intercode->u.Dec.size);
                    break;
                case IR_ARG:
                    printf("ARG "); // ARG a
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
                case IR_CALL:   // a := CALL fact
                    print_operand(fp, cur->intercode->u.Assign.left);
                    printf(" := CALL ");
                    print_operand(fp, cur->intercode->u.Assign.right);
                    break;
                case IR_PARAM:  // PARAM a
                    printf("PARAM ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
                case IR_READ:
                    printf("READ ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
                case IR_WRITE:
                    printf("WRITE ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
            }
            printf("\n");
        } 
        else{
            switch (cur->intercode->kind) {
                case IR_LABEL:
                    fprintf(fp, "LABEL ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    fprintf(fp, " :");
                    break;
                case IR_FUNCTION:
                    fprintf(fp, "FUNCTION ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    fprintf(fp, " :");
                    break;
                case IR_ASSIGN:
                    print_operand(fp, cur->intercode->u.Assign.left);
                    fprintf(fp, " := ");
                    print_operand(fp, cur->intercode->u.Assign.right);
                    break;
                case IR_ADD:
                    print_operand(fp, cur->intercode->u.BinOp.result);
                    fprintf(fp, " := ");
                    print_operand(fp, cur->intercode->u.BinOp.op1);
                    fprintf(fp, " + ");
                    print_operand(fp, cur->intercode->u.BinOp.op2);
                    break;
                case IR_SUB:
                    print_operand(fp, cur->intercode->u.BinOp.result);
                    fprintf(fp, " := ");
                    print_operand(fp, cur->intercode->u.BinOp.op1);
                    fprintf(fp, " - ");
                    print_operand(fp, cur->intercode->u.BinOp.op2);
                    break;
                case IR_MUL:
                    print_operand(fp, cur->intercode->u.BinOp.result);
                    fprintf(fp, " := ");
                    print_operand(fp, cur->intercode->u.BinOp.op1);
                    fprintf(fp, " * ");
                    print_operand(fp, cur->intercode->u.BinOp.op2);
                    break;
                case IR_DIV:
                    print_operand(fp, cur->intercode->u.BinOp.result);
                    fprintf(fp, " := ");
                    print_operand(fp, cur->intercode->u.BinOp.op1);
                    fprintf(fp, " / ");
                    print_operand(fp, cur->intercode->u.BinOp.op2);
                    break;
                case IR_GET_ADDR:
                    print_operand(fp, cur->intercode->u.Assign.left);
                    fprintf(fp, " := &");
                    print_operand(fp, cur->intercode->u.Assign.right);
                    break;
                case IR_READ_ADDR:
                    print_operand(fp, cur->intercode->u.Assign.left);
                    fprintf(fp, " := *");
                    print_operand(fp, cur->intercode->u.Assign.right);
                    break;
                case IR_WRITE_ADDR:
                    fprintf(fp, "*");
                    print_operand(fp, cur->intercode->u.Assign.left);
                    fprintf(fp, " := ");
                    print_operand(fp, cur->intercode->u.Assign.right);
                    break;
                case IR_GOTO:
                    fprintf(fp, "GOTO ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
                case IR_IF_GOTO:
                    fprintf(fp, "IF ");
                    print_operand(fp, cur->intercode->u.IfGoto.x);
                    fprintf(fp, " ");
                    print_operand(fp, cur->intercode->u.IfGoto.relop);
                    fprintf(fp, " ");
                    print_operand(fp, cur->intercode->u.IfGoto.y);
                    fprintf(fp, " GOTO ");
                    print_operand(fp, cur->intercode->u.IfGoto.z);
                    break;
                case IR_RETURN:
                    fprintf(fp, "RETURN ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
                case IR_DEC:
                    fprintf(fp, "DEC ");
                    print_operand(fp, cur->intercode->u.Dec.op);
                    fprintf(fp, " ");
                    fprintf(fp, "%d", cur->intercode->u.Dec.size);
                    break;
                case IR_ARG:
                    fprintf(fp, "ARG ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
                case IR_CALL:
                    print_operand(fp, cur->intercode->u.Assign.left);
                    fprintf(fp, " := CALL ");
                    print_operand(fp, cur->intercode->u.Assign.right);
                    break;
                case IR_PARAM:
                    fprintf(fp, "PARAM ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
                case IR_READ:
                    fprintf(fp, "READ ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
                case IR_WRITE:
                    fprintf(fp, "WRITE ");
                    print_operand(fp, cur->intercode->u.OneOp.op);
                    break;
            }
            fprintf(fp, "\n");
        }
    }
}

pInterCodeHead new_intercode_head(pInterCode intercode){
    pInterCodeHead intercode_head = (pInterCodeHead)malloc(sizeof(InterCodeHead));
    intercode_head->intercode = intercode;
    intercode_head->prev = NULL;
    intercode_head->next = NULL;
    return intercode_head;
}

void delete_intercode_head(pInterCodeHead intercode_head){
    delete_intercode(intercode_head->intercode);
    free(intercode_head);
}

pInterCodeList new_intercode_list(){
    pInterCodeList intercode_list = (pInterCodeList)malloc(sizeof(InterCodeList));
    intercode_list->head = NULL;
    intercode_list->cur = NULL;
    // intercode_list->last_array_name = NULL;
    intercode_list->temp_var_num = 1;
    intercode_list->label_num = 1;
}

void delete_intercode_list(pInterCodeList intercode_list){
    pInterCodeHead intercode_head = intercode_list->head;
    pInterCodeHead delete_head;
    while(intercode_head){
        delete_head = intercode_head;
        intercode_head = intercode_head->next;
        delete_intercode_head(delete_head);
    }
    free(intercode_list);
}

void add_intercode_head(pInterCodeList intercode_list, pInterCodeHead intercode_head){
    if(intercode_list->head == NULL){
        intercode_list->head = intercode_head;
        intercode_list->cur = intercode_head;
    }
    else{
        intercode_list->cur->next = intercode_head;
        intercode_head->prev = intercode_list->cur;
        intercode_list->cur = intercode_head;
    }
}

pArg new_arg(pOperand operand){
    pArg arg = (pArg)malloc(sizeof(Arg));
    arg->op = operand;
    arg->next = NULL;
}

void delete_arg(pArg arg){
    delete_operand(arg->op);
    free(arg);
}

pArgList new_arg_list(){
    pArgList arg_list = (pArgList)malloc(sizeof(ArgList));
    arg_list->head = NULL;
    arg_list->cur = NULL;
}

void delete_arg_list(pArgList arg_list){
    pArg arg = arg_list->head;
    pArg temp_arg;
    while(arg){
        temp_arg = arg;
        arg = arg->next;
        delete_arg(temp_arg);
    }
    free(arg_list);
}

void add_arg(pArgList arg_list, pArg arg){
    if(arg_list->head == NULL){
        arg_list->head = arg;
        arg_list->cur = arg;
    } 
    else{
        arg_list->cur->next = arg;
        arg_list->cur = arg;
    }
}

/*tool*/
pOperand new_temp_var(){
    char name[10] = {0};
    sprintf(name, "t%d", intercode_list->temp_var_num);
    intercode_list->temp_var_num++;
    pOperand temp_op = new_operand(OP_VARIABLE, new_string(name));
    return temp_op;
}

pOperand new_label(){
    char name[10] = {0};
    sprintf(name, "label%d", intercode_list->label_num);
    intercode_list->label_num++;
    pOperand temp_op = new_operand(OP_LABEL, new_string(name));
    return temp_op;
}

int get_type_size(pType type){
    if(type == NULL)
        return 0;
    else if(type->kind == BASIC)
        return 4;
    else if(type->kind == ARRAY)
        return type->u.array.size * get_type_size(type->u.array.elem);
    else if(type->kind == STRUCTURE) {
        int size = 0;
        pFieldList temp = type->u.structure.field;
        while (temp) {
            size += get_type_size(temp->type);
            temp = temp->tail;
        }
        return size;
    }
    return 0;
}

void traverse_tree_generate_intercode(pNode node){
    if(node == NULL) return;
    if(!strcmp(node->name, "ExtDefList")) translate_ExtDefList(node);
    else{
        for(int i = 0; i < node->child_num; i++){
            traverse_tree_generate_intercode(node->children[i]);
        }    
    }
}

void generate_intercode(int kind, ...){
    va_list vaList;
    va_start(vaList, kind);
    pOperand temp = NULL, result = NULL, op1 = NULL, op2 = NULL, relop = NULL;
    int size = 0;
    pInterCodeHead intercode_head = NULL;
    switch (kind) {
        case IR_LABEL:  // 单操作数
        case IR_FUNCTION:
        case IR_GOTO:
        case IR_RETURN:
        case IR_ARG:
        case IR_PARAM:
        case IR_READ:
        case IR_WRITE:
            op1 = va_arg(vaList, pOperand);
            if(op1->kind == OP_ADDRESS){
                temp = new_temp_var();
                generate_intercode(IR_READ_ADDR, temp, op1);    // 使用中间变量来读某个地址的数据
                op1 = temp;
            }
            intercode_head = new_intercode_head(new_intercode(kind, op1));
            add_intercode_head(intercode_list, intercode_head);
            break;

        case IR_ASSIGN: // 双操作数
        case IR_GET_ADDR:
        case IR_READ_ADDR:
        case IR_WRITE_ADDR:
        case IR_CALL:
            op1 = va_arg(vaList, pOperand);
            op2 = va_arg(vaList, pOperand);
            if(kind == IR_ASSIGN && (op1->kind == OP_ADDRESS || op2->kind == OP_ADDRESS)){
                if(op1->kind == OP_ADDRESS && op2->kind != OP_ADDRESS)  // a[2] = b;    写地址
                    generate_intercode(IR_WRITE_ADDR, op1, op2);
                else if (op2->kind == OP_ADDRESS && op1->kind != OP_ADDRESS)    // a = b[2];     读地址
                    generate_intercode(IR_READ_ADDR, op1, op2);
                else {
                    temp = new_temp_var();  // a[2] = b[2];  又写又读
                    generate_intercode(IR_READ_ADDR, temp, op2);
                    generate_intercode(IR_WRITE_ADDR, op1, temp);
                }
            } 
            else{
                intercode_head = new_intercode_head(new_intercode(kind, op1, op2));
                add_intercode_head(intercode_list, intercode_head);
            }
            break;

        case IR_ADD:    // 三操作数
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            result = va_arg(vaList, pOperand);
            op1 = va_arg(vaList, pOperand);
            op2 = va_arg(vaList, pOperand);
            if(op1->kind == OP_ADDRESS){
                temp = new_temp_var();
                generate_intercode(IR_READ_ADDR, temp, op1);
                op1 = temp;
            }
            if (op2->kind == OP_ADDRESS) {
                temp = new_temp_var();
                generate_intercode(IR_READ_ADDR, temp, op2);
                op2 = temp;
            }
            intercode_head = new_intercode_head(new_intercode(kind, result, op1, op2));
            add_intercode_head(intercode_list, intercode_head);
            break;

        case IR_DEC:
            op1 = va_arg(vaList, pOperand);
            size = va_arg(vaList, int);
            intercode_head = new_intercode_head(new_intercode(kind, op1, size));
            add_intercode_head(intercode_list, intercode_head);
            break;
        
        case IR_IF_GOTO:
            result = va_arg(vaList, pOperand);
            relop = va_arg(vaList, pOperand);
            op1 = va_arg(vaList, pOperand);
            op2 = va_arg(vaList, pOperand);
            intercode_head = new_intercode_head(new_intercode(kind, result, relop, op1, op2));
            add_intercode_head(intercode_list, intercode_head);
            break;
    }
}

/*translate*/
/*
ExtDefList
    ExtDef ExtDefList
    none
*/
void translate_ExtDefList(pNode node){
    if(node->child_num){
        translate_ExtDef(node->children[0]);
        translate_ExtDefList(node->children[1]);
    }
}

/*
ExtDef
    Specifier ExtDecList SEMI (没有全局变量)
    Specifier SEMI
    Specifier FunDec CompSt
*/
void translate_ExtDef(pNode node){
    if(node->child_num == 3 && !strcmp(node->children[1]->name, "FunDec")){
        translate_FunDec(node->children[1]);
        translate_CompSt(node->children[2]);
    }
}

/*
FunDec
    ID LP VarList RP
    ID LP RP
*/
void translate_FunDec(pNode node){
    if(inter_error) return;
    generate_intercode(IR_FUNCTION, new_operand(OP_FUNCTION, new_string(node->children[0]->value.str_value)));
    pItem func_item = search_item(symbol_table, node->children[0]->value.str_value);
    pFieldList temp_argv = func_item->field->type->u.function.argv; // 处理函数参数声明
    while(temp_argv){
        generate_intercode(IR_PARAM, new_operand(OP_VARIABLE, new_string(temp_argv->name)));
        temp_argv = temp_argv->tail;
    }
}

/*
CompSt
    LC DefList StmtList RC
*/
void translate_CompSt(pNode node){
    if(inter_error) return;
    pNode deflist = node->children[1];
    pNode stmtlist = node->children[2];
    translate_DefList(deflist);
    translate_StmtList(stmtlist);
}

/*
DefList
    Def DefList
    none
*/
void translate_DefList(pNode node){
    if(inter_error) return;
    if(node->child_num){
        translate_Def(node->children[0]);
        translate_DefList(node->children[1]);
    }
}

/*
StmList
    Stmt StmtList
    none
*/
void translate_StmtList(pNode node){
    if(inter_error) return;
    if(node->child_num){
        translate_Stmt(node->children[0]);
        translate_StmtList(node->children[1]);
    }
}

/*
Stmt
    Exp SEMI
    CompSt
    RETURN Exp SEMI
    IF LP Exp RP Stmt
    IF LP Exp RP Stmt ELSE Stmt
    WHILE LP Exp RP Stmt
*/
void translate_Stmt(pNode node){
    if(inter_error) return;

    // Stmt -> Exp SEMI
    if (!strcmp(node->children[0]->name, "Exp")) translate_Exp(node->children[0], NULL);

    // Stmt -> CompSt
    else if (!strcmp(node->children[0]->name, "CompSt")) translate_CompSt(node->children[0]);

    // Stmt -> RETURN Exp SEMI
    else if (!strcmp(node->children[0]->name, "RETURN")) {
        pOperand t1 = new_temp_var();
        translate_Exp(node->children[1], t1);   // 先求值
        generate_intercode(IR_RETURN, t1);            // 再return
    }

    // Stmt -> IF LP Exp RP Stmt
    // Stmt -> IF LP Exp RP Stmt ELSE Stmt
    else if (!strcmp(node->children[0]->name, "IF")) {
        pNode exp = node->children[2];
        pNode stmt = node->children[4];
        pOperand label1 = new_label();  // true
        pOperand label2 = new_label();  // false

        translate_Cond(exp, label1, label2);    // 翻译条件
        generate_intercode(IR_LABEL, label1);
        translate_Stmt(stmt);                   // 翻译语句（第一个stmt就是true对应的语句）
        // Stmt -> IF LP Exp RP Stmt
        if (node->child_num == 5) {
            generate_intercode(IR_LABEL, label2);
        }
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        else {
            pOperand label3 = new_label();
            generate_intercode(IR_GOTO, label3);    // 跳出if
            generate_intercode(IR_LABEL, label2);
            translate_Stmt(node->children[6]);
            generate_intercode(IR_LABEL, label3);   // if 后
        }
    }

    // Stmt -> WHILE LP Exp RP Stmt
    else if (!strcmp(node->children[0]->name, "WHILE")) {
        pOperand label1 = new_label();  // while 判断前
        pOperand label2 = new_label();  // while 判断后
        pOperand label3 = new_label();  // while 后

        generate_intercode(IR_LABEL, label1);
        translate_Cond(node->children[2], label2, label3);
        generate_intercode(IR_LABEL, label2);
        translate_Stmt(node->children[4]);
        generate_intercode(IR_GOTO, label1);
        generate_intercode(IR_LABEL, label3);
    }
}

/*
Def
    Specifier DecList SEMI      变量声明
*/
void translate_Def(pNode node){
    if(inter_error) return;
    translate_DecList(node->children[1]);
}

/*
DecList
    Dec
    Dec COMMA DecList
*/
void translate_DecList(pNode node){
    if(inter_error) return;
    pNode temp = node;
    while (temp)
    {
        translate_Dec(temp->children[0]);
        if(temp->child_num==3) temp=temp->children[2];
        else break;
    }  
}

/*
Dec
    VarDec              不赋值
    VarDec ASSIGNP Exp  赋值
*/
void translate_Dec(pNode node){
    if(inter_error) return;

    // Dec -> VarDec
    if(node->child_num==1){
        translate_VarDec(node->children[0], NULL);
    }

    // Dec -> VarDec ASSIGNOP Exp
    else{
        pOperand op1 = new_temp_var();
        translate_VarDec(node->children[0], op1);   // 翻译声明
        pOperand op2 = new_temp_var();
        translate_Exp(node->children[2], op2);      // 翻译表达式
        generate_intercode(IR_ASSIGN, op1, op2);    // 翻译赋值
    }
}

/*
VarDec
    ID
    VarDec LB INT RB
*/
void translate_VarDec(pNode node, pOperand operand){
    if(inter_error) return;

    // VarDec -> ID
    if(node->child_num==1){
        pItem item = search_item(symbol_table, node->children[0]->value.str_value);
        pType type = item->field->type;
        if(type->kind == BASIC){
            if(operand){    // 如果符号表中存在该变量，则使用该变量名
                intercode_list->temp_var_num--;
                set_operand(operand, OP_VARIABLE, new_string(item->field->name));
            }
        }
        else if(type->kind == ARRAY){
            // 没有高维数组
            if(type->u.array.elem->kind == ARRAY){
                inter_error = true;
                printf("Cannot translate: Code containsvariables of multi-dimensional array type or parameters of array type.\n");
                return;
            }
            else{
                generate_intercode(IR_DEC, new_operand(OP_VARIABLE, new_string(item->field->name)), get_type_size(type));
            }
        }
        else if(type->kind == STRUCTURE){
            // 要求3.1，定义结构体空间
            generate_intercode(IR_DEC, new_operand(OP_VARIABLE, new_string(item->field->name)), get_type_size(type));
        }
    }

    // VarDec->VarDec LB INT RB
    else{
        translate_VarDec(node->children[0], operand);
    }
    
}

/*
Exp
    LP Exp RP

    NOT Exp
    Exp AND Exp
    Exp OR Exp
    Exp RELOP Exp

    Exp ASSIGNOP Exp
    Exp PLUS Exp
    Exp MINUS Exp
    Exp STAR Exp
    Exp DIV Exp
    
    
    Exp LB Exp RB
    Exp DOT ID

    MINUS Exp

    ID LP Args RP
    ID LP RP

    ID
    INT
    FLOAT    
*/
void translate_Exp(pNode node, pOperand operand){
    if(inter_error) return;
    
    // LP Exp RP
    if(!strcmp(node->children[0]->name, "LP")) translate_Exp(node->children[1], operand);

    else if(!strcmp(node->children[0]->name, "Exp") || !strcmp(node->children[0]->name, "NOT")){
        // 条件表达式 基本表达式
        if(strcmp(node->children[1]->name, "LB") && strcmp(node->children[1]->name, "DOT")){
            if (!strcmp(node->children[1]->name, "AND") ||  // 条件，结果是1或0，使用if来
                !strcmp(node->children[1]->name, "OR") ||
                !strcmp(node->children[1]->name, "RELOP") ||
                !strcmp(node->children[0]->name, "NOT")){
                    pOperand label1 = new_label();
                    pOperand label2 = new_label();
                    pOperand true_num = new_operand(OP_CONSTANT, 1);
                    pOperand false_num = new_operand(OP_CONSTANT, 0);
                    generate_intercode(IR_ASSIGN, operand, false_num);
                    translate_Cond(node, label1, label2);
                    generate_intercode(IR_LABEL, label1);
                    generate_intercode(IR_ASSIGN, operand, true_num);
                    generate_intercode(IR_LABEL, label2);   // warning 1
            }
            else{
                // Exp ASSIGNOP Exp
                pOperand t1 = new_temp_var();
                translate_Exp(node->children[0], t1);
                pOperand t2 = new_temp_var();
                translate_Exp(node->children[2], t2);
                if(!strcmp(node->children[1]->name, "ASSIGNOP")) generate_intercode(IR_ASSIGN, t1, t2);
                else{
                    // Exp -> Exp PLUS Exp
                    if(!strcmp(node->children[1]->name, "PLUS")) generate_intercode(IR_ADD, operand, t1, t2);
                    
                    // Exp -> Exp MINUS Exp
                    else if(!strcmp(node->children[1]->name, "MINUS")) generate_intercode(IR_SUB, operand, t1, t2);
                    
                    // Exp -> Exp STAR Exp
                    else if(!strcmp(node->children[1]->name, "STAR")) generate_intercode(IR_MUL, operand, t1, t2);

                    // Exp -> Exp DIV Exp
                    else if(!strcmp(node->children[1]->name, "DIV")) generate_intercode(IR_DIV, operand, t1, t2);
                }
            }
        }
        
        // 数组 结构体
        else{
            // Exp -> Exp LB Exp RB
            if(!strcmp(node->children[1]->name, "LB")){
                if(node->children[0]->child_num >= 2 && !strcmp(node->children[0]->children[1]->name, "LB")){
                    // 多维数组
                    inter_error = true;
                    printf("Cannot translate: Code containsvariables of multi-dimensional array type or parameters of array type.\n");
                    return;
                }
                else{
                    pOperand idx = new_temp_var();
                    translate_Exp(node->children[2], idx);
                    pOperand base = new_temp_var();
                    translate_Exp(node->children[0], base);
                    pOperand width, target, offset = new_temp_var();
                    pItem item = search_item(symbol_table, base->u.name);
                    width = new_operand(OP_CONSTANT, get_type_size(item->field->type->u.array.elem));
                    generate_intercode(IR_MUL, offset, idx, width); // 计算偏移量
                    if(base->kind == OP_VARIABLE){  // 非结构体数组，基址是一个变量
                        target = new_temp_var();
                        generate_intercode(IR_GET_ADDR, target, base);  // 获取变量地址
                    }
                    else target = base; // 结构体数组，基址就是地址
                    generate_intercode(IR_ADD, operand, target, offset);
                    operand->kind=OP_ADDRESS;
                    // intercode_list->last_array_name = base->u.name;
                }
            }

            // Exp -> Exp DOT ID 要求3.1，访问结构体数据
            else{
                pOperand temp = new_temp_var();
                translate_Exp(node->children[0], temp);
                pOperand target;
                if(temp->kind == OP_ADDRESS) target = new_operand(temp->kind, temp->u.name);
                else{
                    target = new_temp_var();
                    generate_intercode(IR_GET_ADDR, target, temp);
                }

                pOperand id = new_operand(OP_VARIABLE, new_string(node->children[2]->value.str_value));
                int offset = 0;
                pItem item = search_item(symbol_table, temp->u.name);

                // if(!item) item = search_item(symbol_table, intercode_list->last_array_name);

                pFieldList temp2;

                if (item->field->type->kind == ARRAY) {
                    temp2 = item->field->type->u.array.elem->u.structure.field;
                }

                else {
                    temp2 = item->field->type->u.structure.field;
                }

                while (temp2) {
                    if (!strcmp(temp2->name, id->u.name)) break;
                    offset += get_type_size(temp2->type);
                    temp2 = temp2->tail;
                }

                pOperand tOffset = new_operand(OP_CONSTANT, offset);
                if (operand) {
                    generate_intercode(IR_ADD, operand, target, tOffset);
                    set_operand(operand, OP_ADDRESS, (void*)new_string(id->u.name));
                }
            }
        }
    }

    // 单目运算
    // Exp -> MINUS Exp
    else if(!strcmp(node->children[0]->name, "MINUS")){
        pOperand t1 = new_temp_var();
        translate_Exp(node->children[1], t1);
        pOperand zero = new_operand(OP_CONSTANT, 0);
        generate_intercode(IR_SUB, operand, zero, t1);
    }
    // Exp -> ID LP Args RP
    //        ID LP RP
    else if(!strcmp(node->children[0]->name, "ID") && node->child_num>=3){
        pOperand temp_func = new_operand(OP_FUNCTION, new_string(node->children[0]->value.str_value));
        if(node->child_num == 4){
            pArgList arg_list = new_arg_list();
            translate_Args(node->children[2], arg_list);
            if(!strcmp(node->children[0]->value.str_value, "write")) generate_intercode(IR_WRITE, arg_list->head->op);
            else{
                pArg temp_arg = arg_list->head;
                while(temp_arg){
                    if(temp_arg->op->kind == OP_VARIABLE){    // 结构体传地址
                        pItem item = search_item(symbol_table, temp_arg->op->u.name);
                        if (item && item->field->type->kind == STRUCTURE) {
                            pOperand temp_var = new_temp_var();
                            generate_intercode(IR_GET_ADDR, temp_var, temp_arg->op);
                            pOperand temp_var_copy = new_operand(OP_ADDRESS_STRUCT, temp_var->u.name);  // 要求3.1，传参的时候传结构体指针的指针
                            generate_intercode(IR_ARG, temp_var_copy);
                        }
                    }
                    else generate_intercode(IR_ARG, temp_arg->op);
                    temp_arg = temp_arg -> next;
                }
                if(operand) generate_intercode(IR_CALL, operand, temp_func);
                else{
                    pOperand temp = new_temp_var();
                    generate_intercode(IR_CALL, temp, temp_func);
                }
            }
        }
        else{
            if(!strcmp(node->children[0]->value.str_value, "read")) generate_intercode(IR_READ, operand);
            else{
                if(operand) generate_intercode(IR_CALL, operand, temp_func);
                else{
                    pOperand temp = new_temp_var();
                    generate_intercode(IR_CALL, temp, temp_func);
                }
            }
        }
    }

    // Exp -> ID
    else if(!strcmp(node->children[0]->name, "ID") && node->child_num==1){
        pItem item = search_item(symbol_table, node->children[0]->value.str_value);
        intercode_list->temp_var_num--;
        // 结构体传地址
        if(item->field->is_arg && item->field->type->kind == STRUCTURE) set_operand(operand, OP_ADDRESS, new_string(node->children[0]->value.str_value));
        else set_operand(operand, OP_VARIABLE, new_string(node->children[0]->value.str_value));
    }
    // FLOAT INT
    else{
        intercode_list->temp_var_num--;
        set_operand(operand, OP_CONSTANT, node->children[0]->value.int_value);
    }
}

void translate_Cond(pNode node, pOperand label_true, pOperand label_false){
    if(inter_error) return;

    // Exp->NOT Exp
    if(!strcmp(node->children[0]->name, "NOT")) translate_Cond(node->children[1], label_false, label_true);

    // Exp->Exp RELOP Exp
    else if(!strcmp(node->children[1]->name, "RELOP")){
        pOperand t1 = new_temp_var();
        pOperand t2 = new_temp_var();
        translate_Exp(node->children[0], t1);
        translate_Exp(node->children[2], t2);

        pOperand relop = new_operand(OP_RELOP, new_string(node->children[1]->value.str_value));

        if(t1->kind == OP_ADDRESS){
            pOperand temp = new_temp_var();
            generate_intercode(IR_READ_ADDR, temp, t1);
            t1 = temp;
        }
        if (t2->kind == OP_ADDRESS) {
            pOperand temp = new_temp_var();
            generate_intercode(IR_READ_ADDR, temp, t2);
            t2 = temp;
        }

        generate_intercode(IR_IF_GOTO, t1, relop, t2, label_true);
        generate_intercode(IR_GOTO, label_false);
    }

    // Exp -> Exp AND Exp
    else if(!strcmp(node->children[1]->name, "AND")){
        pOperand label1 = new_label();
        translate_Cond(node->children[0], label1, label_false); // 假直接跳出
        generate_intercode(IR_LABEL, label1);
        translate_Cond(node->children[2], label_true, label_false);
    }

    // Exp -> Exp OR Exp
    else if(!strcmp(node->children[1]->name, "OR")){
        pOperand label1 = new_label();
        translate_Cond(node->children[0], label_true, label1);
        generate_intercode(IR_LABEL, label1);
        translate_Cond(node->children[2], label_true, label_false);
    }

    else{   // 生成中间代码块：如果node为真（不等于0），则跳转至true，否则跳转至false
        pOperand t1 = new_temp_var();   // 中间变量取node的值
        translate_Exp(node, t1);
        pOperand t2 = new_operand(OP_CONSTANT, 0);  // 构造取等判断
        pOperand relop = new_operand(OP_RELOP, new_string("!="));

        if (t1->kind == OP_ADDRESS) {
            pOperand temp = new_temp_var();
            generate_intercode(IR_READ_ADDR, temp, t1); // 地址特殊处理
            t1 = temp;
        }
        generate_intercode(IR_IF_GOTO, t1, relop, t2, label_true);
        generate_intercode(IR_GOTO, label_false);
    }
}

/*
Args
    Exp COMMA Args
    Exp
*/
void translate_Args(pNode node, pArgList arg_list){
    if(inter_error) return;
    // Args -> Exp
    pArg temp_arg = new_arg(new_temp_var());
    translate_Exp(node->children[0], temp_arg->op);
    if(temp_arg->op->kind == OP_VARIABLE){
        pItem item = search_item(symbol_table, temp_arg->op->u.name);
        if(item && item->field->type->kind == ARRAY){
            inter_error = true;
            printf("Cannot translate: Code containsvariables of multi-dimensional array type or parameters of array type.\n");
            return;
        }
    }
    add_arg(arg_list, temp_arg);

    // Args -> Exp COMMA Args
    if(node->child_num>1) translate_Args(node->children[2], arg_list);
}