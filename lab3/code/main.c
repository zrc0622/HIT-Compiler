#include <stdio.h>
#include "tree.h"
#include "syntax.tab.h" // 词法分析和语法分析
#include "semantic.h"   // 语义分析
#include "inter.h"      // 中间代码生成

extern int LexError;
extern int SynError;
extern int yyrestart(FILE* f);

int main(int argc, char** argv)
{
    if (argc <= 1) return 1;
    FILE* fr = fopen(argv[1], "r");
    if (!fr){
        perror(argv[1]);
        return 1;
    }

    FILE* fw = fopen(argv[2], "wt+");
    if (!fw) {
        perror(argv[2]);
        return 1;
    }

    /*词法分析和语法分析*/
    yyrestart(fr);
    yyparse();
    // if(root != NULL && !LexError && !SynError){  // 输出语法分析树
    //     PrintTree(root, 0);
    // }

    /*语义分析*/
    if(root != NULL && !LexError && !SynError){
        symbol_table=init_cross_table();
        traverse_tree_semantic_analyze(root);
    }

    /*中间代码生成*/
    intercode_list = new_intercode_list();
    if(root != NULL && !LexError && !SynError){
        traverse_tree_generate_intercode(root);
        if(!inter_error) print_intercode_list(NULL, intercode_list);
        if(!inter_error) print_intercode_list(fw, intercode_list);
    }

    FreeTree(root);
    return 0;
}
