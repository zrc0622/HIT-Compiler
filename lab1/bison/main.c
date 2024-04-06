#include <stdio.h>
#include "syntax.tab.h" //yyparse()
extern int yyrestart(FILE* f); //yyrestart()

int main(int argc, char** argv)
{
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f)
    {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f); // 词法分析器的输入重定向到新的文件流
    yyparse(); // 使用语法分析器解析由词法分析器yyrestart(f)指定的文件内容
    return 0;
}
