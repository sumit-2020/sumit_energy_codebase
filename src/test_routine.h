#ifndef SAFARIMC_TEST_ROUTINE_H_
#define SAFARIMC_TEST_ROUTINE_H_

#include <stdio.h>
#include <cassert>
#include <type_traits>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <bitset>

#include "timer.h"
#include "utils.h"
#include "minicsv.h"
#include "softmc_api.h"

using namespace std;

extern int toggle01;
extern int toggle10;

//#define SANDBOX //if defined, fpga is by-passed completely.

// CSV related
struct RowErrInfo
{
    // Ctors
    RowErrInfo() : iteration(0), row_num(0), bank_num(0),
          total_err_bit(0), cacheline_err_loc(""),
          beat0_err(0), beat1_err(0), beat2_err(0),
          beat3_err(0), beat4_err(0), beat5_err(0),
          beat6_err(0), beat7_err(0) {}

    RowErrInfo(int iteration_, int row_num_, int bank_num_) : iteration(iteration_),
        row_num(row_num_), bank_num(bank_num_) {}

    RowErrInfo(int iteration_, int row_num_, int bank_num_, int total_err_bit_,
    string cacheline_err_loc_, int beat0_err_, int beat1_err_, int beat2_err_,
    int beat3_err_, int beat4_err_, int beat5_err_, int beat6_err_, int beat7_err_)
        : iteration(iteration_), row_num(row_num_), bank_num(bank_num_),
          total_err_bit(total_err_bit_), cacheline_err_loc(cacheline_err_loc_),
          beat0_err(beat0_err_), beat1_err(beat1_err_), beat2_err(beat2_err_),
          beat3_err(beat3_err_), beat4_err(beat4_err_), beat5_err(beat5_err_),
          beat6_err(beat6_err_), beat7_err(beat7_err_) {}

    int iteration;
    int row_num;
    int bank_num;
    int total_err_bit;
    string cacheline_err_loc;
    int beat0_err, beat1_err, beat2_err, beat3_err, beat4_err, beat5_err,
        beat6_err, beat7_err;
    bitset<128> err_cl_vec;
};

// Prototypes
const std::string fmtTime();
int recv_compare(fpga_t* fpga, int ch, uint8_t pattern, unsigned burst_errors[]);
void write_count_file(uint row, uint col, uint bank, int err_cnt,
        ofstream& count_file, bool count_enable, unsigned burst_errors[]);
void write_histo_file(ofstream& histo_file, uint bank,
        map<int,int>* err_map_bank, const int cline_size_bits);
void openAndCloseRow(fpga_t* fpga, const uint row, const uint bank,
                const uint delay_tras, const uint delay_trp, CmdQueue*& cq);
void writeCol(fpga_t* fpga, const uint col, const uint row, const uint bank,
                const uint8_t pattern, CmdQueue*& cq);
int readAndCompareColCount(fpga_t* fpga, const uint col, const uint row,
        const uint bank, const uint8_t pattern, const uint trcd, CmdQueue*& cq);
void readAndCompareCol(fpga_t* fpga, const uint col, const uint row,
        const uint bank, const uint8_t pattern, const uint trcd, CmdQueue*& cq);
void precharge(fpga_t *fpga, CmdQueue*& cq, uint bank);

// Test on various timing parameters
void testRetention(fpga_t* fpga, const int retention);
void test_tras(fpga_t* fpga, const uint retention, const uint tras,
                const uint wait_to_read);
void batch_test_trcd_col_order(fpga_t* fpga, const uint retention,
                const uint trcd);
void trcd_transient_test(fpga_t* fpga, const uint retention, const uint trcd);
int test_trcd_col_order(fpga_t* fpga, const uint retention, const uint trcd,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern,
        bool count_enable, uint nrow, uint ncol, bool bypass_verif);
int test_twr_col_order(fpga_t* fpga, const uint retention, const uint twr,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern,
        bool count_enable, uint nrow, uint ncol) ;
int test_tras_row_order(fpga_t* fpga, const uint retention, const uint tras,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern,
        bool count_enable, uint nrow, uint ncol) ;
int test_trp_row_order(fpga_t* fpga, const uint retention, const uint trp,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern,
        bool count_enable, uint nrow, uint ncol, bool bypass_verif) ;

// Row level
void writeRow(fpga_t* fpga, uint row, uint bank, uint8_t pattern,
                CmdQueue*& cq, uint ncol);
void writeRowFlexRCD(fpga_t* fpga, uint row, uint bank, uint8_t pattern,
        CmdQueue*& cq, uint ncol, uint trcd, uint start_col,
        bool alternate_col_patt);
int readAndCompareRow(fpga_t* fpga, const uint row, const uint bank,
        const uint8_t pattern, CmdQueue*& cq, const uint ncol, uint start_col,
        bool alternate_col_patt);
int readAndCompareRow(fpga_t* fpga, const uint row, const uint bank,
        const uint8_t pattern, CmdQueue*& cq, const uint ncol, uint start_col);
int readAndCompareRow(fpga_t* fpga, const uint row, const uint bank,
                const uint8_t pattern, CmdQueue*& cq, const uint ncol);
int readAndCompareRowOpen(fpga_t* fpga, const uint row, const uint bank,
                const uint8_t pattern, CmdQueue*& cq, const uint ncol);

// Power Test Related Functions
void openRowRead(fpga_t* fpga,  const uint row, const uint bank,
                const uint8_t pattern, const uint trcd, CmdQueue*& cq);
void openRowWrite(fpga_t* fpga, uint row, uint bank, uint8_t pattern,
                uint trcd, CmdQueue*& cq);
void writeColBuffered(fpga_t* fpga, const uint col, const uint row,
                      const uint bank, const uint8_t pattern, CmdQueue*& cq);
void readColBuffered(fpga_t* fpga, const uint col, const uint row, const uint bank,
                     const uint8_t pattern, const uint trcd, CmdQueue*& cq);

// Voltage scaling
void test_trrd4ck_col_order(fpga_t* fpga, const uint retention,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern,
        bool count_enable, uint nrow, uint ncol);

// Multiple timing parameter test
void writeHeaderCSV(csv::ofstream& of_csv);
int writeAndReadRow(fpga_t* fpga, int test_it, uint row, uint row_next,
        uint bank, uint8_t pattern, CmdQueue* cq,
        const uint trcd, const uint trp,
        const uint ms_wait_time, csv::ofstream& of_csv);

#endif // SAFARIMC_TEST_ROUTINE_H_