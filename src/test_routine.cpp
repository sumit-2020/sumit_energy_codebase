/**
 * @brief Simple test routines on characterizing various timing parameters
 *
 * @author Kevin Chang <kevincha@cmu.edu>
 * @author Abhijith Kashyap <akashyap@andrew.cmu.edu>
 */

#include "test_routine.h"

// Global Variables
// # of bits flipping from 0->1 and 1->0
int toggle01 = 0;
int toggle10 = 0;

// Get the current time
const std::string fmtTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%m%d%Y_%H%M", &tstruct);
    return buf;
}

/**
 * @brief Dumps out the number of bit flips for each cache line. In addition,
 * break down the errors for each 8-bit of data in a data burst.
 **/
void write_count_file(uint row, uint col, uint bank, int err_cnt,
        ofstream& count_file, bool count_enable, unsigned burst_errors[])
{
    if (!count_enable)
        return ;
    count_file << bank << " " << row << " " << col <<" " << err_cnt <<
        " b "   << burst_errors[0] <<
        ":"     << burst_errors[1] <<
        ":"     << burst_errors[2] <<
        ":"     << burst_errors[3] <<
        ":"     << burst_errors[4] <<
        ":"     << burst_errors[5] <<
        ":"     << burst_errors[6] <<
        ":"     << burst_errors[7] <<
        endl;
}

/**
 * @brief Bit flip histogram: For each possbile bit flip in a cache line (512
 * bits), dump out the number of cache lines that match the bit flip count.
 **/
void write_histo_file(ofstream& histo_file, uint bank,
        map<int,int>* err_map_bank, const int cline_size_bits)
{
    histo_file << "BANK " << bank << " PDF:";
    int cnt;
    // Go through each possbile bit flip
    for (int berr = 0; berr <= cline_size_bits; berr++)
    {
        cnt = 0;
        map<int,int>::iterator err_map_find = err_map_bank->find(berr) ;
        if (err_map_find != err_map_bank->end())
            cnt = err_map_find->second;
        histo_file << " " << cnt;
    }
    err_map_bank->clear();
    histo_file << endl;
}

/**
 * @brief Transfer the read cache line from the FPGA to the host machine and
 * store it in a buffer (size 16 32-bit elements => 512 bits => 64 bytes). Then
 * compare the read data with the written pattern.
 *
 * @burst_errors store the number bit flips in each transfer (eight in total)
 * in a data burst
 **/
int recv_compare(fpga_t* fpga, int ch, uint8_t pattern, unsigned burst_errors[])
{
    //uint rbuf[16];
    uint rbuf[32]; // use a bigger (2x) buffer size

    int num_recv = 0 ;
    #ifndef SANDBOX
    //num_recv = fpga_recv(fpga, ch, (void*)rbuf, 16, 1000);
    num_recv = fpga_recv(fpga, ch, (void*)rbuf, 32, 1000);
    if (num_recv != 16)
    {
        detail_print(RED "Received %d words instead of 16.\n" RESET, num_recv);
        return -1 ;
    }

    uint8_t* rbuf8 = (uint8_t *) rbuf;
    int err_cnt = 0;

    // Compare the data pattern to each transfer of a data burst
    for(int j = 0; j < 8; j++) {
        int burst_error_cnt = 0 ;
        for (int k = 0; k < 8; k++) {
            uint8_t diff = rbuf8[8*j+k] ^ pattern;
            uint8_t pattern_copy = pattern;
            for (int ci = 0; ci < 8; ci++) {
                err_cnt += (diff & 1);
                burst_error_cnt += (diff & 1);
                if (pattern_copy & 1)
                  toggle10 += (diff & 1); //1 to 0 toggle
                else
                  toggle01 += (diff & 1); //0 to 1 toggle
                pattern_copy >>= 1;
                diff >>= 1;
            }
        }
        burst_errors[j] += burst_error_cnt ;
    }
    // A cache line is only 64 bytes (512 bits)
    assert(err_cnt <= 512);
    return err_cnt;
    #else 
    return 0;
    #endif
}

/**
 * @brief Activate the target row -> Write data to one column -> Precharge bank
 **/
void writeCol(fpga_t* fpga, const uint col, const uint row, const uint bank,
              const uint8_t pattern, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    //reuse the provided CmdQueue to avoid dynamic allocation for each call
    else
        cq->size = 0;

    //Precharge target bank (just in case if its left activated)
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE));
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT)); //Activate target row
    cq->insert(genWaitCMD(DEF_TRCD)); //Wait for tRCD
    cq->insert(genWriteCMD(col, bank, pattern)); // Write the data
    cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    cq->insert(genWaitCMD(3)); //Wait some more in any case
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE)); //Precharge target bank
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP

    //START Transaction
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

/**
 * @brief Write a single pattern to every cache line in a specified row.
 *
 * @alternate_col_patt invert the pattern every other column. Even col is always
 * pattern and odd col is always ~pattern.
 **/
