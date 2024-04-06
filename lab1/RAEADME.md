# 文件结构
|- flex: 使用flex进行词法分析。包含flex代码`lexical.l`(使用`flex lexical.l`编译)，主程序`main.c`(使用`gcc main.c lex.yy.c -lfl -o scanner`编译)。代码通过标准输出打印识别到的token类型，无法识别会报错
|- bison: 使用bison进行语法分析。包含bison代码`syntax.y`(使用`bison -d syntax.y`编译)，主程序`main.c`(使用`gcc main.c syntax.tab.c -lfl -ly -o parser`编译)。也可以直接使用`./make.sh`进行编译
    |- tree.c: 语法树构建代码
|- test：测试文件