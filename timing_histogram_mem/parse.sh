cd out
for i in `ls *rcd4_rp4*`; do echo $i; python ../../parse_timing_histogram/histfile_tester.py $i; done
cd ..
