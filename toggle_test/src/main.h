#include "power_test.h"
#include "dimm.h"
#include "node_state.h"
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <ctime>

enum test_t {
    ZERO, STOP, 
    R1, R2, R3, R4, 
    R5, R6, R7, R8,
    R9, R10, R11, R12,
    R13, R14, R15, R16,
    R17, R18, R19, R20,
    B0C0, B0B1C0, B0C1, B0C2, B0CX, BXCX,
    X0X1X2, B0C0C1,
    BASE_TEST3, BASE_TEST4,
    TEST3,TEST4,
    NUM_TESTS
};

test_t which_test(std::string test);