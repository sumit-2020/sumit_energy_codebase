#!/bin/bash

if [ -z "$1" ]; then
    echo "No DIMM info (argument) supplied! ./run.sh crucialb41 3 1.5 70c"
    exit
fi
if [ -z "$2" ]; then
    echo "No iteration count supplied! ./run.sh crucialb41 3 1.5 70c"
    exit
fi
if [ -z "$3" ]; then
    echo "No voltage supplied! ./run.sh crucialb41 3 1.5 70c"
    exit
fi
if [ -z "$4" ]; then
    echo "No temperature supplied! ./run.sh crucialb41 3 1.5 70c"
    exit
fi



DIMM=$1
N_IT=$2
TEMPERATURE=$4
TEST=(trrd)
# Different types of test
VOLTAGE=$3

echo "<<<DIMM: $DIMM | Temp: $TEMPERATURE | Test: ${TEST[*]}>>>"

#DEFAULT nrow, ncol = 1GB
#64 ms wait for tras test. others are 0ms
for i in `seq 1 $N_IT`; do
    for CURR_TEST in ${TEST[*]} ; do
        export CURR_TEST
        export TEMPERATURE
        export DIMM
        export VOLTAGE
        gdb bin/safari_mc_test < gdb_rrd_cmds
        sleep 5

        gdb bin/safari_mc_test_samepatt < gdb_rrd_cmds
        sleep 5

        mkdir -p results/trrd_same_patt
        rsync -avz --remove-source-files same_patt/trrd/* results/trrd_same_patt
    done
done
