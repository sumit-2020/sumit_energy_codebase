#!/bin/bash

if [ -z "$1" ]; then
    echo "No DIMM info (argument) supplied! ./run.sh crucialb41 3 1 70"
    exit
fi
if [ -z "$2" ]; then
    echo "No iteration count supplied! ./run.sh crucialb41 3 1 70"
    exit
fi
if [ -z "$3" ]; then
    echo "Test type. 1:rcd+rp 2:rcd 3:rp $ ./run.sh crucialb41 3 1 70"
    exit
fi
if [ -z "$4" ]; then
    echo "Temperature. 20, 30, or 70"
    exit
fi


DIMM=$1
N_IT=$2
#TEMPERATURE="20c"
TEMPERATURE=$4"c"
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
        if [ $CURR_TEST == "tras" ]; then
            bin/safari_mc_test \
                -t	$CURR_TEST \
                -w  64 \
                -m	$TEMPERATURE \
                -d	$DIMM \
                -c
        else
            bin/safari_mc_test \
                -t	$CURR_TEST \
                -m	$TEMPERATURE \
                -d	$DIMM \
                -e 2 \
                -r  32736  # From 32768 - Assume some unused spare rows...
        fi

        # Trap segfault program
        if [ $? -eq 139 ]; then
            echo "Test crashed at iteration $i with test $CURR_TEST !!"
            exit 1
        fi
    done
done
