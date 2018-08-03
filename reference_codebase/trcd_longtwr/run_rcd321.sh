vendor="crucial"
dimm="b6"
dir=$vendor"_results/"$dimm"_rcd321"
for i in 1; do
    mkdir -p $dir
    echo $dir
    echo $vendor$dimm
    ./safari_mc_test_longtwr $vendor$dimm 0 $dir
done
