cd test
for ((i=3; i<=4; i++))
do
    echo "------------------------------------------------------------"
    echo "test$i:"
    ../bin/parser test$i.cmm ../bin/out$i,ir
    echo "------------------------------------------------------------"
    echo
done