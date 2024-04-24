#include "semantic.h"

CrossTable* symbol_table;   // 全局符号表，在main中初始化

/*功能函数***************************************************************************/
char* new_string(const char* src)   // 创建一个字符串副本
{ 
    if (src == NULL) return NULL;
    size_t length = strlen(src) + 1;
    char* memory = (char*)malloc(length * sizeof(char));
    if (memory == NULL) return NULL;
    strncpy(memory, src, length);
    return memory;
}

unsigned int hash_pjw(char* name)
{
    unsigned int val = 0, i;
    for(; *name; ++name)
    {
        val = (val << 2) + *name;
        if(i=val & ~SYMBOL_TABLE_SIZE) val = (val ^ (i>>12)) & SYMBOL_TABLE_SIZE;
    }
    return val;
}
/*错误处理***************************************************************************/
HashItem* search_item(CrossTable* symbol_table, char* name) // 搜索符号表中名为name的符号
{
    unsigned int hash_bucket = hash_pjw(name);
    HashItem* temp_item = get_hash_head(symbol_table->hash_table, hash_bucket);
    while (temp_item)
    {
        if(!strcmp(temp_item->field->name, name)) return temp_item;
        temp_item = temp_item->next_hash_item;
    }
    return NULL;
}

bool check_item_conflict(CrossTable* symbol_table, HashItem* item)
{
    HashItem* temp_item = search_item(symbol_table, item->field->name);
    if(temp_item==NULL) return false;
    while(temp_item)    // 遍历是否有重名的项，需要注意的时search_item返回之后的项可能还有重名的，所以需要循环递归后续的项
    {
        if(!strcmp(temp_item->field->name, item->field->name))
        {
            if(temp_item->field->type->kind == STRUCTURE || item->field->type->kind == STRUCTURE) return true;  // 错误类型3（部分）、错误类型16：变量-结构体、结构体-变量、结构体-结构体 冲突
            if(temp_item->scope_layer == symbol_table->stack->stack_layer) return true; // 错误类型3（部分）（并完成要求2.2：检查之前定义的变量作用域和现在的是否相同）：变量-变量 冲突
        }
        temp_item=temp_item->next_hash_item; // 遍历下一项
    }
    return false; // 无冲突
}

void semantic_error(ErrorType error_type, int line, char* msg)  // 错误信息输出
{
    printf("Error type %d at line %d: %s\n", error_type, line, msg);
}

bool is_structure(HashItem* item)   // 判断是否是结构体的定义
{
    if(item->field->type->kind != STRUCTURE) return false;  // 类型不是STRUCTURE
    if(item->field->type->u.structure.name) return false;   // 如果类型有名字，说明这是一个结构体的变量，而不是定义
    return true;
}
/*节点创建***************************************************************************/
CrossTable* init_cross_table()
{
    CrossTable* ptable = (CrossTable*)malloc(sizeof(CrossTable));
    ptable->hash_table = new_hash_table();
    ptable->stack = new_stack();
    ptable->unnamed_struct = 0;
    symbol_table=ptable;
    return ptable;
}

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
    FieldList* temp_field = NULL;
    FieldList* delete_field = NULL;
    switch (specifier_type->kind)
    {
    case BASIC:
        break;
    case ARRAY:
        free_type(specifier_type->u.array.elem);
        break;
    case STRUCTURE:
        if(specifier_type->u.structure.name) free(specifier_type->u.structure.name);
        specifier_type->u.structure.name = NULL;
        temp_field = specifier_type->u.structure.field;
        while (temp_field)
        {
            delete_field = temp_field;
            temp_field = temp_field->tail;
            free_fieldlist(delete_field);
        }
        specifier_type->u.structure.field = NULL;
        break;
    case FUNCTION:
        // TODO
    default:
        break;
    }
    free(specifier_type);
}

