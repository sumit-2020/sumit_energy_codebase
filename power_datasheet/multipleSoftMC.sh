declare -a arr1=( "FCFS" "FRFCFS" )
declare -a arr2=( "sumitFRFCFSPRIORHIT" "sumitAHB" "sumitFCFS" "sumitFRFCFS" )

for c in 0 1;do
	cp "../tempfiles/${arr1[c]}_power_test.cpp" "../src/power_test.cpp"
	make clean
	make -j
	python ./run.py samsungo18 15
done