void writeRowFlexRCD(fpga_t* fpga, uint row, uint bank, uint8_t pattern,
        CmdQueue*& cq, uint ncol, uint trcd, uint start_col,
        bool alternate_col_patt)
{
    int ch = 0; //riffa channel should always be 0
    if(cq == nullptr)
        cq = new CmdQueue();
    //reuse the provided CmdQueue to avoid dynamic allocation for each call
    else
        cq->size = 0;

    //Precharge target bank (just in case if its left activated)
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE));
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT)); //Activate target row

    // -1 due to the elapse time of the cmd issue cycle
    if (trcd > 1)
        cq->insert(genWaitCMD(trcd - 1));

    // adjust the first pattern being written based on the first col location
    if (alternate_col_patt)
        pattern = ((start_col/8) % 2 == 0) ? pattern : (uint8_t)~pattern;

    //Write to some columns in a row
    for(int i = start_col; i < (ncol+start_col); i+=8) {
        cq->insert(genWriteCMD(i, bank, pattern));
        cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
        if (alternate_col_patt)
            pattern = (uint8_t)~pattern;
    }

    cq->insert(genWaitCMD(3)); //Wait some more in any case
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE)); //Precharge target bank
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP

    //START Transaction
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

/**
 * @brief Write a single pattern to every cache line in a specified row with
 * the default tRCD value and use a single pattern
 **/
void writeRow(fpga_t* fpga, uint row, uint bank, uint8_t pattern,
        CmdQueue*& cq, uint ncol)
{
    writeRowFlexRCD(fpga, row, bank, pattern, cq, ncol, DEF_TRCD, 0, false);
}

/**
 * @brief Read N cache lines from a row and compare it to the written value
 *
 * @alternate_col_patt invert the pattern every other column. Even col is always
 * pattern and odd col is always ~pattern.
 **/
int readAndCompareRow(fpga_t* fpga, const uint row, const uint bank,
        uint8_t pattern, CmdQueue*& cq, const uint ncol, uint start_col,
        bool alternate_col_patt)
{
    int ch = 0; //riffa channel should always be 0
    if(cq == nullptr)
        cq = new CmdQueue();
    //reuse the provided CmdQueue to avoid dynamic allocation for each call
    else
        cq->size = 0;

    //Precharge target bank (just in case if its left activated)
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE));
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT)); //Activate target row
    cq->insert(genWaitCMD(DEF_TRCD-1)); //Wait for tRCD

    //Read the entire row
    for(int i = start_col; i < (ncol+start_col); i+=8) {
        cq->insert(genReadCMD(i, bank));
        cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    }

    cq->insert(genWaitCMD(3)); //Wait some more in any case
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE)); //Precharge target bank
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP

    // START Transaction
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);

    // Compare each cache line's result
    int total_bit_error = 0;
    bool fpga_err = false;
    // adjust the first pattern being read based on the first col location
    if (alternate_col_patt)
        pattern = ((start_col/8) % 2 == 0) ? pattern : (uint8_t)~pattern;

    for(int i = 0; i < ncol; i+=8) {
        //Receive the data
        unsigned burst_errors[8] = {0,0,0,0,0,0,0,0} ;
        int err_cnt = recv_compare(fpga, ch, pattern, burst_errors) ;
        if (err_cnt > 0) {
            total_bit_error += err_cnt;
            debug_print("RdCmp Error at Col: %d, Row: %u, Bank: %u, cnt: %d \n",
                    i, row, bank, err_cnt);
        }
        else if (-1 == err_cnt) {
            fpga_err = true;
            detail_print(RED "FPGA Error at Col: %d, Row: %u, Bank: %u\n" RESET, i, row, bank);
        }
        if (alternate_col_patt)
            pattern = (uint8_t)~pattern;
    }
    return fpga_err ? -1 : total_bit_error;
}

int readAndCompareRow(fpga_t* fpga, const uint row, const uint bank,
        const uint8_t pattern, CmdQueue*& cq, const uint ncol, uint start_col)
{
    return readAndCompareRow(fpga, row, bank, pattern, cq, ncol, start_col, false);

}

/**
 * @brief Read N cache lines from a row and compare it to the written value
 **/
int readAndCompareRow(fpga_t* fpga, const uint row, const uint bank,
        const uint8_t pattern, CmdQueue*& cq, const uint ncol)
{
    return readAndCompareRow(fpga, row, bank, pattern, cq, ncol, 0);
}

/**
 * @brief Read every cache line from a row and compare it to the written value
 * Leave the row open(activated) after the operation
 **/
