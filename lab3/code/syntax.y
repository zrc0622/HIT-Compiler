%locations

%{
    #include <stdarg.h>
    #include "tree.h"
    #include "lex.yy.c"
    #include <stdbool.h>
    #include <stdio.h>
    Node* root = NULL; // 使用全局变量保存根节点
    Node** pack(int child_num, Node* child1, ...); // 打包子树
    void yyerror(const char* msg); // 需要先定义yyerror
    int SynError = 0;
    int error_type = 0;
%}

%token INT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE
%type Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def Dec DecList Exp Args

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
Program : ExtDefList {$$ = MakeNode("Program", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1)); root=$$;}
    ;
ExtDefList : ExtDef ExtDefList {$$ = MakeNode("ExtDefList", SYN_NO_NULL, NULL, @$.first_line, 2, pack(2, $1, $2));}
    | /* empty */ {$$ = MakeNode("ExtDefList", SYN_NULL, NULL, @$.first_line, 0, NULL);}
    ;
ExtDef : Specifier ExtDecList SEMI {$$ = MakeNode("ExtDef", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | Specifier SEMI {$$ = MakeNode("ExtDef", SYN_NO_NULL, NULL, @$.first_line, 2, pack(2, $1, $2));}
    | Specifier FunDec CompSt {$$ = MakeNode("ExtDef", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | Specifier ExtDecList error {$$ = MakeNode("Error", SYN_NULL, NULL, @$.first_line, 0, NULL); error_type=1; yyerror(NULL);}
    | Specifier error {$$ = MakeNode("Error", SYN_NULL, NULL, @$.first_line, 0, NULL); error_type=1; yyerror(NULL);}
    ;
ExtDecList : VarDec {$$ = MakeNode("ExtDecList", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    | VarDec COMMA ExtDecList {$$ = MakeNode("ExtDecList", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    ;

Specifier : TYPE {$$ = MakeNode("Specifier", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    | StructSpecifier {$$ = MakeNode("Specifier", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {$$ = MakeNode("StructSpecifier", SYN_NO_NULL, NULL, @$.first_line, 5, pack(5, $1, $2, $3, $4, $5));}
    | STRUCT Tag {$$ = MakeNode("StructSpecifier", SYN_NO_NULL, NULL, @$.first_line, 2, pack(2, $1, $2));}
    ;
OptTag : ID {$$ = MakeNode("OptTag", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    | /* empty */ {$$ = MakeNode("OptTag", SYN_NULL, NULL, @$.first_line, 0, NULL);}
    ;
Tag : ID {$$ = MakeNode("Tag", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    ;

VarDec : ID {$$ = MakeNode("VarDec", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    | VarDec LB INT RB {$$ = MakeNode("VarDec", SYN_NO_NULL, NULL, @$.first_line, 4, pack(4, $1, $2, $3, $4));}
    | VarDec LB error RB {$$ = MakeNode("Error", SYN_NULL, NULL, @$.first_line, 0, NULL); error_type=2; yyerror(NULL);}
    ;
FunDec : ID LP VarList RP {$$ = MakeNode("FunDec", SYN_NO_NULL, NULL, @$.first_line, 4, pack(4, $1, $2, $3, $4));}
    | ID LP RP {$$ = MakeNode("FunDec", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    ;
VarList: ParamDec COMMA VarList {$$ = MakeNode("VarList", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | ParamDec {$$ = MakeNode("VarList", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    ;
ParamDec : Specifier VarDec {$$ = MakeNode("ParamDec", SYN_NO_NULL, NULL, @$.first_line, 2, pack(2, $1, $2));}
    ;

CompSt : LC DefList StmtList RC {$$ = MakeNode("CompSt", SYN_NO_NULL, NULL, @$.first_line, 4, pack(4, $1, $2, $3, $4));}
    ;
StmtList : Stmt StmtList {$$ = MakeNode("StmtList", SYN_NO_NULL, NULL, @$.first_line, 2, pack(2, $1, $2));}
    | /* empty */ {$$ = MakeNode("StmtList", SYN_NULL, NULL, @$.first_line, 0, NULL);}
    ;
Stmt : Exp SEMI {$$ = MakeNode("Stmt", SYN_NO_NULL, NULL, @$.first_line, 2, pack(2, $1, $2));}
    | CompSt {$$ = MakeNode("Stmt", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    | RETURN Exp SEMI {$$ = MakeNode("Stmt", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = MakeNode("Stmt", SYN_NO_NULL, NULL, @$.first_line, 5, pack(5, $1, $2, $3, $4, $5));}
    | IF LP Exp RP Stmt ELSE Stmt {$$ = MakeNode("Stmt", SYN_NO_NULL, NULL, @$.first_line, 7, pack(7, $1, $2, $3, $4, $5, $6, $7));}
    | WHILE LP Exp RP Stmt {$$ = MakeNode("Stmt", SYN_NO_NULL, NULL, @$.first_line, 5, pack(5, $1, $2, $3, $4, $5));}
    | Exp error {$$ = MakeNode("Error", SYN_NULL, NULL, @$.first_line, 0, NULL); error_type=1; yyerror(NULL);}
    | RETURN Exp error {$$ = MakeNode("Error", SYN_NULL, NULL, @$.first_line, 0, NULL); error_type=1; yyerror(NULL);}
    ;

DefList : Def DefList {$$ = MakeNode("DefList", SYN_NO_NULL, NULL, @$.first_line, 2, pack(2, $1, $2));}
    | /* empty */ {$$ = MakeNode("DefList", SYN_NULL, NULL, @$.first_line, 0, NULL);}
    ;
Def : Specifier DecList SEMI {$$ = MakeNode("Def", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | Specifier DecList error {$$ = MakeNode("Error", SYN_NULL, NULL, @$.first_line, 0, NULL); error_type=1; yyerror(NULL);}
    ;
DecList : Dec {$$ = MakeNode("DecList", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    | Dec COMMA DecList  {$$ = MakeNode("DecList", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    ;
Dec : VarDec {$$ = MakeNode("Dec", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    | VarDec ASSIGNOP Exp {$$ = MakeNode("Dec", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    ;

Exp : Exp ASSIGNOP Exp {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | Exp AND Exp {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | Exp OR Exp {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | Exp RELOP Exp {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | Exp PLUS Exp {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));} 
    | Exp MINUS Exp {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));} 
    | Exp STAR Exp {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}  
    | Exp DIV Exp {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | LP Exp RP {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | MINUS Exp {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 2, pack(2, $1, $2));} 
    | NOT Exp {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 2, pack(2, $1, $2));}  
    | ID LP Args RP {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 4, pack(4, $1, $2, $3, $4));} 
    | ID LP RP {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));} 
    | Exp LB Exp RB {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 4, pack(4, $1, $2, $3, $4));} 
    | Exp LB error RB{$$ = MakeNode("Error", SYN_NULL, NULL, @$.first_line, 0, NULL); error_type=2; yyerror(NULL);} 
    | Exp DOT ID {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));} 
    | ID {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));} 
    | INT {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));} 
    | FLOAT {$$ = MakeNode("Exp", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));} 
    ;
Args : Exp COMMA Args {$$ = MakeNode("Args", SYN_NO_NULL, NULL, @$.first_line, 3, pack(3, $1, $2, $3));}
    | Exp {$$ = MakeNode("Args", SYN_NO_NULL, NULL, @$.first_line, 1, pack(1, $1));}
    ;

%%
Node** pack(int child_num, Node* child1, ...)
{
    va_list child_list;
    va_start(child_list, child1);
    Node** result = (Node**)malloc(sizeof(Node*) * child_num);
    result[0] = child1;
    for (int i = 1; i < child_num; i++)
    {
        result[i] = va_arg(child_list, Node*);
    }
    return result;
}


void yyerror(const char* msg) 
{
    SynError++;
    if(!msg)
    {
        switch(error_type)
        {
            case 0: printf("Error type B at Line %d: syntax error.", yylineno); break;
            case 1: printf("Error type B at Line %d: Missing \";\".\n", yylineno); break;
            case 2: printf("Error type B at Line %d: Missing \"]\".\n", yylineno); break;
        }
    }
    error_type = 0;
}
