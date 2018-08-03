#!/bin/bash

if [ -z "$1" ]; then
    echo "No DIMM info (argument) supplied! ./run.sh crucialb41 3 1"
    exit
fi
if [ -z "$2" ]; then
    echo "No iteration count supplied! ./run.sh crucialb41 3 1"
    exit
fi
if [ -z "$3" ]; then
    echo "Test type. 1:rcd+rp 2:rcd 3:rp $ ./run.sh crucialb41 3 1"
    exit
fi


DIMM=$1
N_IT=$2
TEMPERATURE="20c"
TEST=(trcd trp)
# Different types of test
if [ "$3" -eq 2 ]; then
    TEST=(trcd)
elif [ "$3" -eq 3 ]; then
    TEST=(trp)
fi

echo "<<<DIMM: $DIMM | Temp: $TEMPERATURE | Test: ${TEST[*]}>>>"

#DEFAULT nrow, ncol = 1GB
#64 ms wait for tras test. others are 0ms
for i in `seq 1 $N_IT`; do
    for CURR_TEST in ${TEST[*]} ; do
        export CURR_TEST
        export TEMPERATURE
        export DIMM
            gdb bin/safari_mc_test < gdb_cmds
    done
done
