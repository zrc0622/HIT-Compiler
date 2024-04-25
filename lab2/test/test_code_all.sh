cd test
for ((i=1; i<=17; i++))
do
    echo "-------------------------------------------------"
    echo "test$i:"
    ../bin/parser test$i.cmm
    echo "-------------------------------------------------"
    echo
done