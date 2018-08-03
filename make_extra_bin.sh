# same pattern
make clean
make INV="-DINVERT_PATT=0 -DINVERT_HALF=0"
mv bin/safari_mc_test bin/safari_mc_test_samepatt

# Invert half
make clean
make INV="-DINVERT_PATT=0 -DINVERT_HALF=1"
mv bin/safari_mc_test bin/safari_mc_test_halfpatt

# Invert pattern
make clean all
