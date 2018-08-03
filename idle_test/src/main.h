#include "power_test.h"
#include "dimm.h"
#include "node_state.h"
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <ctime>

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

enum test_name : uint{
    // Functional
    TEST        =  0,
    STOP_LOOP   =  1,
    FORCED_PRECHARGE = 2,

    // Datasheet Defined Tests
    IDD0        =  3,
    IDD1        =  4,
    IDD4R       =  5,
    IDD4W       =  6,

    // Custom Tests
    A2RP        =  7,
    A3RP        =  8,
    A4RP        =  9,
    A2WP        = 10,
    A3WP        = 11,
    A4WP        = 12,
    ANRP        = 13,
    ANWP        = 14,
    ANRPFT      = 15,
    AARP        = 16,
    AAWP        = 17,
    BUSDIRTEST  = 18,

    IDLE_RD     = 19,
    IDLE_WR_0   = 20,
    IDLE_WR_1   = 21,

    NUM_TESTS   = 22
};

void print_help(int argc);
test_name arg_test_name_match(std::string);
