git pull
make clean all
for d in */ ; do
    cd $d
    mkdir -p bin/
    mkdir -p out/
    make all
    cd ..
done
mkdir -p $HOME/dram_codebase/timing_histogram_mem/out_fix/
