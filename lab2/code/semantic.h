#include "tree.h"
#include <stdarg.h>
#include <stdlib.h> 

/*数据结构*/
/*enum*/
typedef enum Kind {BASIC, ARRAY, STRUCTURE, FUNCTION} Kind; // 类型表示：int、float，数组，结构体，函数
typedef enum Basic {INT_TYPE, FLOAT_TYPE} Basic;

/*struct*/
typedef struct Type Type;
typedef struct FieldList FieldList;
typedef struct HashItem HashItem;

typedef struct Type // 类型
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

typedef struct HashItem
{
    // TODO
} HashItem;


/*函数声明*/
Type* new_type(Kind kind, ...);
void free_type(Type* specifier_type);

void TraverseTree(Node* node);
void ExtDef(Node* node);
Type* Specifier(Node* node);
Type* StructSpecifier(Node* node);
void ExtDecList(Node* node, Type* specifier_type);
void FunDec(Node* node, Type* specifier_type);
void CompSt(Node* node, Type* specifier_type);