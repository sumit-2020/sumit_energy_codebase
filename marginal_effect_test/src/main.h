#include "power_test.h"
#include "dimm.h"
#include "node_state.h"
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <ctime>

enum test_name : uint{
    STOP_LOOP   =  0,
    ACT_NRD_PRE_FT = 1,
    ACT_NWR_PRE_FT = 2,
    ACT_NBWR_PRE_FT = 3,
    NUM_TESTS = 4
};

test_name arg_test_name_match(std::string);