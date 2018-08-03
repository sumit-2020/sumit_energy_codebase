#ifndef TIMING_HISTOGRAM_MEM_H
#define TIMING_HISTOGRAM_MEM_H

#include "test_routine.h"
#include "combined_timing_test.h"
#include <chrono>
#include <fstream>
#include <list>
#include <sys/time.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

// Dimensions of the Histogram
#define TEST_NUM_BANKS 8
#define TEST_NUM_ROWS 32768
#define TEST_NUM_CACHELINES 128

// Error related
#define RECV_TRY_MAX 5

/**
 * @brief storing the histogram data in arrays cause nondeterministic
 * failures related to the allocation policies and the recent situation
 * of the host machine.
 **/
typedef std::vector< uint8_t > row_hist;
typedef std::list<row_hist> bank_hist;
typedef std::list<bank_hist> dimm_hist;

/**
 * @brief each row is classified into low, fuzzy, and high according to the number
 * of faulty cache lines.
 *
 * @low number of faulty cache lines indicates that the errors might be clustered
 * in a particular region of the row buffer. Possibly the first accessed columns.
 * @low number of faulty cache lines may occur as a result of low tRCD.
 *
 * @high number of faulty cache lines indicates that the errors look like spread
 * across the row buffer. Failure on the bitlines is a strong possibility in this
 * scenario. @high number of faulty cache lines may occur as a result of low tRP.
 *
 * if the number of faulty cache lines is in between 33% and 66%, the success of any
 * guess will be random. Therefore, in case of @fuzzy result, both latency values
 * should be tested.
 *
 * all three counters are stored in a error counters array. 0: low , 1: fuzzy, 2: high
 **/
static uint err_cntrs [3];

std::pair<row_hist,row_hist> wr_rd_2rows( fpga_t*, std::pair<row_hist,row_hist>,
	uint , uint , uint8_t , CmdQueue * , const uint , const uint , const uint );
std::pair<row_hist,row_hist> wr_rd_2rows_fix( fpga_t*, std::pair<row_hist,row_hist>,
	uint , uint , uint8_t , CmdQueue * , const uint , const uint , const uint );
row_hist update_hists(uint bank, uint row, row_hist row_h , bitset<TEST_NUM_CACHELINES> err_vec);
dimm_hist initialize_histogram(string file_name);
void dump_to_file(dimm_hist hists, string file_name);
void dump_beat_histo(ofstream& f);
void reset_cntrs();
uint * read_cntrs();
int log_iteration_count(string log_file_name, string key, int value);

#endif
