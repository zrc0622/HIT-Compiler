#include <stdio.h>
#include "tree.h"
#include "syntax.tab.h" // 词法分析和语法分析
#include "semantic.h"   // 语义分析
extern Node* root;
extern int LexError;
extern int SynError;
extern int yyrestart(FILE* f);

int main(int argc, char** argv)
{
    /*词法分析和语法分析*/
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f){
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    // if(root != NULL && !LexError && !SynError){  // 输出语法分析树
    //     PrintTree(root, 0);
    // }

    /*语义分析*/
    if(root != NULL && !LexError && !SynError){
        TraverseTree(root);
    }

    FreeTree(root);
    return 0;
}
