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
            if(temp_item->scope_layer == symbol_table->stack->stack_layer) return true; // 错误类型3（部分）（并完成要求2.2：检查之前定义的变量作用域和现在的是否相同）、错误类型4：变量-变量、函数名-函数名 冲突
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
        va_start(vaList, kind);
        ptype->u.function.argc = va_arg(vaList, int);
        ptype->u.function.argv = va_arg(vaList, FieldList*);
        ptype->u.function.return_type = va_arg(vaList, Type*);
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
        specifier_type->u.array.elem=NULL;
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
        free_type(specifier_type->u.function.return_type);
        specifier_type->u.function.return_type = NULL;
        temp_field = specifier_type->u.function.argv;
        while (temp_field)
        {
            delete_field = temp_field;
            temp_field = temp_field->tail;
            free_fieldlist(delete_field);
        }
        specifier_type->u.function.argv = NULL;
        break;
    default:
        // TODO: find error
        break;
    }
    free(specifier_type);
}

Type* copy_type(Type* src_type)
{
    if(src_type==NULL) return NULL;
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
        copy->u.function.argc = src_type->u.function.argc;
        copy->u.function.argv = copy_fieldlist(src_type->u.function.argv);
        copy->u.function.return_type = copy_type(src_type->u.function.return_type);
        break;
    default:
        break;
    }
    return copy;
}

