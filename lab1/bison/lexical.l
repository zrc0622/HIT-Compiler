%option yylineno

%{
    #include "tree.h"
    #define YYSTYPE Node* // 定义yylval即终结符和非终结符的属性为Node*

    #include "syntax.tab.h" // 增加对语法中token定义的引用
    
    /*用于维护语法单元的位置信息*/
    int yycolumn = 1; 
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;

    int LexError = 0; // 词法错误数

    void PrintError(int type, int lineno, char* error); // 输出错误: 错误的int和float
%}

DIGIT [0-9]
LETTER [a-zA-Z]
DECIMAL 0|[1-9]{DIGIT}*
OCTAL 0[0-7]+
HEXADECIMAL (0x|0X)[0-9a-fA-F]+

ERROR_OCTAL 0{DIGIT}*[89]{DIGIT}*
ERROR_HEXADECIMAL (0x|0X)[0-9a-zA-Z]*[g-zG-Z][0-9a-zA-Z]*
ERROR_INT {ERROR_OCTAL}|{ERROR_HEXADECIMAL}
ERROR_FLOAT (\.{DIGIT}+|{DIGIT}+\.{DIGIT}+|{DIGIT}+\.)[eE]

INT {DECIMAL}|{OCTAL}|{HEXADECIMAL}
FLOAT ({DIGIT}+\.{DIGIT}+)|((\.{DIGIT}+|{DIGIT}+\.{DIGIT}+|{DIGIT}+\.)[eE]([+-]?{DIGIT}+))
ID ({LETTER}|_)({DIGIT}|{LETTER}|_)*
SEMI  ";" 
COMMA  "," 
ASSIGNOP  "=" 
RELOP  ">"|"<"|">="|"<="|"=="|"!="
PLUS  "+" 
MINUS  "-" 
STAR  "*" 
DIV  "/" 
AND  "&&" 
OR  "||" 
DOT  "." 
NOT  "!" 
TYPE  "int"|"float"
LP  "(" 
RP  ")" 
LB  "[" 
RB  "]" 
LC  "{" 
RC  "}" 
STRUCT  "struct" 
RETURN  "return" 
IF  "if" 
ELSE  "else" 
WHILE  "while"

SPACE [ \t]+
LINE \n

%%
{TYPE} {yylval=MakeLeafNode("TYPE", LEX_TYPE, yytext, yylineno); return TYPE;}
{STRUCT} {yylval=MakeLeafNode("STRUCT", LEX_OTHERS, yytext, yylineno); return STRUCT;}
{RETURN} {yylval=MakeLeafNode("RETURN", LEX_OTHERS, yytext, yylineno); return RETURN;}
{IF} {yylval=MakeLeafNode("IF", LEX_OTHERS, yytext, yylineno); return IF;}
{ELSE} {yylval=MakeLeafNode("ELSE", LEX_OTHERS, yytext, yylineno); return ELSE;}
{WHILE} {yylval=MakeLeafNode("WHILE", LEX_OTHERS, yytext, yylineno); return WHILE;}
{INT} {yylval=MakeLeafNode("INT", LEX_INT, yytext, yylineno); return INT;}
{FLOAT} {yylval=MakeLeafNode("FLOAT", LEX_FLOAT, yytext, yylineno); return FLOAT;}
{ID} {yylval=MakeLeafNode("ID", LEX_ID, yytext, yylineno); return ID;}
{SEMI} {yylval=MakeLeafNode("SEMI", LEX_OTHERS, yytext, yylineno); return SEMI;}
{COMMA} {yylval=MakeLeafNode("COMMA", LEX_OTHERS, yytext, yylineno); return COMMA;}
{ASSIGNOP} {yylval=MakeLeafNode("ASSIGNOP", LEX_OTHERS, yytext, yylineno); return ASSIGNOP;}
{RELOP} {yylval=MakeLeafNode("RELOP", LEX_OTHERS, yytext, yylineno); return RELOP;}
{PLUS} {yylval=MakeLeafNode("PLUS", LEX_OTHERS, yytext, yylineno); return PLUS;}
{MINUS} {yylval=MakeLeafNode("MINUS", LEX_OTHERS, yytext, yylineno); return MINUS;}
{STAR} {yylval=MakeLeafNode("STAR", LEX_OTHERS, yytext, yylineno); return STAR;}
{DIV} {yylval=MakeLeafNode("DIV", LEX_OTHERS, yytext, yylineno); return DIV;}
{AND} {yylval=MakeLeafNode("AND", LEX_OTHERS, yytext, yylineno); return AND;}
{OR} {yylval=MakeLeafNode("OR", LEX_OTHERS, yytext, yylineno); return OR;}
{DOT} {yylval=MakeLeafNode("DOT", LEX_OTHERS, yytext, yylineno); return DOT;}
{NOT} {yylval=MakeLeafNode("NOT", LEX_OTHERS, yytext, yylineno); return NOT;}
{LP} {yylval=MakeLeafNode("LP", LEX_OTHERS, yytext, yylineno); return LP;}
{RP} {yylval=MakeLeafNode("RP", LEX_OTHERS, yytext, yylineno); return RP;}
{LB} {yylval=MakeLeafNode("LB", LEX_OTHERS, yytext, yylineno); return LB;}
{RB} {yylval=MakeLeafNode("RB", LEX_OTHERS, yytext, yylineno); return RB;}
{LC} {yylval=MakeLeafNode("LC", LEX_OTHERS, yytext, yylineno); return LC;}
{RC} {yylval=MakeLeafNode("RC", LEX_OTHERS, yytext, yylineno); return RC;}

{ERROR_OCTAL} {yylval=MakeLeafNode("ERROR_INT", LEX_OTHERS, NULL, yylineno); PrintError(1, yylineno, yytext); LexError++; return INT;}
{ERROR_HEXADECIMAL} {yylval=MakeLeafNode("ERROR_INT", LEX_OTHERS, NULL, yylineno); PrintError(2, yylineno, yytext); LexError++; return INT;}
{ERROR_FLOAT} {yylval=MakeLeafNode("ERROR_FLOAT", LEX_OTHERS, NULL, yylineno); PrintError(3, yylineno, yytext); LexError++; return FLOAT;}

"//" {for(char a=input(); a != '\n';) a=input();}
"/*" {for(char a=input(), b=input(); a != '*' || b != '/';){a=b; b=input();}}
{SPACE} {}
{LINE} {yycolumn = 1;}
. {printf("Error type A at Line %d: Mysterious characters \"%s\".\n", yylineno, yytext); LexError++;}

%%
void PrintError(int type, int lineno, char* error)
{
    switch(type)
    {
        case 1: printf("Error type A at Line %d: Illegal octal number \'%s\'.\n", lineno, error); break;
        case 2: printf("Error type A at Line %d: Illegal hexadecimal number \'%s\'.\n", lineno, error); break;
        case 3: printf("Error type A at Line %d: Illegal floating point number \"%s\".\n", lineno, error); break;
    }
}
