#include "power_test.h"
#include "dimm.h"
#include "node_state.h"
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <ctime>

enum test_name : uint{
    // Functional
    TEST        =  0,
    STOP_LOOP   =  1,
    // Datasheet Defined Tests
    IDD0        =  2,
    IDD1        =  3,
    IDD4R       =  4,
    IDD4W       =  5,
    // Custom Tests
    A2RP        =  6,
    A3RP        =  7,
    A4RP        =  8,
    A2WP        =  9,
    A3WP        = 10,
    A4WP        = 11,
    ANRP        = 12,
    ANWP        = 13,
    ANRPFT      = 14,
    ANWPFT      = 15,
    AARP        = 16,
    AAWP        = 17,
    BUSDIRTEST  = 18,

    NUM_TESTS   = 19
};

void print_help(int argc);
test_name arg_test_name_match(std::string);
