%option yylineno

%{
    #include "syntax.tab.h" // 增加对语法中token定义的引用
    /*用于维护语法单元的位置信息*/
    int yycolumn = 1; 
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
%}

DIGIT [0-9]
LETTER [a-zA-Z]
DECIMAL 0|[1-9]{DIGIT}*
OCTAL 0[0-7]+
HEXADECIMAL (0x|0X)[0-9a-fA-F]+
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

{TYPE} {return TYPE;}
{STRUCT} {return STRUCT;}
{RETURN} {return RETURN;}
{IF} {return IF;}
{ELSE} {return ELSE;}
{WHILE} {return WHILE;}
{INT} {return INT;}
{FLOAT} {return FLOAT;}
{ID} {return ID;}
{SEMI} {return SEMI;}
{COMMA} {return COMMA;}
{ASSIGNOP} {return ASSIGNOP;}
{RELOP} {return RELOP;}
{PLUS} {return PLUS;}
{MINUS} {return MINUS;}
{STAR} {return STAR;}
{DIV} {return DIV;}
{AND} {return AND;}
{OR} {return OR;}
{DOT} {return DOT;}
{NOT} {return NOT;}
{LP} {return LP;}
{RP} {return RP;}
{LB} {return LB;}
{RB} {return RB;}
{LC} {return LC;}
{RC} {return RC;}
{SPACE} {}
{LINE} {yycolumn = 1;}
. {printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);}

%%
