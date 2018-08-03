/**
 * @brief A test routine that tunes muliple timing parameters at the same time
 *
 * @author Kevin Chang <kevincha@cmu.edu>
 **/

#include "combined_timing_test.h"

#define OVERHEAD_MS 1

using namespace std;
using namespace std::chrono;

// Write a row of data using standard timings
void insertWriteRowCmds(CmdQueue* cq, uint row, uint bank, uint8_t pattern)
{
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT)); //Activate target row
    cq->insert(genWaitCMD(DEF_TRCD)); //Wait for tRCD

    //Write to some columns in a row
    for(int i = 0; i < NUM_COLS; i+=8){
        cq->insert(genWriteCMD(i, bank, pattern));
        cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    }

    cq->insert(genWaitCMD(3)); //Wait some more in any case
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE)); //Precharge target bank
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
}

// Read a row of data using user-specified timings
void insertFastReadCmds(CmdQueue* cq, uint row, uint bank, uint8_t pattern,
        const uint trcd, const uint trp)
{
    // Activate the target row
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
    // -1 due to the elapse time of the cmd issue cycle
    if (trcd > 1)
        cq->insert(genWaitCMD(trcd - 1));

    // Read the row
    for (int i = 0; i < NUM_COLS; i+=8) {
        cq->insert(genReadCMD(i, bank));
        cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    }
    cq->insert(genWaitCMD(3)); //Wait some more in any case

    // Precharge
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE));
    if (trp > 1)
        cq->insert(genWaitCMD(trp - 1));
}

/**
 * @brief Check the correctness of a row data and record the error info
 * in that row.
 **/
int checkData(fpga_t* fpga, uint8_t pattern, RowErrInfo& row_info)
{
    assert((NUM_COLS/COLS_PER_CACHELINE) <= 128);
    bitset<128> err_cl_vec;
    int bit_err_cnt = 0;
    int err_cnt = 0;
    unsigned beat_errors[8] = {0,0,0,0,0,0,0,0};

    // Compare each cache line's result
    for (int i = 0; i < NUM_COLS; i+=COLS_PER_CACHELINE)
    {
        err_cnt = recv_compare(fpga, 0, pattern, beat_errors) ;
        debug_print("Beat 0 error: %d | col %d\n", beat_errors[0], i);
        if (err_cnt == -1) {
            detail_print("recv error: didn't receive enough data!\n");
            return -1;
        }
        else if (err_cnt > 0)
            err_cl_vec.set(i/COLS_PER_CACHELINE);
        bit_err_cnt += err_cnt;
    }

    // Fill in the row info
    row_info.err_cl_vec = err_cl_vec;
    // TODO: This line causes severe memory leak!
    /*row_info.cacheline_err_loc = err_cl_vec.to_string<char,
        std::string::traits_type,std::string::allocator_type>();*/
    row_info.total_err_bit = bit_err_cnt;
    row_info.beat0_err = beat_errors[0];
    row_info.beat1_err = beat_errors[1];
    row_info.beat2_err = beat_errors[2];
    row_info.beat3_err = beat_errors[3];
    row_info.beat4_err = beat_errors[4];
    row_info.beat5_err = beat_errors[5];
    row_info.beat6_err = beat_errors[6];
    row_info.beat7_err = beat_errors[7];
    return 0;
}

std::string to_hex(int i)
{
    std::stringstream stream;
    stream << "0x"
        << std::setfill ('0') << std::setw(2)
        << std::hex << i;
    return stream.str();
}

void dumpRowInfo(csv::ofstream& of_csv, RowErrInfo& row, uint8_t pattern)
{
    of_csv << row.iteration << row.bank_num << row.row_num << row.total_err_bit <<
    row.cacheline_err_loc << row.beat0_err << row.beat1_err << row.beat2_err <<
    row.beat3_err << row.beat4_err << row.beat5_err << row.beat6_err <<
    row.beat7_err << to_hex(pattern) << NEWLINE;
}


void writeHeaderCSV(csv::ofstream& of_csv)
{
    of_csv << "Iteration" << "Bank" << "Row" << "TotalErrorBits" <<
    "CachelineErrBitVec" << "Beat0Err" << "Beat1Err" <<
    "Beat2Err" << "Beat3Err" << "Beat4Err" << "Beat5Err" <<
    "Beat6Err" << "Beat7Err" << "Pattern" << NEWLINE;
}

int writeAndReadRow(fpga_t* fpga, int test_it, uint row, uint row_next,
        uint bank, uint8_t pattern, CmdQueue* cq,
        const uint trcd, const uint trp,
        const uint ms_wait_time, csv::ofstream& of_csv)
{
    int ch = 0; //riffa channel should always be 0
    if (cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;
    uint8_t pattern_inv = (uint8_t)~pattern;

    /////////////
    // WRITE
    /////////////
    cq->insert(genBusDirCMD(BUSDIR::WRITE));
    cq->insert(genWaitCMD(5));
    insertWriteRowCmds(cq, row, bank, pattern);
    insertWriteRowCmds(cq, row_next, bank, pattern_inv);

    // Insert now other wise the queue will overflow
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
    //high_resolution_clock::time_point t1 = high_resolution_clock::now();

    // Wait x ms -- retention time
    if (ms_wait_time > 1)
        usleep((ms_wait_time-OVERHEAD_MS)*1000);

    /////////////
    // READ
    /////////////
    cq->size = 0;
    cq->insert(genBusDirCMD(BUSDIR::READ));
    cq->insert(genWaitCMD(5));
    insertFastReadCmds(cq, row, bank, pattern, trcd, trp);
    insertFastReadCmds(cq, row_next, bank, pattern_inv, trcd, trp);

    // START Transaction
    assert(cq->size < 1024);
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);

    //////////////////////
    // Check data content
    //////////////////////
    RowErrInfo row_info(test_it, row, bank);
    if (checkData(fpga, pattern, row_info) == -1)
        return -1;

    // for inv pattern
    RowErrInfo row_next_info(test_it, row_next, bank);
    if (checkData(fpga, pattern_inv, row_next_info) == -1)
        return -1;

    // Write to csv file
    dumpRowInfo(of_csv, row_info, pattern);
    dumpRowInfo(of_csv, row_next_info, pattern_inv);
    return 0;
}