int readAndCompareRowOpen(fpga_t* fpga, const uint row, const uint bank,
        const uint8_t pattern, CmdQueue*& cq, const uint ncol)
{
    int ch = 0; //riffa channel should always be 0
    if(cq == nullptr)
        cq = new CmdQueue();
    //reuse the provided CmdQueue to avoid dynamic allocation for each call
    else
        cq->size = 0;

    //Precharge target bank (just in case if its left activated)
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE));
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT)); //Activate target row
    cq->insert(genWaitCMD(DEF_TRCD)); //Wait for tRCD

    //Read the entire row
    for(int i = 0; i < ncol; i+=8)
    {
        cq->insert(genReadCMD(i, bank));
        cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    }

    cq->insert(genWaitCMD(3)); //Wait some more in any case

    /* NOT PRECHARGING AT THE END> LEAVING ROW ACT.
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE)); //Precharge target bank
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
    */

    // START Transaction
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);

    // Compare each cache line's result
    int error = 0;
    for(int i = 0; i < ncol; i+=8)
    {
        //Receive the data
        unsigned burst_errors[8] = {0,0,0,0,0,0,0,0} ;
        int err_cnt ;
        err_cnt = recv_compare(fpga, ch, pattern, burst_errors) ;
        if (err_cnt > 0)
        {
            error = -1;
            printf("Error at Col: %d, Row: %u, Bank: %u, cnt: %d \n",
                    i, row, bank, err_cnt);
        }
        else if (-1 == err_cnt) //timeout
        {
            error = -2;
        }
    }

    return error ;
}

/**
 * @brief Read a cache line with a user-specified tRCD
 * @return number of bit errors in the column
 **/
int readCachelineMyTrcd(fpga_t* fpga, const uint col, const uint row,
        const uint bank, const uint8_t pattern, const uint trcd,
        CmdQueue*& cq, unsigned burst_errors[])
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    //reuse the provided CmdQueue to avoid dynamic allocation for each call
    else
        cq->size = 0;

    //Activate target row
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

    // Additional wait for tRCD if it's at least 2 cycles
    // (-1 due to the elapse time of the cmd issue cycle)
    if (trcd > 1)
        cq->insert(genWaitCMD(trcd - 1));

    cq->insert(genReadCMD(col, bank));
    cq->insert(genWaitCMD(DEF_TRAS)); // Wait for RAS before precharge
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE)); //Precharge target bank
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP

    //START Transaction
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
    return recv_compare(fpga, ch, pattern, burst_errors);
}

/**
 * @brief Go through one column at a time across all rows first, then wraps
 * around back to the first row.
             |-----retention------|
 | write col |-----wait-----------|read col (default rcd), read col (fast rcd) |
 **/
int test_trcd_col_order(fpga_t* fpga, const uint retention, const uint trcd,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern,
        bool count_enable, uint nrow, uint ncol, bool bypass_verif)
{
    assert(trcd > 0); //trcd should be at elast one cycle (2.5ns)
    int return_flag = 0 ;
    int failed_verf_read = 0;
    int fpga_errors = 0;

    toggle10 = 0;
    toggle01 = 0;

    CmdQueue* cq = nullptr;
    // Init timers
    GET_TIME_INIT(2);
    // Error map -- key: number of bit errors in a cache line
    // value: number of matching cache lines
    map<int, int> err_map_bank;
    const int cline_size_bits = 512;

    histo_file << "READ    " << retention << "ms" << " " << trcd <<
        " " <<  DEF_TRAS << " " << DEF_TRP << " " << DEF_TWR << " :" << endl <<
        endl << "Pattern 0x" << hex << unsigned(pattern) << dec << endl;
    if (count_enable)
        count_file << "READ    " << retention << "ms" << " tRCD:" << trcd <<
            "Pattern 0x" << hex << unsigned(pattern) << dec << endl;

    // Start going through every bank
    for (uint bank = 0; bank < NUM_BANKS; bank++) {
        cout << "Testing bank" << bank << endl;
        int total_bank_cl_errs = 0;
        // Column order
        for (uint col = 0; col < ncol; col += BURST_LEN) {
            cout  << "\rTesting Column: " << col;
            fflush(stdout);
            for (uint row = 0; row < nrow; row++) {
                // Write a column
                turnBus(fpga, BUSDIR::WRITE, cq);
                writeCol(fpga, col, row, bank, pattern, cq);

                // Wait for the retention (ms)
                usleep(retention * 1000);

                // Verify read: Read the data to make sure it's fully written
                turnBus(fpga, BUSDIR::READ, cq);
                unsigned burst_errors[8] = {0,0,0,0,0,0,0,0} ;
                int err_cnt = 0;
                if (!bypass_verif)
                    err_cnt = readCachelineMyTrcd(fpga, col, row, bank, pattern,
                        DEF_TRCD, cq, burst_errors);

                // FPGA error on read -- skip it
                if (err_cnt == -1) {
                    if (fpga_errors > 100) {
                        detail_print(RED "Too many FPGA errors! Terminate the test\n" RESET);
                        return -2;
                    }
                    fpga_errors++;
                    continue;
                }
                // Errors on a verified read
                else if (err_cnt) {
                    detail_print(RED "FAILED on first read! err count=%d\n" RESET,
                            err_cnt);
                    // Tolerate some of these errors as it's unknown if the failure
                    // occurs at write or read
                    if (failed_verf_read < 100) {
                        failed_verf_read++;
                        continue; // Skip on this row & column for now
                    }
                    else {
                        histo_file << "FAILED on first read!\n";
                        detail_print(RED "Too many FAILED on first read! \
                                Terminate the test\n" RESET);
                        return -2;
                    }
                }

                ///////////////////////
                // Fast tRCD read
                ///////////////////////
                err_cnt = readCachelineMyTrcd(fpga, col, row, bank,
                        pattern, trcd, cq, burst_errors);
                // If on FPGA errors, skip this row for N times
                if (err_cnt == -1) {
                    if (fpga_errors > 100) {
                        detail_print(RED "Too many FPGA errors! Terminate the test\n" RESET);
                        return -2;
                    }
                    fpga_errors++;
                    continue ;
                }
                err_map_bank[err_cnt]++;

                // Count every error location
                if (err_cnt > 0) {
                    return_flag = -1;
                    write_count_file(row,col,bank, err_cnt, count_file,
                            count_enable, burst_errors);
                    total_bank_cl_errs++;
                }
            }
        }
        cout << endl;
        // -- Done going through the entire bank --

        // Report some simple stats: % of cache lines with errors
        if (total_bank_cl_errs > 0)
        {
            double total_cl = nrow * ncol / BURST_LEN;
            cout << RED "Bank Error $lines %: " <<total_bank_cl_errs / total_cl
                << "" RESET << endl;
        }

        // Dump out the histogram to a file
        write_histo_file(histo_file, bank, &err_map_bank, cline_size_bits);
    }
    histo_file << endl;
    histo_file << "Total 0->1 toggles for trcd="<<trcd<<" :" <<toggle01 <<endl;
    histo_file << "Total 1->0 toggles for trcd="<<trcd<<" :" <<toggle10 <<endl;

    if (cq)
        delete cq;
    return return_flag;
}

