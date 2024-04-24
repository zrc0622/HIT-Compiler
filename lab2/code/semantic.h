#include "tree.h"
#include <stdarg.h>
#include <stdlib.h> 
#include <stdbool.h>

/*define*/
#define SYMBOL_TABLE_SIZE 0x3fff  // 符号表大小

/*数据结构*/
// enum
typedef enum Kind {BASIC, ARRAY, STRUCTURE, FUNCTION} Kind; // 类型表示：int、float，数组，结构体，函数
typedef enum Basic {INT_TYPE, FLOAT_TYPE} Basic;
typedef enum ErrorType {   // 错误类型
    UNDEF_VAR = 1,          // 未定义的变量
    UNDEF_FUNC,             // 未定义的函数
    REDEF_VAR,              // 变量重定义
    REDEF_FUNC,             // 函数重定义
    TYPE_MISMATCH_ASSIGN,   // 分配时类型不匹配
    LEFT_VAR_ASSIGN,        // 赋值的左侧必须是变量
    TYPE_MISMATCH_OP,       // 操作数类型不匹配
    TYPE_MISMATCH_RETURN,   // 返回值类型不匹配
    FUNC_AGRC_MISMATCH,     // 函数参数不适用
    NOT_A_ARRAY,            // 变量不是数组
    NOT_A_FUNC,             // 变量不是函数
    NOT_A_INT,              // 变量不是整数
    ILLEGAL_USE_DOT,        // 非法使用“.”
    NONEXISTFIELD,          // 非存在的字段
    REDEF_FEILD,            // 字段重定义
    DUPLICATED_NAME,        // 名称重复: 错误类型3、16
    UNDEF_STRUCT            // 未定义的结构体: 错误类型17
} ErrorType;

// struct
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
    FieldList* field;   // 具体的信息
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
char* new_string(const char* src);
unsigned int hash_pjw(char* name);

HashItem* search_item(CrossTable* symbol_table, char* name);
bool check_item_conflict(CrossTable* symbol_table, HashItem* item);
void semantic_error(ErrorType error_type, int line, char* msg);
bool is_structure(HashItem* item);

CrossTable* init_cross_table();
Type* new_type(Kind kind, ...);
void free_type(Type* specifier_type);
Type* copy_type(Type* src_type);
HashItem* new_item(int scope_layer, FieldList* pfield);
void free_item(HashItem* item);
void add_item_to_table(HashItem* item, CrossTable* symbol_table);
FieldList* new_fieldlist(char* name, Type* type);
void set_fieldlistname(FieldList* p, char* name);
FieldList* copy_fieldlist(FieldList* src_fieldlist);
void free_fieldlist(FieldList* fieldlist);
HashTable* new_hash_table();
HashItem* get_hash_head(HashTable* hash_table, int index);
void add_item_to_hash(HashItem* item, HashTable* hash, int hash_bucket);
Stack* new_stack();
HashItem* get_stack_cur_head(Stack* stack);
void add_item_to_stack(HashItem* item, Stack* stack);

/*
Ext: 外部
Def: 定义，例如：int、struct type、struct hash{}
Tag: 结构体标签，即定义的结构体名称，可以用于声明其它结构体（注意是不是结构体*变量*名称，而是*结构体*名称）
Dec: 声明
Var: 变量（变量名），例如：a、a[5]
Fun: 函数
*/
void TraverseTree(Node* node);
void ExtDef(Node* node);
Type* Specifier(Node* node);
Type* StructSpecifier(Node* node);
void ExtDecList(Node* node, Type* specifier_type);
void FunDec(Node* node, Type* specifier_type);
void CompSt(Node* node, Type* specifier_type);
void DefList(Node* node, HashItem* struct_item);