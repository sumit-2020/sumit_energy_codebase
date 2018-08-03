#ifndef SAFARIMC_TEST_ROUTINE_H_
#define SAFARIMC_TEST_ROUTINE_H_

#include <stdio.h>
#include <riffa.h>
#include <cassert>
#include <type_traits>
#include <iostream>
#include <fstream>
#include <map>

#include "timer.h"
#include "utils.h"

using namespace std;

// Command granularity
Cmd genRowCMD(uint row, uint bank, MC_CMD c);
Cmd genWriteCMD(uint col, uint bank, uint8_t pattern);
Cmd genReadCMD(uint col, uint bank);
Cmd genWaitCMD(uint cycles_to_wait);
Cmd genBusDirCMD(BUSDIR dir);
Cmd genStartTR();
Cmd genZQCMD();

void openAndCloseRow(fpga_t* fpga, const uint row, const uint bank, const uint delay_tras, const uint delay_trp, CmdQueue*& cq);
void writeCol(fpga_t* fpga, const uint col, const uint row, const uint bank, const uint8_t pattern, CmdQueue*& cq);
void writeRow(fpga_t* fpga, uint row, uint bank, uint8_t pattern, CmdQueue*& cq);
int readAndCompareColCount(fpga_t* fpga, const uint col, const uint row, const uint bank, const uint8_t pattern, const uint trcd, CmdQueue*& cq) ;
void readAndCompareCol(fpga_t* fpga, const uint col, const uint row, const uint bank, const uint8_t pattern, const uint trcd, CmdQueue*& cq) ;
int readAndCompareRow(fpga_t* fpga, const uint row, const uint bank, const uint8_t pattern, CmdQueue*& cq );
void calibZQ(fpga_t* fpga, CmdQueue*& cq);
void turnBus(fpga_t* fpga, BUSDIR b, CmdQueue* cq);
void testRetention(fpga_t* fpga, const int retention);
void test_tras(fpga_t* fpga, const uint retention, const uint tras, const uint wait_to_read);
void batch_test_trcd_col_order(fpga_t* fpga, const uint retention, const uint trcd);
void trcd_transient_test(fpga_t* fpga, const uint retention, const uint trcd);
void test_trcd_col_order(fpga_t* fpga, const uint retention, const uint trcd,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern);
void test_trp_row_order(fpga_t* fpga, const uint retention, const uint trp,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern0, uint8_t pattern1) ;
void test_tras_row_order(fpga_t* fpga, const uint retention, const uint tras,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern0, uint8_t pattern1) ;
void simple_tras_test(fpga_t* fpga, const uint retention, const uint tras,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern) ;
void modified_tras_test(fpga_t* fpga, const uint retention, const uint tras,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern) ;
void modified_tras_test2(fpga_t* fpga, const uint retention, const uint tras,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern) ;
void test_twr_col_order(fpga_t* fpga, const uint retention, const uint twr,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern) ;

#endif // SAFARIMC_TEST_ROUTINE_H_
