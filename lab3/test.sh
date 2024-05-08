cd code
flex lexical.l
bison -d syntax.y
gcc main.c syntax.tab.c tree.c semantic.c -lfl -ly -o ../bin/parser
cd ../test
../bin/parser test1.cmm