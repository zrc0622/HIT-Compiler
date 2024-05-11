cd code
flex lexical.l
bison -d syntax.y
gcc main.c syntax.tab.c tree.c semantic.c inter.c -lfl -ly -o ../bin/parser