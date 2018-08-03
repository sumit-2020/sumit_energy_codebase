declare -a arr1=( "FRFCFS_PriorHit" "AHB" "FCFS" "FRFCFS" )
declare -a arr2=( "sumitFRFCFSPRIORHIT" "sumitAHB" "sumitFCFS" "sumitFRFCFS" )

for c in 2 3;do
	cp "../tempfiles/${arr1[c]}_power_test.cpp" "../src/power_test.cpp"
	make clean
	make -j
	python ./run.py samsungo18 10
done
