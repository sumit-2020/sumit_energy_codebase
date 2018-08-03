#!/bin/bash
volt=$2
dimm=$1
iter=$3
for i in `seq 1 $iter`;
do
  foldername="out_"
  foldername+=$dimm
  foldername+="_"
  foldername+=$volt
  foldername+="_"
  foldername+=$(date)
  foldername=${foldername// /}
  echo $foldername
  mkdir $foldername

  #echo "[$(date)] Starting 64"
  #./bin/ret_test 64 	&> ./$foldername/64ms.log
  #echo "[$(date)] Starting 128"
  #./bin/ret_test 128 	&> ./$foldername/128ms.log
  #echo "[$(date)] Starting 256"
  #./bin/ret_test 256 	&> ./$foldername/256ms.log
  echo "[$(date)] Starting 512"
  ./bin/ret_test 512 	&> ./$foldername/512ms.log
  echo "[$(date)] Starting 1024"
  ./bin/ret_test 1024 &> ./$foldername/1024ms.log
  echo "[$(date)] Starting 1536"
  ./bin/ret_test 1536 &> ./$foldername/1536ms.log
  echo "[$(date)] Starting 2048"
  ./bin/ret_test 2048 &> ./$foldername/2048ms.log
done