Type* copy_type(Type* src_type)
{
    Type* copy = (Type*)malloc(sizeof(Type));
    copy->kind=src_type->kind;
    switch (copy->kind)
    {
    case BASIC:
        copy->u.basic = src_type->u.basic;
        break;
    case ARRAY:
        copy->u.array.elem = copy_type(src_type->u.array.elem);
        copy->u.array.size = src_type->u.array.size;
        break;
    case STRUCTURE:
        copy->u.structure.name = new_string(src_type->u.structure.name);
        copy->u.structure.field = copy_fieldlist(src_type->u.structure.field);
        break;
    case FUNCTION:
        // TODO
    default:
        break;
    }
    return copy;
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

void free_item(HashItem* item)
{
    if(item->field != NULL) free_fieldlist(item->field);
    free(item);
}

void add_item_to_table(HashItem* item, CrossTable* symbol_table) // 将item插入符号表
{
    unsigned int hash_bucket = hash_pjw(item->field->name);
    add_item_to_stack(item, symbol_table->stack);
    add_item_to_hash(item, symbol_table->hash_table, hash_bucket);
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

FieldList* copy_fieldlist(FieldList* src_fieldlist)
{
    FieldList* head = NULL, * now = NULL, * temp = src_fieldlist;
    while (temp)
    {
        if(!head)
        {
            head = new_fieldlist(temp->name, copy_type(temp->type));
            now = head;
            temp = temp->tail;
        }
        else
        {
            now->tail = new_fieldlist(temp->name, copy_type(temp->type));
            now = now->tail;
            temp = temp->tail;
        }
    }
    return head;
}

void free_fieldlist(FieldList* fieldlist)
{
    if(fieldlist->name) free(fieldlist->name);
    fieldlist->name=NULL;
    if(fieldlist->type) free_type(fieldlist->type);
    fieldlist->type=NULL;
    free(fieldlist);
}

HashTable* new_hash_table()
{
    HashTable* ptable = (HashTable*)malloc(sizeof(HashTable));
    ptable->hash_array = (HashItem**)malloc(sizeof(HashItem*)*SYMBOL_TABLE_SIZE);
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++) ptable->hash_array[i] = NULL;
    return ptable;
}

HashItem* get_hash_head(HashTable* hash_table, int index)   // 返回哈希头
{
    return hash_table->hash_array[index];
}

void add_item_to_hash(HashItem* item, HashTable* hash, int hash_bucket)
{
    item->next_hash_item = get_hash_head(hash, hash_bucket);
    hash->hash_array[hash_bucket] = item;
}

Stack* new_stack()
{
    Stack* p = (Stack*)malloc(sizeof(Stack));
    p->stack_array = (HashItem**)malloc(sizeof(HashItem*)*SYMBOL_TABLE_SIZE);
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++)  p->stack_array[i] = NULL;
    p->stack_layer=0;
    return p;
}

HashItem* get_stack_cur_head(Stack* stack)
{
    return stack->stack_array[stack->stack_layer];
}

void add_item_to_stack(HashItem* item, Stack* stack)    // 将item插入到stack中
{
    item->next_layer_item = get_stack_cur_head(stack);
    stack->stack_array[stack->stack_layer] = item;
}

/*语义操作***************************************************************************/

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
    if(child_node->type == LEX_TYPE){// TYPE
        if(!strcmp(child_node->value.str_value, "int")) return new_type(BASIC, INT_TYPE);
        else return new_type(BASIC, FLOAT_TYPE);
    }
    else return StructSpecifier(child_node);
}