/**
 * @brief Test the effect of reduced retention time
 * Write as many rows as possible within reterntion window.
 * Verify whether normal reads are correct.
 * After retention period, check by accessing at reduced RAS
 * |------Retention period---------|----Reduced tRAS testing---|
 * |write_row,verify_row,wr_row....|(PRE-ACT-tRAS-PRE-ACT-RD)..|
 **/
int test_tras_row_order(fpga_t* fpga, const uint retention, const uint tras,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern,
        bool count_enable, uint nrow, uint ncol)
{
    assert(tras > 0); //tras should be at elast one cycle (2.5ns)

    //If any of the banks have errors, this will be set to -1
    int return_flag = 0 ;

    int ch = 0 ; //Riffa channel should always be 0
    CmdQueue* cq = nullptr;
    // Init timers
    GET_TIME_INIT(2);

    // Error map -- key: number of bit errors in a cache line
    // value: number of matching cache lines
    map<int, int> err_map_bank;
    const int cline_size_bits = 512;

    histo_file << "READ    " << retention <<"ms"<< " " << DEF_TRCD <<
        " " <<  tras <<" "<< DEF_TRP <<" "<< DEF_TWR <<" :"<< endl <<
        endl <<"Pattern 0x"<< hex << unsigned(pattern) << dec << endl;

    if (count_enable)
        count_file <<"READ    "<< retention <<"ms"<<" tRAS:"<< tras <<
            "Pattern 0x" << hex << unsigned(pattern) << dec << endl;

    // Start going through every bank
    for (uint bank = 0; bank < NUM_BANKS; bank++)
    {
        cout << "Testing bank" << bank << endl;
        int total_bank_cl_errs = 0;
        // Row order
        uint row = 0;
        uint prev_row = 0;
        while (row < nrow)
        {
            GET_TIME_VAL(0); //Start of the retention window

            // Write and read-verify many rows in a retention window
            do
            {
                cout  << "\rWriting Row: " << row;
                fflush(stdout);
                turnBus(fpga, BUSDIR::WRITE, cq);
                writeRow(fpga, row, bank, pattern, cq, ncol);

                //Verify Row
                turnBus(fpga, BUSDIR::READ, cq);
                int read_out;
                read_out = readAndCompareRow(fpga,row,bank, pattern, cq, ncol);
                if (read_out) //Errors present in the normal read data !
                {
                    printf(RED "\nFAILED on normal read after write!!\n" RESET);
                    histo_file << "FAILED on normal read after write!! \n";
                    return -1;
                }


                GET_TIME_VAL(1); //Update current time
                row++;
                if (row > nrow)
                    break;
            } while ((TIME_VAL_TO_MS(1) - TIME_VAL_TO_MS(0)) < retention);


            for (int row_read = prev_row; row_read < row; row_read++)
            {
                int rerun_flag = 0 ;
                cout << "\rReading Row: " << row_read ;
                fflush(stdout) ;
                //Reinit command queue
                if(cq == nullptr)
                    cq = new CmdQueue();
                else
                    cq->size = 0;

                // Precharge the bank
                cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE));
                cq->insert(genWaitCMD(DEF_TRP)); //tRP

                // Activate of current row with reduced tRAS, which means that
                // we are simply issuing an ACT-PRE pair of commands.
                cq->insert(genRowCMD(row_read, bank, MC_CMD::ACT));
                if (tras > 1)
                    cq->insert(genWaitCMD(tras-1)); //tRAS
                cq->insert(genRowCMD(0, bank, MC_CMD::PRE));
                cq->insert(genWaitCMD(DEF_TRP)); //tRP

                // Re-activate and read the entire row
                cq->insert(genRowCMD(row_read, bank, MC_CMD::ACT));
                cq->insert(genWaitCMD(DEF_TRCD)); //Wait for tRCD
                //Read the entire row
                for(int i = 0; i < ncol; i+=8)
                {
                    cq->insert(genReadCMD(i, bank));
                    cq->insert(genWaitCMD(DEF_TCL + 4));
                }
                cq->insert(genWaitCMD(3));
                cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE));
                cq->insert(genWaitCMD(DEF_TRP));

                // START Transaction
                cq->insert(genStartTR());
                fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size,
                          0, 1, 0);

                // Check if there's any error due to reduced tRAS
                for (int col=0; col < ncol; col+=8)
                {
                    unsigned burst_errors[8] = {0,0,0,0,0,0,0,0} ;
                    int err_cnt  ;
                    err_cnt = recv_compare(fpga, ch, pattern, burst_errors);

                    if (err_cnt <0)
                    {
                        rerun_flag = 1 ;
                        break ;     //Improper read, discard and restart
                    }

                    err_map_bank[err_cnt]++;
                    // Count every error location
                    if (err_cnt > 0)
                    {
                        write_count_file(row,col,bank, err_cnt,
                                count_file, count_enable, burst_errors);
                        total_bank_cl_errs++;
                    }
                }

                // Recover from fpga/pcie fault
                if (rerun_flag == 1)
                {
                    rerun_flag = 0 ;
                    row_read-- ; //Rerun this row as read timed out
                    continue ;
                }
            }
            prev_row = row ;
        }
        cout << endl;

        // -- Done going through the entire bank --
        // Report some simple stats: % of cache lines with errors
        if (total_bank_cl_errs > 0)
        {
            double total_cl = nrow * ncol / BURST_LEN;
            cout << RED "Bank Error $lines %: " <<total_bank_cl_errs / total_cl
                << "" RESET << endl;
            return_flag = -1 ;
        }

        // Dump out the histogram to a file
        write_histo_file(histo_file, bank, &err_map_bank, cline_size_bits);
    }
    histo_file << endl;
    return return_flag ;
}

