cd code
flex lexical.l
bison -d syntax.y
gcc main.c syntax.tab.c tree.c semantic.c -lfl -ly -o parser
cd ../test
../code/parser test.cmm