bool check_type(Type* type1, Type* type2)   // 检查两个类型是否相等
{
    if(type1==NULL || type2==NULL) return true; // 出现错误的时候判为相等
    if(type1->kind != type2->kind) return false;
    else 
    {
        switch (type1->kind) 
        {
            case BASIC:
                return type1->u.basic == type2->u.basic;
            case ARRAY:
                return check_type(type1->u.array.elem, type2->u.array.elem);
            case STRUCTURE:
                return !strcmp(type1->u.structure.name, type2->u.structure.name);
        }
    }
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

void delete_item_from_table(HashItem* item, CrossTable* table)
{
    unsigned int hash_bucket = hash_pjw(item->field->name);
    if(item == get_hash_head(table->hash_table, hash_bucket)) table->hash_table->hash_array[hash_bucket] = item->next_hash_item;
    else
    {
        HashItem* now = get_hash_head(table->hash_table, hash_bucket);
        HashItem* last = now;
        while (now != item)
        {
            last = now;
            now = now->next_hash_item;
        }
        last->next_hash_item = now->next_hash_item;
    }
    free_item(item);
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

void add_stack_layer(Stack* stack){stack->stack_layer++;}

void sub_stack_layer(Stack* stack){stack->stack_layer--;}

void clear_now_layer(CrossTable* table)
{
    Stack* stack = table->stack;
    HashItem* item = get_stack_cur_head(stack);
    while (item)
    {
        HashItem* delete_item = item;
        item = item->next_layer_item;
        delete_item_from_table(delete_item, symbol_table);
    }
    stack->stack_array[stack->stack_layer] = NULL;
    sub_stack_layer(stack);
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

/*
FunDec
    ID LP VarList RP
    ID LP RP
*/
void FunDec(Node* node, Type* specifier_type)
{
    HashItem* item = new_item(symbol_table->stack->stack_layer, new_fieldlist(node->children[0]->value.str_value, new_type(FUNCTION, 0, NULL, copy_type(specifier_type))));

    // FunDec->ID LP VarList RP
    if(!strcmp(node->children[2]->name, "VarList")) VarList(node->children[2], item);

    if(check_item_conflict(symbol_table, item)) // 错误类型4
    {
        char msg[100] = {0};
        sprintf(msg, "Redefined function \"%s\".", item->field->name);
        semantic_error(REDEF_FUNC, node->lineno, msg);
        free_item(item);
    }
    else add_item_to_table(item, symbol_table);
}

/*
CompSt 函数中括号内部
    LC DefList StmList RC
Def
    Specifier DecList SEMI
StmList
    Stmt StmList
    none
*/
void CompSt(Node* node, Type* specifier_type)
{
    add_stack_layer(symbol_table->stack);
    Node* deflist=node->children[1];    // DefList
    Node* stmlist=node->children[2];    // StmList
    if(deflist->type == SYN_NO_NULL)
    {
        DefList(deflist, NULL);
    }
    if (stmlist->type == SYN_NO_NULL)
    {
        StmList(stmlist, specifier_type); // 注意比较return的值
    }
    clear_now_layer(symbol_table);
}

/*
DefList
    Def DefList
    none            （第一次不会生成空，进入此函数时已经判断了）
*/
void DefList(Node* node, HashItem* item)    
{
    while(node->type == SYN_NO_NULL)
    {
        Def(node->children[0], item);
        node = node->children[1];
    }  
}

/*
Def
    Specifier DecList SEMI      变量声明
*/
void Def(Node* node, HashItem* item)
{
    Type* specifier_type = Specifier(node->children[0]);
    DecList(node->children[1], specifier_type, item);
    if(specifier_type) free_type(specifier_type);
}

/*
DecList
    Dec
    Dec COMMA DecList
*/
void DecList(Node* node, Type* specifier_type, HashItem* item)
{
    Node* temp = node;
    while(temp)
    {
        Dec(temp->children[0], specifier_type, item);
        if(temp->child_num>=3) temp = temp->children[2];
        else break;
    }
}

/*
Dec
    VarDec              不赋值
    VarDec ASSIGNP Exp  赋值
*/
void Dec(Node* node, Type* specifier_type, HashItem* item)
{
    // Dec->VarDec
    if(node->child_num<=1)
    {
        if(item != NULL)    // 只有定义结构体(函数内的结构体，非全局)时这里不为空
        {
            HashItem* vardec = VarDec(node->children[0], specifier_type);
            FieldList* vardec_field = vardec->field;
            FieldList* struct_field = item->field->type->u.structure.field;
            FieldList* last = NULL;
            while(struct_field != NULL) // 错误类型15，判断结构体内域名是否重复定义，不重复则退出循环插入该域名
            {
                if(!strcmp(vardec_field->name, struct_field->name))
                {
                    char msg[100]={0};
                    sprintf(msg, "Redefined field \"%s\".", vardec_field->name);
                    semantic_error(REDEF_FEILD, node->lineno, msg);
                    free_item(vardec);
                    return;
                }
                else
                {
                    last = struct_field;
                    struct_field = struct_field->tail;
                }
            }
            if(last == NULL) item->field->type->u.structure.field = copy_fieldlist(vardec_field);    // 首次直接插入
            else last->tail = copy_fieldlist(vardec_field);
            free_item(vardec);
        }
        else    // 非结构体的情况
        {
            HashItem* vardec = VarDec(node->children[0], specifier_type);
            if(check_item_conflict(symbol_table, vardec))   // 错误类型3
            {
                char msg[100]={0};
                sprintf(msg, "Redefined variable \"%s\".", vardec->field->name);
                semantic_error(REDEF_VAR, node->lineno, msg);
                free_item(vardec);
            }
            else
            {
                add_item_to_table(vardec, symbol_table);
            }
        }
    }

    // Dec->VarDec ASSIGNP Exp
    else
    {
        if(item != NULL) semantic_error(REDEF_FEILD, node->lineno, "Illegal initialize variable in struct.");   // 错误类型15：结构体定义中赋值
        else
        {
            // 判断赋值是否正确，如果正确则添加至符号表
            HashItem* vardec = VarDec(node->children[0], specifier_type);
            Type* exp_type = Exp(node->children[2]);
            if(check_item_conflict(symbol_table, vardec))   // 错误类型3：重名
            {
                char msg[100] = {0};
                sprintf(msg, "Redefined variable \"%s\".", vardec->field->name);
                semantic_error(REDEF_VAR, node->lineno, msg);
                free_item(vardec);
            }
            else if(!check_type(vardec->field->type, exp_type))  // 错误类型5：赋值类型不符
            {
                semantic_error(TYPE_MISMATCH_ASSIGN, node->lineno, "Type mismatchedfor assignment.");
                free_item(vardec);
            }
            else if(vardec->field->type && vardec->field->type->kind == ARRAY)   // 错误类型5：直接对数组赋值。TODO：确认这部分的作用
            {
                semantic_error(TYPE_MISMATCH_ASSIGN, node->lineno, "Illegal initialize variable");
                free_item(vardec);
            }
            else add_item_to_table(vardec, symbol_table);

            if(exp_type) free_type(exp_type);
        }
    }
}

/*
Exp
    Exp ASSIGNOP Exp
    Exp AND Exp
    Exp OR Exp
    Exp RELOP Exp
    Exp PLUS Exp
    Exp MINUS Exp
    Exp STAR Exp
    Exp DIV Exp
-   Exp LB Exp RB
    Exp DOT ID
    
    MINUS Exp
    NOT Exp

    LP Exp RP

    ID LP Args RP
    ID LP RP

    ID
    INT
    FLOAT
    
*/
Type* Exp(Node* node)
{
    Node* child1 = node->children[0];
    if(!strcmp(child1->name, "Exp"))
    {
        Node* child2 = node->children[1];
        Node* child3 = node->children[2];
        // 上半部分(基本数学及逻辑运算)
        if(strcmp(child2->name, "LB") && strcmp(child2->name, "DOT"))
        {
            Type* p1 = Exp(child1);
            Type* p2 = Exp(child3);
            Type* return_type = NULL;

            // Exp -> Exp ASSIGNOP Exp
            if(!strcmp(child2->name, "ASSIGNOP"))
            {   
                // 左值检查
                if(child1->children[0]->type == LEX_FLOAT || child1->children[0]->type == LEX_INT)
                {
                    semantic_error(LEFT_VAR_ASSIGN, child1->lineno, "The left-hand side of an assignment must be avaliable.");  // 错误类型6
                }
                else if(!strcmp(child1->children[0]->name, "ID") || !strcmp(child1->children[1]->name, "LB") || !strcmp(child1->children[1]->name, "DOT"))  // 普通变量、数组、结构体
                {
                    if(!check_type(p1, p2)) semantic_error(TYPE_MISMATCH_ASSIGN, child1->lineno, "Type mismatched for assignment.");    // 错误类型5
                    else return_type = copy_type(p1);
                }
                else semantic_error(LEFT_VAR_ASSIGN, child1->lineno, "The left-hand side of an assignment must be avaliable."); // 错误类型6
            }
            // Exp -> Exp AND Exp
            //        Exp OR Exp
            //        Exp RELOP Exp
            //        Exp PLUS Exp
            //        Exp MINUS Exp
            //        Exp STAR Exp
            //        Exp DIV Exp
            else
            {
                if(p1 && p2 && (p1->kind == ARRAY || p2->kind == ARRAY)) semantic_error(TYPE_MISMATCH_OP, child1->lineno, "Type mismatched for operands."); // 错误类型7
                else if(!check_type(p1, p2)) semantic_error(TYPE_MISMATCH_OP, child1->lineno, "Type mismatched for operands."); // 错误类型7
                else{if(p1 && p2) return_type = copy_type(p1);}
            }
            
            if(p1) free_type(p1);
            if(p2) free_type(p2);
            return return_type;
        }
        // 下半部分(数组和结构体)
        else
        {
            // Exp -> Exp LB Exp RB 数组
            if(!strcmp(child2->name, "LB"))
            {
                Type* p1 = Exp(child1); // Exp1
                Type* p2 = Exp(child3); // Exp2
                Type* return_type = NULL;
                if(!p1){}   // 第一个为空直接错误
                else if(p1 && p1->kind != ARRAY)    //  非数组使用了[]
                {
                    char msg[100] = {0};
                    sprintf(msg, "\"%s\" is not an array.", child1->children[0]->value.str_value);  // 错误类型10
                    semantic_error(NOT_A_ARRAY, child1->lineno, msg);
                }
                else if(!p2 || p2->kind != BASIC || p2->u.basic != INT_TYPE)    // 没用使用整数索引
                {
                    char msg[100] = {0};
                    sprintf(msg, "\"%s\" is not an integer.", child3->children[0]->value.str_value);    // 错误类型12
                    semantic_error(NOT_A_INT, child3->lineno, msg);
                }
                else return_type = copy_type(p1->u.array.elem);

                if(p1) free_type(p1);
                if(p2) free_type(p2);
                return return_type;
            }
            // Exp -> Exp DOT ID    结构体
            else
            {
                Type* p1 = Exp(child1);
                Type* return_type = NULL;
                if(!p1 || p1->kind != STRUCTURE || !p1->u.structure.name)   // 非结构体使用.进行索引
                {
                    semantic_error(ILLEGAL_USE_DOT, child1->lineno, "Illegal use of \".\".");    // 错误类型13
                    if(p1) free_type(p1);
                }
                else    // 处理域名，是否与定义的不同
                {
                    Node* struct_id = child3;
                    FieldList* struct_field = p1->u.structure.field;
                    while(struct_field != NULL) // 遍历判断是否存在该域
                    {
                        if(!strcmp(struct_field->name, struct_id->value.str_value)) break;
                        struct_field = struct_field->tail;
                    }
                    if(struct_field == NULL) printf("Error type %d at Line %d: %s.\n", 14, child3->lineno, "NONEXISTFIELD");    // 错误类型14
                    else return_type = copy_type(struct_field->type);
                }
                if(p1) free_type(p1);
                return return_type;
            }
        }
    }
    else if(!strcmp(child1->name, "MINUS") || !strcmp(child1->name, "NOT"))
    {
        Type* temp = Exp(node->children[1]);    // 错误类型7
        Type* return_type = NULL;
        if(!temp || temp->kind != BASIC) printf("Error type %d at Line %d: %s.\n", 7, child1->lineno, "TYPE_MISMATCH_OP");
        else return_type = copy_type(temp);
        if(temp) free_type(temp);
        return return_type;
    }
    // LP Exp RP
    else if(!strcmp(child1->name, "LP")) return Exp(node->children[1]);
    else if(!strcmp(child1->name, "ID") && node->child_num>=2)
    {
        HashItem* item = search_item(symbol_table, child1->value.str_value);

        // error
        if(item == NULL)
        {
            char msg[100] = {0};
            sprintf(msg, "Undefined function \"%s\".", child1->value.str_value);   // 错误类型2
            semantic_error(UNDEF_FUNC, node->lineno, msg);
            return NULL;
        }
        else if(item->field->type->kind != FUNCTION)
        {
            char msg[100] = {0};
            sprintf(msg, "\"%s\" is not a function.", child1->name);
            semantic_error(NOT_A_FUNC, node->lineno, msg);  // 错误类型 11
            return NULL;
        }

        // Exp->ID LP Args RP
        else if(!strcmp(node->children[2]->name, "Args"))
        {
            Args(node->children[2], item);
            return copy_type(item->field->type->u.function.return_type);
        }

        // Exp->ID LP RP
        else
        {
            if(item->field->type->u.function.argc !=0)
            {
                char msg[100] = {0};
                sprintf(msg, "too few arguments to function \"%s\", except %d args.", item->field->name, item->field->type->u.function.argc);
                semantic_error(FUNC_AGRC_MISMATCH, node->lineno, msg);  // 错误类型9
            }
            return copy_type(item->field->type->u.function.return_type);
        }
    }
    else if(!strcmp(child1->name, "ID"))
    {
        HashItem* item = search_item(symbol_table, child1->value.str_value);
        if(item == NULL || is_structure(item))
        {
            char msg[100] = {0};
            sprintf(msg, "Undefined variable \"%s\".", child1->value.str_value);    // 错误类型1
            semantic_error(UNDEF_VAR, child1->lineno, msg);
            return NULL;
        }
        else return copy_type(item->field->type);
    }
    else
    {
        // Exp->FLOAT
        if(!strcmp(child1->name, "FLOAT")) return new_type(BASIC, FLOAT_TYPE);
        // Exp->INT
        else return new_type(BASIC, INT_TYPE);
    }
}

/*
Args
    Exp COMMA Args
    Exp
*/
void Args(Node* node, HashItem* item)
{
    Node* temp = node;
    FieldList* argv = item->field->type->u.function.argv;   // 函数定义的参数
    while(temp) // 将argv与函数定义的参数一一匹配
    {
        if(argv == NULL)    // 实际参数过多
        {
            char msg[100] = {0};
            sprintf(msg, "too many arguments to function \"%s\", except %d args.", item->field->name, item->field->type->u.function.argc);  // 错误类型9
            semantic_error(FUNC_AGRC_MISMATCH, node->lineno, msg);
            break;
        }
        Type* real_type = Exp(temp->children[0]);   // 实际传入的参数
        if(!check_type(real_type, argv->type))
        {
            char msg[100] = {0};
            sprintf(msg, "Function \"%s\" is not applicable for arguments.", item->field->name);    // 错误类型9
            semantic_error(FUNC_AGRC_MISMATCH, node->lineno, msg);
            if(real_type) free_type(real_type);
            return;
        }
        if(real_type) free_type(real_type);
        
        argv = argv->tail;
        if(temp->child_num>=3) temp = temp->children[2];
        else break;
    }
    if(argv != NULL)    // 实际参数过少
    {
        char msg[100] = {0};
        sprintf(msg, "too few argumetns to function \"%s\", except %d args.", item->field->name, item->field->type->u.function.argc);
        semantic_error(FUNC_AGRC_MISMATCH, node->lineno, msg);  // 错误类型9
    }
}

/*
StmList
    Stmt StmtList
    none
*/
void StmList(Node* node, Type* return_type)
{
    while(node->type == SYN_NO_NULL)
    {
        Stmt(node->children[0], return_type);
        node = node->children[1];
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
void Stmt(Node* node, Type* return_type)
{
    // Stmt->Exp SEMI
    Type* exp_type = NULL;
    if(!strcmp(node->children[0]->name, "Exp")) exp_type=Exp(node->children[0]);

    // Stmt->CompSt
    else if(!strcmp(node->children[0]->name, "CompSt")) CompSt(node->children[0], return_type);
    
    // Stmt->RETURN Exp SEMI
    else if(!strcmp(node->children[0]->name, "RETURN")) // 比较返回值与定义的类型是否相同
    {
        exp_type = Exp(node->children[1]);
        if(!check_type(return_type, exp_type)) semantic_error(TYPE_MISMATCH_RETURN, node->lineno, "Type mismatched for return.");   // 错误类型8
    }

    // Stmt->IF LP Exp RP Stmt
    // Stmt->IF LP Exp RP Stmt ELSE Stmt
    else if(!strcmp(node->children[0]->name, "IF"))
    {
        Node* stmt = node->children[4];
        exp_type = Exp(node->children[2]);
        Stmt(stmt, return_type);
        if(node->child_num>=7) Stmt(node->children[6], return_type);
    }

    // Stmt->WHILE LP Exp RP Stmt
    else if(!strcmp(node->children[0]->name, "WHILE"))
    {
        exp_type = Exp(node->children[2]);
        Stmt(node->children[4], return_type);
    }

    if(exp_type) free_type(exp_type);
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

/*
VarList
    ParamDec COMMA VarList
    ParamDec
ParamDec
    Specifier VarDec
*/
void VarList(Node* node, HashItem* item)
{
    add_stack_layer(symbol_table->stack);
    int argc = 0;
    Node* temp = node->children[0]; // ParamDec
    FieldList* now = NULL;

    // VarList->ParamDec(一个参数)
    FieldList* paramdec = ParamDec(temp);
    item->field->type->u.function.argv = copy_fieldlist(paramdec);
    now = item->field->type->u.function.argv;
    argc = 1;

    // VarList->ParamDec COMMA VarList
    while(node->child_num>=3)
    {
        node = node->children[2];   // VarList
        temp = node->children[0];   // next ParamDec
        paramdec = ParamDec(temp);
        if(paramdec)
        {
            now->tail = copy_fieldlist(paramdec);
            now = now->tail;
            argc++;
        }
    }
    item->field->type->u.function.argc = argc;
    sub_stack_layer(symbol_table->stack);
}

/*
ParamDec
    Specifier VarDec
*/
FieldList* ParamDec(Node* node)
{
    Type* specifier_type = Specifier(node->children[0]);
    HashItem* item = VarDec(node->children[1], specifier_type);
    if(specifier_type) free_type(specifier_type);
    if(check_item_conflict(symbol_table, item)) // 错误类型3
    {
        char msg[100] = {0};
        sprintf(msg, "Redefined variable \"%s\".", item->field->name);
        semantic_error(REDEF_VAR, node->lineno, msg);
        free_item(item);
        return NULL;
    }
    else
    {
        add_item_to_table(item, symbol_table);
        return item->field;
    }
}