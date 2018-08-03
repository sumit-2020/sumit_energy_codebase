#!/bin/bash

TESTS=(power_test/out power_datasheet/out_fix idle_test/out col_tests/out toggle_new/out )
UPLOADBUFFER="dram@dramg2.andrew.cmu.edu:/home/dram/power_tests/writeup_dram_energy_modeling"
SUBDIRS=("upload_buffer" "power_datasheet/src" "idle_analysis/original_results" "structural_variation/coltest/original_results" "upload_buffer" )
let "END = ${#TESTS[@]} - 1"

for i in $(seq 0 $END); do
    SRC_DIR="${TESTS[$i]}"
    DEST_DIR="$UPLOADBUFFER/${SUBDIRS[$i]}/"
    echo -e "$i: $SRC_DIR > $DEST_DIR" # rsync -rlptDvz
    if [ -d "$SRC_DIR" ]
        then
        cnt=`ls -l ${SRC_DIR}/*.csv | wc -l`
        if [ $cnt != 0 ]
            then
            rsync -rlptDvz ${SRC_DIR}/*.csv ${DEST_DIR}
            let "i = $i + 1"
        else
            echo "Directory is there, but no csv files are found!"
        fi
    else
        echo "Source directory does not exist!"
    fi
    echo ""
done

