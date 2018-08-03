#include "power_test.h"
#include "dimm.h"
#include "node_state.h"
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <ctime>

#define SANDBOX 1
#define ROW 17868

enum test_t {
    ZERO, STOP, BICI, SBSC,
    SBMC, MBMC, SBDC, DBDC,
    NUM_TESTS
};

enum operation_t {
    READ, WRITE,
    NUM_OPS
};

enum addr_t {
  R01, R02, R03, R04, R05, R06, R07, R08, R09, R10,
  R11, R12, R13, R14, R15, R16, R17, R18, R19, R20,
  D01, D02, D03, D04, D05, D06, D07, D08, D09, D10,
  D11, D12, D13, D14, D15, D16, D17, D18, D19, D20,
  D00, B51, B73, B34,
  NUM_ADDR_SETS
};


addr_t which_addr(std::string);
test_t which_test(std::string);
operation_t which_operation(std::string);

void print_help();