/*
StructSpecifier
    STRUCT OptTag LC DefList RC 使用新定义结构体
    STRUCT Tag  使用已定义结构体
OptTag
    ID
    none
Tag
    ID
*/
Type* StructSpecifier(Node* node)
{
    Node* child1 = node->children[1];   // OptTag 或 Tag
    Type* inherit_type = NULL;
    // 新定义: STRUCT OptTag LC DefList RC
    if(strcmp(child1->name, "Tag"))
    {
        Node* child3 = node->children[3];   // DefList
        HashItem* struct_item = new_item(symbol_table->stack->stack_layer, new_fieldlist("", new_type(STRUCTURE, NULL, NULL)));
        // 命名结构体类型
        if(child1->type==SYN_NO_NULL)   // OptTag产生ID：具名结构体
        {
            set_fieldlistname(struct_item->field, child1->children[0]->value.str_value);    // ID
        }
        else    // OptTag产生空：匿名结构体
        {
            symbol_table->unnamed_struct++;
            char new_name[20] = {0};
            sprintf(new_name, "%d", symbol_table->unnamed_struct);
            set_fieldlistname(struct_item->field, new_name);    // 使用数字进行命名，确保不会产生冲突
        }

        // 处理结构体内部参数
        DefList(child3, struct_item);

        // 判断是否重名
        if(check_item_conflict(symbol_table, struct_item))  // true为重名
        {
            char msg[100] = {0};
            sprintf(msg, "Duplicated name \"%s\".", struct_item->field->name);
            semantic_error(DUPLICATED_NAME, node->lineno, msg);
            free_item(struct_item); // 重名释放
        }
        else
        {
            inherit_type = new_type(STRUCTURE, new_string(struct_item->field->name), copy_fieldlist(struct_item->field->type->u.structure.field));
            if(child1->type==SYN_NO_NULL) add_item_to_table(struct_item, symbol_table);
            else free_item(struct_item);    // 匿名结构体可以不用加入符号表，只返回继承给变量的类型即可
        }
    }
    // 已定义: STRUCT Tag
    else
    {
        HashItem* struct_item = search_item(symbol_table, child1->children[0]->value.str_value);    // ID
        if(struct_item == NULL || !is_structure(struct_item)) // 错误类型17
        {
            char msg[100] = {0};
            sprintf(msg, "Undefined structure \"%s\".", child1->children[0]->value.str_value);
            semantic_error(UNDEF_STRUCT, node->lineno, msg);
        }
        else
        {
            inherit_type = new_type(STRUCTURE, new_string(struct_item->field->name), copy_fieldlist(struct_item->field->type->u.structure.field));
        }
    }
    return inherit_type;
}

/*
ExtDecList
    VarDec
    VarDec COMMA ExtDecList
*/
void ExtDecList(Node* node, Type* specifier_type)   // 使用将声明类型继承给变量名
{
    Node* temp = node;
    while (temp)
    {
        HashItem* item = VarDec(temp->children[0], specifier_type);
        if(check_item_conflict(symbol_table, item))
        {
            char msg[100] = {0};
            sprintf(msg, "Redefined variable \"%s\".", item->field->name);  // 错误类型3
            semantic_error(REDEF_VAR, temp->lineno, msg);
            free_item(item);
        }
        else add_item_to_table(item, symbol_table);

        if(temp->children[1]) temp = temp->children[2]; // 跳过逗号
        else break;
    }
    
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

/*
VarDec
    ID
    VarDec LB INT RB
*/
HashItem* VarDec(Node* node, Type* specifier_type)
{
    Node* id = node;
    while(id->child_num) id=id->children[0];    // 递归取出变量名
    HashItem* item = new_item(symbol_table->stack->stack_layer, new_fieldlist(id->value.str_value, NULL));  // 为变量名创建空item

    if(!strcmp(node->children[0]->name, "ID")) item->field->type = copy_type(specifier_type);   // VarDec->ID
    else    // VarDec->VarDec LB INT RB
    {
        Node* vardec = node->children[0];
        Node* array_int = node->children[2];
        Type* temp_type = specifier_type;
        while(true)
        {
            item->field->type = new_type(ARRAY, copy_type(temp_type), array_int->value.int_value);
            if(vardec->child_num<=1) break; // 到ID了
            array_int = vardec->children[2];
            vardec = vardec->children[0];
            temp_type = item->field->type;
        }
    }
    return item;
}