# 文件结构
|- bison: 使用bison进行语法分析。包含bison代码`syntax.y`(使用`bison -d syntax.y`编译)，主程序`main.c`(使用`gcc main.c syntax.tab.c -lfl -ly -o parser`编译)。也可以直接使用`./make.sh`进行编译
    |- tree.c: 语法树构建代码
|- test：测试文件

# 编译与测试
1. 编译
    ```
    ./bison/make.sh
    ```
2. 测试
    ```
    # 单次测试
    ./test/test_bison.sh

    # 测试所有
    ./test/test_bison_all.sh
    ```

# 结果
1. 