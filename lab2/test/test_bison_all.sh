cd test
for ((i=1; i<=17; i++))
do
    echo "-------------------------------------------------"
    echo "test$i:"
    echo
    ../bison/parser test$i.cmm
    echo "-------------------------------------------------"
    echo
done