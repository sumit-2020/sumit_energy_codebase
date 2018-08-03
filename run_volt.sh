#!/bin/bash

# Args check
if [ -z "$1" ]; then
    echo "No DIMM info (argument) supplied! ./run.sh crucialb41 450 3 20 1"
    exit
fi
if [ -z "$2" ]; then
    echo "No starting volt supplied! ./run.sh crucialb41 450 3"
    exit
fi
if [ -z "$3" ]; then
    echo "No iteration count supplied! ./run.sh crucialb41 450 3"
    exit
fi
if [ -z "$4" ]; then
    echo "Temperature. 20, 30, or 70"
    exit
fi
if [ -z "$5" ]; then
    echo "Test type. 1:rcd+rp 2:rcd 3:rp $ ./run.sh crucialb41 3 1 70"
    exit
fi

DIMM=$1
VOLTAGE=$2 #millivolts
N_IT=$3
TEMPERATURE=$4"c"

TEST=(trcd trp)
date_time=$(date +"%Y/%m/%d %T")

# Different types of test
if [ "$5" -eq 2 ]; then
    TEST=(trcd)
elif [ "$5" -eq 3 ]; then
    TEST=(trp)
fi

END_DELAY=2
START_DELAY=5
if [ "$6" ]; then
    START_DELAY=$6
    END_DELAY=3
fi

#DEFAULT nrow, ncol = 1GB
#64 ms wait for tras test. others are 0ms
while [ $VOLTAGE -ge 100 ]; do
	echo "set vdd to 1.${VOLTAGE}"
	read
    for CURR_TEST in ${TEST[*]} ; do
        for i in `seq 1 $N_IT`; do #repeat N times
            echo "Test starts @ $date_time"
            echo "<<<DIMM: $DIMM | Temp: $TEMPERATURE | Test: $CURR_TEST | Iter: $i>>>"
            if [ $CURR_TEST == "tras" ]; then
                bin/safari_mc_test \
                    -t	$CURR_TEST \
                    -w  64 \
                    -m	$TEMPERATURE \
                    -v	1.$VOLTAGE \
                    -d	$DIMM \
                    -c
            else
                bin/safari_mc_test \
                    -t	$CURR_TEST \
                    -m	$TEMPERATURE \
                    -v	1.$VOLTAGE \
                    -d	$DIMM \
                    -e  $END_DELAY \
                    -s  $START_DELAY \
                    -r  32736 \
                    -b

                    # -r  32736 -> From 32768 - Assume some unused spare rows...
                    # -b bypass verify read
                    #-c  \
            fi
        done
    done
	let VOLTAGE=$VOLTAGE-50
done
