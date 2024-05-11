#ifndef INTER_H
#define INTER_H
#include "tree.h"
#include "semantic.h"
#include <stdbool.h>
#include <stdarg.h>

typedef struct Operand* pOperand;
typedef struct InterCode* pInterCode;
typedef struct InterCodeHead* pInterCodeHead;
typedef struct InterCodeList* pInterCodeList;
typedef struct Arg* pArg;
typedef struct ArgList* pArgList;

typedef struct Type* pType;
typedef struct Node* pNode;
typedef struct FieldList* pFieldList;
typedef struct HashItem* pItem;

extern bool inter_error;
extern pInterCodeList intercode_list;   // 声明中间代码双向链表

/*数据结构*/
typedef struct Operand{ // 操作数
    enum {
        OP_VARIABLE,
        OP_CONSTANT,
        OP_ADDRESS,
        OP_LABEL,
        OP_FUNCTION,
        OP_RELOP,
        OP_ADDRESS_STRUCT,  // 要求3.1，结构体指针
    } kind; // 操作数的种类

    union {
        int value;
        char* name;
    } u;    // 根据操作数种类存储不同的数据

} Operand;

typedef struct InterCode{  // 中间代码
    enum {
        IR_LABEL,
        IR_FUNCTION,
        IR_ASSIGN,
        IR_ADD,
        IR_SUB,
        IR_MUL,
        IR_DIV,
        IR_GET_ADDR,
        IR_READ_ADDR,
        IR_WRITE_ADDR,
        IR_GOTO,
        IR_IF_GOTO,
        IR_RETURN,
        IR_DEC,
        IR_ARG,
        IR_CALL,
        IR_PARAM,
        IR_READ,
        IR_WRITE,
    } kind; // 中间代码种类

    union {
        struct {
            pOperand op;
        } OneOp;    // 单操作数的数据
        
        struct {
            pOperand right, left;
        } Assign;   // 赋值语句的左右操作数
        
        struct {
            pOperand result, op1, op2;
        } BinOp;    // 二元操作语句的结果和操作数
        
        struct {
            pOperand x, relop, y, z;
        } IfGoto;   // 条件跳转指令的数据
        
        struct {
            pOperand op;
            int size;
        } Dec;      // 声明语句的变量和大小
    } u;

} InterCode;

typedef struct InterCodeHead{   // 中间代码双向链表头
    pInterCode intercode;
    pInterCodeHead prev, next;  // 前后
} InterCodeHead;

typedef struct InterCodeList{   // 中间代码双向链表
    pInterCodeHead head;
    pInterCodeHead cur;
    // char* last_array_name;  // 
    int temp_var_num;       // 临时变量的数量，用于给临时变量命名
    int label_num;          // 标签的数量，用于给标签命名
} InterCodeList;

typedef struct Arg{ // 参数链表
    pOperand op;
    pArg next;
} Arg;

typedef struct ArgList{
    pArg head;
    pArg cur;
} ArgList;

/*数据结构维护函数*/
/*operand 维护函数*/
pOperand new_operand(int kind, ...);            // 创建新的操作数
void delete_operand(pOperand operand);          // 删除操作数
void set_operand(pOperand operand, int kind, ...);  // 设置操作数的值
void print_operand(FILE* fp, pOperand operand); // 打印操作数到文件中

/*intercode list 维护函数*/
/*intercode*/
pInterCode new_intercode(int kind, ...); // 创建新的中间代码
void delete_intercode(pInterCode intercode); // 删除中间代码

/*intercode head*/
pInterCodeHead new_intercode_head(pInterCode intercode); // 创建新的中间代码链头
void delete_intercode_head(pInterCodeHead intercode_head);  // 删除中间代码链头

/*intercode list*/
void print_intercode_list(FILE* fp, pInterCodeList intercode_list); // 打印中间代码链表
pInterCodeList new_intercode_list();    // 创建新的中间代码链表
void delete_intercode_list(pInterCodeList intercode_list);  // 删除中间代码链表
void add_intercode_head(pInterCodeList intercode_list, pInterCodeHead intercode_head);  // 将新的中间代码添加到链表中

/*arg list 维护函数*/
/*arg*/
pArg new_arg(pOperand operand); // 创建新的参数
void delete_arg(pArg arg); // 删除参数

/*arg list*/
pArgList new_arg_list(); // 创建新的参数列表
void delete_arg_list(pArgList arg_list); // 删除参数列表
void add_arg(pArgList arg_list, pArg arg); // 向参数列表中添加参数

/*中间代码生成相关函数*/
/*功能函数*/
pOperand new_temp_var();    // 创建临时变量存储中间结果
pOperand new_label();   // 创建新的标签为跳转服务

int get_type_size(pType type);  // 获取类型的尺寸
void traverse_tree_generate_intercode(pNode node); // 遍历语法树
void generate_intercode(int kind, ...); // 调用new_intercode生成中间代码: 一行代码对应多行中间代码，new_intercode生成单行中间代码，而generate_intercode对一行代码生成多行中间代码

/*语法单元翻译函数*/
void translate_ExtDefList(pNode node);      // 翻译外部定义列表
void translate_ExtDef(pNode node);          // 翻译外部定义
void translate_FunDec(pNode node);          // 翻译函数声明
void translate_CompSt(pNode node);          // 翻译复合语句
void translate_DefList(pNode node);         // 翻译定义列表
void translate_StmtList(pNode node);        // 翻译语句列表
void translate_Stmt(pNode node);            // 翻译语句
void translate_Def(pNode node);             // 翻译定义
void translate_DecList(pNode node);         // 翻译声明列表
void translate_Dec(pNode node);             // 翻译单个声明
void translate_VarDec(pNode node, pOperand operand);        // 翻译变量声明
void translate_Exp(pNode node, pOperand operand);           // 翻译表达式
void translate_Cond(pNode node, pOperand label_true, pOperand label_false);    // 翻译条件
void translate_Args(pNode node, pArgList arg_list);          // 翻译参数
#endif