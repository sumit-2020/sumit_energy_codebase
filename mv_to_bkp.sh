#!/bin/bash

#EXCLUDE="hynixp68"
EXCLUDE=$1

find results/*/* -type d -ls > folders_list
rm -f mv_list

while read line; do
    if [[ ! $line =~ $EXCLUDE ]] ; then
        echo $line | cut -d" " -f11 | cut -d/ -f 2-3 >> mv_list;
    fi;
done < folders_list

if [ -f mv_list ] ; then
    while read line; do
        mkdir -p results_bkp/$line
        mv -T results/$line results_bkp/$line
    done < mv_list
fi


