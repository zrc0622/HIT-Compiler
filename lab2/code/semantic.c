#include "semantic.h"

CrossTable* symbol_table;   // 全局符号表，在main中初始化

CrossTable* init_cross_table()
{
    CrossTable* ptable = (CrossTable*)malloc(sizeof(CrossTable));
    ptable->hash_table = new_hash_table();
    ptable->stack = new_stack();
    ptable->unnamed_struct = 0;
    symbol_table=ptable;
    return ptable;
}

char* new_string(const char* src)   // 创建一个字符串副本
{ 
    if (src == NULL) return NULL;
    size_t length = strlen(src) + 1;
    char* memory = (char*)malloc(length * sizeof(char));
    if (memory == NULL) return NULL;
    strncpy(memory, src, length);
    return memory;
}

/***********************************************************************************/

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
    va_end(vaList);
    return ptype;
}

void free_type(Type* specifier_type)
{
    // TODO
}

HashItem* new_item(int scope_layer, FieldList* pfield)  // 初始化hash表项
{
    HashItem* pitem = (HashItem*)malloc(sizeof(HashItem));
    pitem->scope_layer=scope_layer;
    pitem->field=pfield;
    pitem->next_hash_item=NULL;
    pitem->next_layer_item=NULL;
    return pitem;
}

FieldList* new_fieldlist(char* name, Type* type)
{
    FieldList* p = (FieldList*)malloc(sizeof(FieldList));
    p->name = new_string(name); // dif
    p->type = type;
    p->tail = NULL;
    return p;
}

void set_fieldlistname(FieldList* p, char* name)
{
    if(p->name != NULL) free(p->name);  
    p->name = new_string(name);
}

HashTable* new_hash_table()
{
    HashTable* ptable = (HashTable*)malloc(sizeof(HashTable));
    ptable->hash_array = (HashItem**)malloc(sizeof(HashItem*)*SYMBOL_TABLE_SIZE);
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++) ptable->hash_array[i] = NULL;
    return ptable;
}

Stack* new_stack()
{
    Stack* p = (Stack*)malloc(sizeof(Stack));
    p->stack_array = (HashItem**)malloc(sizeof(HashItem*)*SYMBOL_TABLE_SIZE);
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++)  p->stack_array[i] = NULL;
    p->stack_layer=0;
    return p;
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
    STRUCT OptTag LC DefList RC 注意OptTag是可选的，用于定义结构体
    STRUCT Tag  用于初始化变量
OptTag
    ID
    none
*/
Type* StructSpecifier(Node* node)
{
    Node* child1 = node->children[1];   // OptTag
    Node* child3 = node->children[3];   // DefList
    // 定义结构体
    if(strcmp(child1->name, "Tag")){
        HashItem* struct_item = new_item(symbol_table->stack->stack_layer, new_fieldlist("", new_type(STRUCTURE, NULL, NULL)));
        // 命名结构体类型
        if(child1->type==SYN_NO_NULL)   // OptTag产生ID
        {
            set_fieldlistname(struct_item->field, child1->children[0]->value.str_value);
        }
        else    // OptTag产生空
        {
            symbol_table->unnamed_struct++;
            char new_name[20] = {0};
            sprintf(new_name, "%d", symbol_table->unnamed_struct);
            set_fieldlistname(struct_item->field, new_name);    // 使用数字进行命名，确保不会产生冲突
        }

        // 处理结构体内部参数
        DefList(child3, struct_item);
    }
    // 初始化变量
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

void DefList(Node* node, HashItem* struct_item)
{
    
}