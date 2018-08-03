#!/bin/bash

# Args check
if [ -z "$1" ]; then
    echo "end tRCD"
    exit
fi
if [ -z "$2" ]; then
    echo "end tRP"
    exit
fi
if [ -z "$3" ]; then
    echo "Iteration"
    exit
fi
if [ -z "$4" ]; then
    echo "Temperature. 20, 30, or 70"
    exit
fi
if [ -z "$5" ]; then
    echo "Voltage"
    exit
fi
if [ -z "$6" ]; then
    echo "DIMM name"
    exit
fi


its=$3
temp=$4
volt=$5

for pattern in "0xaa" "0xff" "0xcc"; do
    for trcd in `seq 5 -1 $1`; do
        for trp in `seq 5 -1 $2`; do
            ./bin/safari_mc_test $6 $pattern $trcd $trp $its $temp $volt
            ./../drain_buffers/bin/safari_mc_test
        done
    done
done