/**
 * @brief Test the effect of reducing tWR.
 **/
int test_twr_col_order(fpga_t* fpga, const uint retention, const uint twr,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern,
        bool count_enable, uint nrow, uint ncol)
{
    assert(twr >= 0); //twr can be merged with tCL+burst, so 0 cycles is valid

    int return_flag = 0 ;
    //If any of the banks have errors, return_flag will be set to -1
    CmdQueue* cq = nullptr;
    // Init timers
    GET_TIME_INIT(2);
    // Error map -- key: number of bit errors in a cache line
    // value: number of matching cache lines
    map<int, int> err_map_bank;
    const int cline_size_bits = 512;

    histo_file << "READ    " << retention << "ms" << " " << DEF_TRCD <<
        " " <<  DEF_TRAS << " " << DEF_TRP << " " << twr << " :" << endl <<
        endl << "Pattern 0x" << hex << unsigned(pattern) << dec << endl;
    if (count_enable)
        count_file << "READ    " << retention << "ms" << " tWR:" << twr <<
            "Pattern 0x" << hex << unsigned(pattern) << dec << endl;

    // Start going through every bank
    for (uint bank = 0; bank < NUM_BANKS; bank++)
    {
        cout << "Testing bank" << bank << endl;
        int total_bank_cl_errs = 0;
        // Column order
        for (uint col = 0; col < ncol; col += 8)
        {
            cout  << "\rTesting Column: " << col;
            fflush(stdout);
            for (uint row = 0; row < nrow; row++)
            {
                int ch = 0; //riffa channel should always be 0
                if(cq == nullptr)
                    cq = new CmdQueue();
                else
                    cq->size = 0;

                turnBus(fpga, BUSDIR::WRITE, cq);
                //Precharge target bank (just in case if its left activated)
                cq->insert(genRowCMD(0, bank, MC_CMD::PRE));
                //Wait for tRP
                cq->insert(genWaitCMD(DEF_TRP));
                //Activate target row
                cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
                //Wait for tRCD
                cq->insert(genWaitCMD(DEF_TRCD));
                cq->insert(genWriteCMD(col, bank, pattern));

                //We need to wait for tCL and 4 cycles burst (double data-rate)
                //cq->insert(genWaitCMD(DEF_TCL + 4 + twr-1));
                if (twr > 1)
                    cq->insert(genWaitCMD(twr-1));
                //Precharge target bank
                cq->insert(genRowCMD(0, bank, MC_CMD::PRE));
                //Wait for tRP
                cq->insert(genWaitCMD(DEF_TRP));
                //START Transaction
                cq->insert(genStartTR());

                fpga_send(fpga,ch,(void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
                GET_TIME_VAL(0);

                do{
                    GET_TIME_VAL(1);
                } while((TIME_VAL_TO_MS(1) - TIME_VAL_TO_MS(0)) < retention);


                turnBus(fpga, BUSDIR::READ, cq);
                // Default tRCD read
                //cout << "compare col" << endl;
                unsigned burst_errors[8] = {0,0,0,0,0,0,0,0} ;

                int err_cnt ;
                err_cnt = readCachelineMyTrcd(fpga, col, row, bank, pattern,
                        DEF_TRCD, cq, burst_errors);

                if (err_cnt == -1)
                {
                    row-- ;
                    continue ;
                }
                err_map_bank[err_cnt]++;
                // Count every error location
                if (err_cnt > 0)
                {
                    write_count_file(row,col,bank, err_cnt, count_file,
                            count_enable, burst_errors);
                    total_bank_cl_errs++;
                }
            }
        }
        cout << endl;

        // -- Done going through the entire bank --

        // Report some simple stats: % of cache lines with errors
        if (total_bank_cl_errs > 0)
        {
            return_flag = -1 ;
            double total_cl = nrow * ncol / BURST_LEN;
            cout<< RED "Bank Error $lines %: "<<total_bank_cl_errs / total_cl<<
                "" RESET << endl;
        }

        // Dump out the histogram to a file
        write_histo_file(histo_file, bank, &err_map_bank, cline_size_bits);

    }
    histo_file << endl;
    return return_flag ;
}

/**
 * @brief Test the reliability of cells as tRP reduces.
 * Reduction of tRP affects the voltage values on the bitlines. Ideally, they
 * would be vdd/2. But with lower tRP vals, the precharged bitline values will
 * be closer to the activated values. As a result, if the next activated row
 * has values closer to the previous activaed row, then the bitlines will be
 * biased toward to the newly acitated row, reducing its activation time.
 *
 * Go through one row at a time across all columns
 *  Steps :
 *  1. Write row 0 with pattern0
 *  2. Write row 1 with pattern1
 *  3. Read row 0 to check valid data
 *  4. Read row 1 to check vaild data
 **/
int test_trp_row_order(fpga_t* fpga, const uint retention, const uint trp,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern0,
        bool count_enable, uint nrow, uint ncol, bool bypass_verif)
{
    assert(trp > 0); //trp should be at elast one cycle (2.5ns)

    int return_flag = 0;

    toggle01 = 0;
    toggle10 = 0;

    uint8_t pattern1 = ~pattern0;

    CmdQueue* cq = nullptr;
    // Init timers
    //GET_TIME_INIT(2);
    // Error map -- key: number of bit errors in a cache line
    // value: number of matching cache lines
    map<int, int> err_map_bank;
    const int cline_size_bits = 512;

    histo_file << "READ    " << retention << "ms" << " " << DEF_TRCD <<
        " " <<  DEF_TRAS << " " << trp << " " << DEF_TWR << " :" << endl <<
        endl << "Pattern0 0x" << hex << unsigned(pattern0) << "\tPattern1 0x"
        << hex << unsigned(pattern1) << dec << endl;

    if (count_enable)
        count_file << "READ    "<< retention << "ms" << " tRCD:" << DEF_TRCD <<
            "Pattern 0x" << hex << unsigned(pattern0) << "\tPattern1 0x"
            << hex << unsigned(pattern1)  << dec << endl;

    int write_flag = 0 ;
    int rerun_flag = 0 ;

    // Start going through every bank
    for (uint bank = 0; bank < NUM_BANKS; bank++)
    {
        cout << "Testing bank" << bank << endl;
        int total_bank_cl_errs = 0;
        // Row order
        for (uint row = 0; row < nrow; row++)
        {
            cout  << "\rTesting Row: " << row;
            fflush(stdout);

            //****************Setting up data in the rows***************//
            uint8_t pattern, alt_pattern ;
            pattern = write_flag ? pattern1 : pattern0 ;
            alt_pattern = write_flag ? pattern0 : pattern1 ;

            int read_out = 0 ;

            if (row == 0) {
                turnBus(fpga, BUSDIR::WRITE, cq);
                writeRow(fpga, row, bank, pattern, cq, ncol) ;
                turnBus(fpga, BUSDIR::READ, cq);
                if (!bypass_verif) {
                    read_out = readAndCompareRowOpen(fpga,row,bank,pattern,
                            cq,ncol);

                    if (-1 == read_out) {
                        printf(RED "FAILED on normal read after write! \n"
                                RESET);
                        histo_file << "FAILED on normal read after write!\n";
                        return -2;
                    }
                    else if (-2 == read_out) {
                        row-- ;
                        continue ;
                    }
                }
            }

            if (row < nrow-1)
            {
                turnBus(fpga, BUSDIR::WRITE, cq);
                writeRow(fpga, row+1, bank, alt_pattern, cq, ncol);
                turnBus(fpga, BUSDIR::READ, cq);
                if (!bypass_verif) {
                    read_out = readAndCompareRowOpen(fpga,row+1,bank,
                            alt_pattern,cq,ncol);
                    if (-1 == read_out)
                    {
                        printf(RED "FAILED on normal read after write! \n"
                                RESET);
                        histo_file << "FAILED on normal read after write!\n";
                        return -2;
                    }
                    else if (-2 == read_out)
                    {
                        row--;
                        continue;
                    }
                }
            }
            else
            {
                turnBus(fpga, BUSDIR::WRITE, cq);
                writeRow(fpga, row-1, bank, alt_pattern, cq, ncol);
                turnBus(fpga, BUSDIR::READ, cq);
                if (!bypass_verif) {

                    read_out = readAndCompareRowOpen(fpga, row-1, bank,
                            alt_pattern, cq,ncol);
                    if (-1 == read_out)
                    {
                        printf(RED "FAILED on normal read after write! \n"
                                RESET);
                        histo_file << "FAILED on normal read after write!\n";
                        return -2;
                    }
                    else if (-2 == read_out)
                    {
                        row-- ;
                        continue ;
                    }
                }
            }

            //******Actual tRP testing begins here****************//
            int ch = 0; //riffa channel should always be 0

            if(cq == nullptr)
                cq = new CmdQueue();
            else
                cq->size = 0;

            //Precharge target bank
            cq->insert(genRowCMD(0, bank, MC_CMD::PRE));

            //Fast tRP
            if (trp > 1)
                cq->insert(genWaitCMD(trp-1));

            //Activate target row
            cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

            //Wait for tRCD
            cq->insert(genWaitCMD(DEF_TRCD));

            //Read the entire row
            for(int i = 0; i < ncol; i+=8){
                cq->insert(genReadCMD(i, bank));

                //We need to wait for tCL and 4 cycles burst
                cq->insert(genWaitCMD(DEF_TCL + 4));
            }

            //Wait some more in any case
            cq->insert(genWaitCMD(3));
            //Precharge target bank
            cq->insert(genRowCMD(0, bank, MC_CMD::PRE));
            //Wait for tRP
            cq->insert(genWaitCMD(DEF_TRP));

            //START Transaction
            cq->insert(genStartTR());
            fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);

            //FPGA has read out from the DRAM
            //Transmit it to host through fpga_recv
            for (int col=0; col < ncol; col+=8)
            {
                unsigned burst_errors[8] = {0,0,0,0,0,0,0,0};
                int err_cnt;
                err_cnt = recv_compare(fpga, ch, pattern, burst_errors);

                if (err_cnt <0)
                {
                    rerun_flag = 1;
                    break;     //Improper read, discard and restart
                }

                err_map_bank[err_cnt]++;
                // Count every error location
                if (err_cnt > 0)
                {
                    return_flag = -1;
                    write_count_file(row,col,bank, err_cnt,
                            count_file, count_enable, burst_errors);
                    total_bank_cl_errs++;
                }
            }

            if (rerun_flag == 1)
            {
                rerun_flag = 0;
                row--; //Rerun this row as read timed out
                continue;
            }

            if (write_flag == 0)
                write_flag = 1;
            else
                write_flag = 0;

        }
        cout << endl;

        // -- Done going through the entire bank --

        // Report some simple stats: % of cache lines with errors
        if (total_bank_cl_errs > 0)
        {
            double total_cl = nrow * ncol / BURST_LEN;
            cout << RED "Bank Error $lines %: " << total_bank_cl_errs / total_cl <<
                "" RESET << endl;
        }

        // Dump out the histogram to a file
        write_histo_file(histo_file, bank, &err_map_bank, cline_size_bits);
    }
    histo_file << endl;

    histo_file << "Total 0->1 toggles for trp="<<trp<<" :" <<toggle01 <<endl;
    histo_file << "Total 1->0 toggles for trp="<<trp<<" :" <<toggle10 <<endl;

    if (cq)
        delete cq;

    return return_flag;
}

/**
 * @brief Writes the given pattern into a specified bank,row by using open
 * page policy. Fills the row in 8x burst mode
 * 
 * TO-FIX 
 **/
void openRowWrite(fpga_t* fpga, uint row, uint bank, uint8_t pattern, uint trcd, CmdQueue*& cq)
{
  int ch = 0; //riffa channel should always be 0

  if(cq == nullptr)
    cq = new CmdQueue();
  else
    cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

  turnBus(fpga, BUSDIR::WRITE, cq);

  if ((bank != 0) && (row == 0))
    cq->insert(genRowCMD(0, bank-1, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank
  else
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE));

  //Wait for tRP
  cq->insert(genWaitCMD(DEF_TRP));

  //Activate target row
  cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

  //Wait for tRCD
  if (trcd > 1)
    cq->insert(genWaitCMD(trcd - 1));

  //Write to the entire row
  for(int i = 0; i < NUM_COLS; i+=8){ //we use 8x burst mode
    cq->insert(genWriteCMD(i, bank, pattern));
    //We need to wait for tCL and 4 cycles burst (double data-rate)
    cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
  }
  cq->insert(genStartTR());
  fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

/**
 * @brief Reads and compares the data in the specified row with a given
 * pattern. Precharging follows the open-page policy. 8x burst mode is used
 * in read.
 * 
 * TO-FIX 
 **/
void openRowRead(fpga_t* fpga,  const uint row, const uint bank,
                 const uint8_t pattern, const uint trcd, CmdQueue*& cq)
{
    uint errors = 0;
    uint prev_errors = 0;
    uint bytes_read = 0;

    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

    turnBus(fpga, BUSDIR::READ, cq);

    //Precharge target bank
    if ((bank != 0) && (row == 0))
      cq->insert(genRowCMD(0, bank-1, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank
    else
      cq->insert(genRowCMD(0, bank, MC_CMD::PRE));

    //Wait for tRP
    cq->insert(genWaitCMD(DEF_TRP));

    //Activate target row
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

    if (trcd > 1)
        cq->insert(genWaitCMD(trcd - 1));

    //Read the entire row
    for(int i = 0; i < NUM_COLS; i+=8){ //we use 8x burst mode
        cq->insert(genReadCMD(i, bank));

        //We need to wait for tCL and 4 cycles burst (double data-rate)
        cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    }

    //START Transaction
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);

    //Receive the data
    uint rbuf[16];
    for(int i = 0; i < NUM_COLS; i+=8){ //we receive a single burst at two times (32 bytes each)
      fpga_recv(fpga, ch, (void*)rbuf, 16, 0);

      //compare with the pattern
      uint8_t* rbuf8 = (uint8_t *) rbuf;
      for(int j = 0; j < 64; j++){
        bytes_read +=1;
        if(rbuf8[j] != pattern) {
          errors +=1;
          //printf("Error at Col: %d, Row: %u, Bank: %u, DATA: %x \n", i, row, bank, rbuf8[j]);
        }
      }
    }

    if (row == NUM_ROWS-1) {
      printf("Bytes read = %u\n", bytes_read);
      printf("Errors = %d\n", errors);
    }
}

/**
 * @brief precharges the given bank. The latency of t_ras and t_rp are
 * implemented in the function. User should consider that to avoid low
 * utilization.
 **/
void precharge(fpga_t *fpga, CmdQueue*& cq, uint bank)
{
    int ch = 0;

    if(cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;

    turnBus(fpga, BUSDIR::READ, cq);

    // TODO: All this waiting may be unnecessary
    // Wait TRAS
    cq->insert(genWaitCMD(DEF_TRAS));
    // Precharge target bank
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); // 0 specifies to PRE the bank
    // Wait TRP
    cq->insert(genWaitCMD(DEF_TRP));
    //START Transaction
    cq->insert(genStartTR());

    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

/**
 * brief Writes the given pattern into a particular column of a specified
 * row of a specified bank.
 **/
void writeColBuffered(fpga_t* fpga, const uint col, const uint row,
                      const uint bank, const uint8_t pattern, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
      cq = new CmdQueue();
    else
      cq->size = 0;

    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE));
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT)); //Activate target row
    cq->insert(genWaitCMD(DEF_TRCD)); //Wait for tRCD
    cq->insert(genWriteCMD(col, bank, pattern)); // Write the data
    cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    cq->insert(genWaitCMD(3)); //Wait some more in any case
    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE)); //Precharge target bank
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

/**
 * brief Reads a particular column of a specified row of a specified bank.
 * Compares the received data with the given pattern for error check.
 **/
void readColBuffered(fpga_t* fpga, const uint col, const uint row, const uint bank,
                     const uint8_t pattern, const uint trcd, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
      cq = new CmdQueue();
    else
      cq->size = 0;

    cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE));
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
    cq->insert(genWaitCMD(DEF_TRCD));
    cq->insert(genReadCMD(col, bank));
    cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    cq->insert(genWaitCMD(3));
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank
    cq->insert(genWaitCMD(DEF_TRP)); //wait TRP
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
    /*
    uint rbuf[16];
    fpga_recv(fpga, riffa_ch, (void*)rbuf, 16, 0);

    uint8_t* rbuf8 = (uint8_t *) rbuf;

    printf("Entering check...\n");
    for(int j = 0; j < 64; j++) {
      if(rbuf8[j] != pattern) {
        printf("Error at Byte: %d, Col: %d, Row: %u, Bank: %u, DATA: %x Correct Data: %x\n",
               j, col, row, bank, rbuf8[j], pattern);
      }
    }
    */

}
