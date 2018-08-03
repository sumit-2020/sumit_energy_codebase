#ifndef TIMING_HISTOGRAM_H
#define TIMING_HISTOGRAM_H

#include "test_routine.h"
#include "combined_timing_test.h"
#include "timing_histogram_mem.h"
#include <chrono>
#include <fstream>
#include <list>
#include <sys/time.h>
#include <string.h>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>

int wr_rd_2rows( fpga_t*, uint , uint , uint8_t , CmdQueue * , const uint , const uint , const uint, string);

int update_row_in_log_file(string file_name, uint bank_num, uint row_num, bitset<TEST_NUM_CACHELINES> err_vec);
uint find_row_in_log(uint bank_num, uint row_num);
int cp(const char *to, const char *from);

// To measure the time cost of the process
timespec start_timer();
double stop_timer(timespec, bool);

#endif