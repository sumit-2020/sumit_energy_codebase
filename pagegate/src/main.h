#include "power_test.h"
#include "dimm.h"
#include "node_state.h"
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <ctime>

enum test_t {
    ZERO, STOP,
    ACT, PRE,
    NUM_TESTS
};

test_t which_test(std::string test);