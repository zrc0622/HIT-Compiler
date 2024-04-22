cd bison
flex lexical.l
bison -d syntax.y
gcc main.c syntax.tab.c tree.c -lfl -ly -o parser