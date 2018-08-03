#include "power_test.h"
#include "dimm.h"
#include "node_state.h"
#include "softmc_api.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <ctime>
#include <vector>
#include <utility>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "softmc_api.h"

#define SANDBOX 1
#define ROW 17868

typedef std::pair< int, int > addr_t;
typedef std::vector<addr_t> addr_arr; 

enum test_t {
    ZERO, STOP, BICI,
    ACTRDPREWAIT, ACTRDWAITPRE, 
    ACTWRPREWAIT, ACTWRWAITPRE, 
    ACTPREWAIT, ACTWAITPRE,
    NUM_TESTS
};

test_t which_test(std::string);
addr_arr parse_addresses(std::string);


void print_help();
