#ifndef COMB_TIMING_TEST_H 

#include "test_routine.h"
#include "softmc_api.h"
#include <iostream>
#include <iomanip>
#include <chrono>

#define OVERHEAD_MS 1


// Write a row of data using standard timings
void insertWriteRowCmds(CmdQueue* cq, uint row, uint bank, uint8_t pattern);

// Read a row of data using user-specified timings
void insertFastReadCmds(CmdQueue* cq, uint row, uint bank, uint8_t pattern,
        const uint trcd, const uint trp);

/**
 * @brief Check the correctness of a row data and record the error info
 * in that row.
 **/
int checkData(fpga_t* fpga, uint8_t pattern, RowErrInfo& row_info);
std::string to_hex(int i);
void dumpRowInfo(csv::ofstream& of_csv, RowErrInfo& row, uint8_t pattern);
void writeHeaderCSV(csv::ofstream& of_csv);
int writeAndReadRow(fpga_t* fpga, int test_it, uint row, uint row_next,
        uint bank, uint8_t pattern, CmdQueue* cq,
        const uint trcd, const uint trp,
        const uint ms_wait_time, csv::ofstream& of_csv);

#endif