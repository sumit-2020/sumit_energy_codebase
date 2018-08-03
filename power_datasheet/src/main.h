#include "power_test.h"
#include "dimm.h"
#include "node_state.h"
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <ctime>

enum test_name : uint{
    // Functional
    STOP  =  0,

    // Datasheet Defined Tests
    IDD0  = 1,
    IDD1  = 2,
    IDD2N = 3,
    IDD3N = 4,
    IDD4R = 5,
    IDD4W = 6,

    TEST  = 7,

    // Power Down, Self Refresh etc.
    IDD2P  = 8,
    IDD2Q  = 9,
    IDD3P  = 10,
    IDD5B  = 11,
    IDD6   = 12,
    IDD6ET = 13,
    IDD7   = 14,
    IDD8   = 15,

    // Sumit's custom benchmarks
    SUMITFCFS           = 17,
    SUMITFRFCFS         = 18,
    SUMITAHB            = 19,
    SUMITFRFCFSPRIORHIT = 20,
    
    NUM_TESTS = 21
};

void print_help(int argc);
test_name arg_test_name_match(std::string);
