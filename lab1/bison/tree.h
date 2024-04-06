#include <stdio.h>
#include <string.h>

// // 儿子节点最大数量
// const int MaxChildNum=10;

// 定义树的节点的类型（枚举类型）
typedef enum{
    /*语法单元，额外打印行号*/
    SYN, // 语法单元
    SYN_NULL, // 产生空串的语法单元
    /*词法单元*/
    ID, // ID: 额外打印该标识符所对应的词素
    TYPE, // TYPE: 额外打印int or float
    INT, // 额外打印具体值
    FLOAT, // 额外打印具体值
    OTHERS
} NodeEnum;

// 定义树的节点
typedef struct Node{
    char* name; // 节点名（指针）
    NodeEnum type; // 节点类型
    union {char str_value[32]; int int_value; float float_value;} value;
    int lineno; // 词法单元行号
    int child_num;
    struct Node** children; // 子节点数组（指针数组）
} Node;

// 定义函数
Node* MakeLeafNode(char* name, NodeEnum type, void* value, int lineno); // 创建叶节点，输入编号和词法值
Node* MakeNode(char* name, NodeEnum type, void* value, int lineno, int child_num, Node** children); // 创建中间节点，输入归约产生式和孩子数
void PrintIndentation(int depth); // 打印缩进的函数
void PrintTree(Node* root, int depth); // 打印树，参数：根节点、深度
