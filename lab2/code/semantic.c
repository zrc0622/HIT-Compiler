#include "semantic.h"

Type* new_type(Kind kind, ...)  // 初始化节点类型
{
    Type* ptype = (Type*)malloc(sizeof(Type));
    ptype->kind = kind;
    va_list vaList;
    switch (kind)
    {
    case BASIC:
        va_start(vaList, kind);
        ptype->u.basic = va_arg(vaList, Basic);
        break;
    case ARRAY:
        va_start(vaList, kind);
        ptype->u.array.elem = va_arg(vaList, Type*);
        ptype->u.array.size = va_arg(vaList, int);
        break;
    case STRUCTURE:
        va_start(vaList, kind);
        ptype->u.structure.name = va_arg(vaList, char*);
        ptype->u.structure.field = va_arg(vaList, FieldList*);
        break;
    case FUNCTION:
        // TODO
        break;
    default:
        break;
    }
}

void free_type(Type* specifier_type)
{
    // TODO
}

HashItem* new_item()
{
    // TODO
}

/***********************************************************************************/

void TraverseTree(Node* node)   // 对语法树进行遍历，并提炼插入到符号表里
{
    if(!strcmp(node->name, "ExtDef")) ExtDef(node);
    // 递归处理子节点
    int child_num = node->child_num;
    for(int i=0; i<child_num; i++){
        TraverseTree(node->children[i]);
    }
}

/*
ExtDef  (Ext 外部)
    Specifier ExtDecList SEMI   全局变量定义
    Specifier FunDec CompSt     函数定义
    Specifier SEMI              全局struct定义
*/
void ExtDef(Node* node)
{
    Type* specifier_type = Specifier(node->children[0]);
    char* name1 = node->children[1]->name;
    if(!strcmp(name1, "ExtDecList")) ExtDecList(node->children[1], specifier_type);
    else if(!strcmp(name1, "FunDec"))
    {
        FunDec(node->children[1], specifier_type);
        CompSt(node->children[2], specifier_type);
    }
    if(specifier_type) free_type(specifier_type);   // 用完释放
}

/*
Specifier
    TYPE
    StructSpecifier
*/
Type* Specifier(Node* node)
{
    Node* child_node = node->children[0];
    if(!strcmp(child_node->name, "TYPE")){
        switch (child_node->type)
        {
        case LEX_FLOAT:
            return new_type(BASIC, FLOAT_TYPE);
            break;
        case LEX_INT:
            return new_type(BASIC, INT_TYPE);
            break;
        default:
            break;
        }
    }
    else return StructSpecifier(child_node);
}

/*
StructSpecifier
    STRUCT OptTag LC DefList RC 注意OptTag是可选的
    STRUCT Tag
*/
Type* StructSpecifier(Node* node)
{
    Node* child1 = node->children[1];
    if(strcmp(child1->name, "Tag")){    // 包括了OptTag和LC（OptTag为空）
        // TODO
    }
    // TODO
}

void ExtDecList(Node* node, Type* specifier_type)
{
    // TODO
}

void FunDec(Node* node, Type* specifier_type)
{
    // TODO
}

void CompSt(Node* node, Type* specifier_type)
{
    // TODO
}