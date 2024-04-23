#include "tree.h"
#include <stdarg.h>
#include <stdlib.h> 

/*define*/
#define SYMBOL_TABLE_SIZE 0x3fff  // 符号表大小

/*数据结构*/
/*enum*/
typedef enum Kind {BASIC, ARRAY, STRUCTURE, FUNCTION} Kind; // 类型表示：int、float，数组，结构体，函数
typedef enum Basic {INT_TYPE, FLOAT_TYPE} Basic;

/*struct*/
typedef struct Type Type;
typedef struct FieldList FieldList;
typedef struct HashItem HashItem;
typedef struct HashTable HashTable;
typedef struct Stack Stack;
typedef struct CrossTable CrossTable;

typedef struct Type // 类型：specifier type
{
    Kind kind;

    union
    {
        Basic basic;

        struct
        {
            Type* elem; // 元素类型
            int size;   // 数组大小
        } array;    // 链表表示多维数组

        struct
        {
            char* name;
            FieldList* field;
        } structure;    // 链表表示结构体
        
        struct
        {
            // TODO
        } function; // 函数      
    } u;
} Type;

typedef struct FieldList
{
    char* name;     // 域的名字
    Type* type;     // 域的类型
    FieldList* tail;    // 下一个域
} FieldList;

// Hash表项
typedef struct HashItem
{
    int scope_layer;    // 作用域层数
    HashItem* next_hash_item;   // 下一个hash表项（横向链）
    HashItem* next_layer_item;  // 下一个同层的hash表项（纵向链）
    FieldList* field;
} HashItem;

// Hash表头
typedef struct HashTable
{
    HashItem** hash_array;  // hash表头
} HashTable;

// Stack表头
typedef struct Stack
{
    HashItem** stack_array; // stack表头
    int stack_layer;    // 当前栈深
} Stack;

// 十字链表 = Hash table + stack
typedef struct CrossTable
{
    HashTable* hash_table;
    Stack* stack;
    int unnamed_struct;
} CrossTable;


/*函数声明*/
CrossTable* init_cross_table();
char* new_string(const char* src);

Type* new_type(Kind kind, ...);
void free_type(Type* specifier_type);

void TraverseTree(Node* node);
void ExtDef(Node* node);
Type* Specifier(Node* node);
Type* StructSpecifier(Node* node);
void ExtDecList(Node* node, Type* specifier_type);
void FunDec(Node* node, Type* specifier_type);
void CompSt(Node* node, Type* specifier_type);
HashTable* new_hash_table();
Stack* new_stack();
void DefList(Node* node, HashItem* struct_item);