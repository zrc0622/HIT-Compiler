#include "tree.h"
#include "malloc.h"
#include <stdarg.h>
#include <stdlib.h>

Node* MakeLeafNode(char* name, NodeEnum type, char* value, int lineno) // 创建叶节点
{
    Node* leaf_node = (Node*)malloc(sizeof(Node));
    leaf_node->name = name;
    leaf_node->type = type;
    switch (type) {
        case LEX_INT:
            leaf_node->value.int_value = strtol(value, NULL, 0); // 强制类型转换后取值
            break;
        case LEX_FLOAT:
            leaf_node->value.float_value = strtof(value, NULL);
            break;
        default:
            if(value) strcpy(leaf_node->value.str_value, value); // 只有int和float不使用字符串，检测value防止段错误
            break;
    }
    leaf_node->lineno = lineno;
    leaf_node->child_num = 0; // 叶节点没有孩子
    leaf_node->children = NULL;
    return leaf_node;
}

Node* MakeNode(char* name, NodeEnum type, char* value, int lineno, int child_num, Node** children) // 创建中间节点
{
    Node* internal_node = MakeLeafNode(name, type, value, lineno);
    internal_node->child_num = child_num;
    internal_node->children = children;
    return internal_node;
}

void PrintIndentation(int depth)
{
    for(int i=0; i<depth; i++) printf("  ");
    // printf("|— ");
}

void PrintTree(Node* root, int depth) // 打印树，参数：根节点、深度
{
    if(root->type != SYN_NULL) PrintIndentation(depth);
    switch (root->type)
    {
    case SYN_NO_NULL:
        printf("%s (%d)\n", root->name, root->lineno);
        break;
    case SYN_NULL:
        break;
    case LEX_ID:
        printf("%s: %s\n", root->name, root->value.str_value);
        break;
    case LEX_TYPE:
        printf("%s: %s\n", root->name, root->value.str_value);
        break;
    case LEX_INT:
        printf("%s: %d\n", root->name, root->value.int_value);
        break;
    case LEX_FLOAT:
        printf("%s: %f\n", root->name, root->value.float_value);
        break;
    case LEX_OTHERS:
        printf("%s\n", root->name);
    }
    // 递归调用
    for(int i=0; i<root->child_num; i++)
    {
        PrintTree(root->children[i], depth+1);
    }
}