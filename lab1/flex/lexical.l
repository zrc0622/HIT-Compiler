%option yylineno

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
LINE [\n]


%%

{TYPE} {printf("TYPE\n");}
{STRUCT} {printf("STRUCT\n");}
{RETURN} {printf("RETURN\n");}
{IF} {printf("IF\n");}
{ELSE} {printf("ELSE\n");}
{WHILE} {printf("WHILE\n");}
{INT} {printf("INT\n");}
{FLOAT} {printf("FLOAT\n");}
{ID} {printf("ID\n");}
{SEMI} {printf("SEMI\n");}
{COMMA} {printf("COMMA\n");}
{ASSIGNOP} {printf("ASSIGNOP\n");}
{RELOP} {printf("RELOP\n");}
{PLUS} {printf("PLUS\n");}
{MINUS} {printf("MINUS\n");}
{STAR} {printf("STAR\n");}
{DIV} {printf("DIV\n");}
{AND} {printf("AND\n");}
{OR} {printf("OR\n");}
{DOT} {printf("DOT\n");}
{NOT} {printf("NOT\n");}
{LP} {printf("LP\n");}
{RP} {printf("RP\n");}
{LB} {printf("LB\n");}
{RB} {printf("RB\n");}
{LC} {printf("LC\n");}
{RC} {printf("RC\n");}
{SPACE} {}
{LINE} {printf("%d\n", yylineno);}
. {printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);}

%%
/* 
编译命令：
flex lexicall
gcc main.c lex.yy.c -lfl -o scanner
*/