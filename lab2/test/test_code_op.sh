cd test
for ((i=1; i<=6; i++))
do
    echo "------------------------------------------------------------"
    echo "test_o$i:"
    ../bin/parser test_o$i.cmm
    echo "------------------------------------------------------------"
    echo
done