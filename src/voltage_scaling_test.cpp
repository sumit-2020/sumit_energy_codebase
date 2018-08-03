/**
 * @brief Test routines to parallelize mulitple activations across banks. The
 * goal is to stress the DRAM utilization so that we can determine the min
 * operating voltage point
 *
 * @author Kevin Chang <kevincha@cmu.edu>
 * @author Abhijith Kashyap <akashyap@andrew.cmu.edu>
 **/
#include "test_routine.h"

// Helper function for the trrd test
void test_trrd_cmd_injection(uint bank, uint row, uint col,
        uint8_t pattern, uint8_t inv_pattern, CmdQueue* cq)
{
    // -- Begin command queue --
    cq->insert(genRowCMD(PRE_ALL_BANK, 0, MC_CMD::PRE));
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP

    //send the write cmds to write 4 cache lines on bank->bank+3
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
    cq->insert(genWaitCMD(3 + GUARDBAND)); //(tRRD-1) = 4-1 = 3
    cq->insert(genRowCMD(row, bank+1, MC_CMD::ACT));

    cq->insert(genWriteCMD(col, bank, pattern));

    cq->insert(genWaitCMD(2 + GUARDBAND)); //tRRD, but 1ck is used for wr cmd
    cq->insert(genRowCMD(row, bank+2, MC_CMD::ACT));
    cq->insert(genWaitCMD(1 + GUARDBAND)); //wr to wr delay
    cq->insert(genWriteCMD(col, bank+1, inv_pattern));

    cq->insert(genWaitCMD(1 + GUARDBAND)); //wr to wr delay
    cq->insert(genRowCMD(row, bank+3, MC_CMD::ACT)); //tRRD = 2+cmd
    cq->insert(genWaitCMD(2 + GUARDBAND)); //wr to wr delay
    cq->insert(genWriteCMD(col, bank+2, pattern));

    cq->insert(genWaitCMD(DEF_TBURST + GUARDBAND)); //wr to wr delay
    cq->insert(genWriteCMD(col, bank+3, inv_pattern));
    cq->insert(genWaitCMD(DEF_TBURST+DEF_TWR + GUARDBAND));

    //Turn bus cmds
    cq->insert(genBusDirCMD(BUSDIR::READ));
    cq->insert(genWaitCMD(5)); //wait for the bus to turn

    cq->insert(genRowCMD(PRE_ALL_BANK, 0, MC_CMD::PRE));
    cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP

    // 4 ACTs
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
    cq->insert(genWaitCMD(3 + RD_GUARDBAND)); //(tRRD-1) = 4-1 = 3
    cq->insert(genRowCMD(row, bank+1, MC_CMD::ACT));
    cq->insert(genReadCMD(col, bank));
    cq->insert(genWaitCMD(2 + RD_GUARDBAND)); //tRRD, but 1ck is used for wr cmd
    cq->insert(genRowCMD(row, bank+2, MC_CMD::ACT));
    cq->insert(genWaitCMD(1 + RD_GUARDBAND)); //wr to wr delay
    cq->insert(genReadCMD(col, bank+1));
    cq->insert(genWaitCMD(1));
    cq->insert(genRowCMD(row, bank+3, MC_CMD::ACT)); //tRRD = 2+cmd
    cq->insert(genWaitCMD(2 + RD_GUARDBAND)); //wr to wr delay
    cq->insert(genReadCMD(col, bank+2));
    cq->insert(genWaitCMD(DEF_TBURST + RD_GUARDBAND));
    cq->insert(genReadCMD(col, bank+3));
    cq->insert(genWaitCMD(DEF_TBURST + RD_GUARDBAND));
    //end of read of 4 cache lines

    //Read the activated rows again
    cq->insert(genReadCMD(col, bank));
    cq->insert(genWaitCMD(DEF_TBURST));
    cq->insert(genReadCMD(col, bank+1));
    cq->insert(genWaitCMD(DEF_TBURST));
    cq->insert(genReadCMD(col, bank+2));
    cq->insert(genWaitCMD(DEF_TBURST));
    cq->insert(genReadCMD(col, bank+3));
    cq->insert(genWaitCMD(DEF_TBURST));

    //Read the activated rows once more
    cq->insert(genReadCMD(col, bank));
    cq->insert(genWaitCMD(DEF_TBURST));
    cq->insert(genReadCMD(col, bank+1));
    cq->insert(genWaitCMD(DEF_TBURST));
    cq->insert(genReadCMD(col, bank+2));
    cq->insert(genWaitCMD(DEF_TBURST));
    cq->insert(genReadCMD(col, bank+3));
    cq->insert(genWaitCMD(DEF_TCL+DEF_TBURST));
    //end of re read of 4 cache lines

    cq->insert(genStartTR());
}

/*
 *
 * Time line of
 * <ACT B0>----<ACT B1><READ B0>--<ACT B2>---<READ B1>
 *    |----tRRD----|----tRRD---------|
 *    DEF : 4 cycles
 *    |-------tRCD----------|------tCL-------|
 *    DEF : 5 cycles             DEF : 6 cycles
 */
void test_trrd4ck_col_order(fpga_t* fpga, const uint retention,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern,
        bool count_enable, uint nrow, uint ncol)
{
    toggle10 = 0;
    toggle01 = 0;

    const int ch = 0; //fpga channel is always 0

    CmdQueue* cq = nullptr;

    // Record histogram of errors
    //vector<map<int, int>> err_map_bank (8);
    // v-Read # (3 in total), v-bank, m-err bits cnt, m-occurance
    vector<vector<map<int, int>>> err_map (3, vector<map<int,int>>(8));

    // Total number of erroneaous cache lines in each bank
    int total_bank_cl_errs[8] = {0,0,0,0,0,0,0,0};

    // Error categorization by comparing three reads
    vector<vector<int>> error_type(8, vector<int>(4)) ;

    histo_file << "READ    " << retention << "ms" << " " << DEF_TRCD <<
        " " <<  DEF_TRAS << " " << DEF_TRP << " " << DEF_TWR << " :" << endl <<
        endl << "Pattern0 0x" << hex << unsigned(pattern) << dec << endl;

    if (count_enable)
        count_file << "READ    "<< retention << "ms" << " tRCD:" << DEF_TRCD <<
        " " <<  DEF_TRAS << " " << DEF_TRP << " " << DEF_TWR << " :" << endl <<
            "Pattern 0x" << hex << unsigned(pattern) << dec << endl;

    // Record the number of faults
    int fpga_fault = 0;

    // Determine the alternate pattern type: 100% or 50% inversion
    uint8_t inv_pattern = (INVERT_PATT) ? ~pattern : pattern;
    inv_pattern = (INVERT_HALF) ? (pattern ^ 0xf0) : inv_pattern;
    cout << "Inverted Pattern: 0x" << hex << unsigned(inv_pattern) << dec << endl;

    const int bank_group_size = 4; // number of banks we are activating in parallel
    const int read_seq_size = 3; // number of reads from each bank

    for (uint col = 0; col < ncol; col+=8)
    {
        cout << "\rTesting Col: " << col;
        fflush(stdout);
        for (uint row = 0; row < nrow; row++)
        {
            for (int bank = 0; bank < NUM_BANKS; bank += bank_group_size)
            {
                //Precharge all banks
                //Need to reset queue anyway, so use the turnbus macro
                turnBus(fpga, BUSDIR::WRITE, cq);

                //reset queue
                if (cq == nullptr)
                    cq = new CmdQueue();
                else
                    cq->size = 0;

                // Inject commands for the test
                test_trrd_cmd_injection(bank, row, col, pattern, inv_pattern, cq);

                // Send all the commands
                fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);

                ///////////////////////////////////////
                // Parse the errors to record the BER
                ///////////////////////////////////////
                //0-3 read 1 for the four banks; 4-7 read 2; 8-11 read 3
                int err_cnt[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
                uint cache_line = 0;
                for (; cache_line < (bank_group_size * read_seq_size); cache_line++)
                {
                    unsigned burst_errors[8] = {0,0,0,0,0,0,0,0};

                    // Get the data through PCIe and compare with pattern
                    if ((cache_line%2)==0)
                        err_cnt[cache_line] =
                            recv_compare(fpga, ch, pattern, burst_errors);
                    else
                        err_cnt[cache_line] =
                            recv_compare(fpga, ch, inv_pattern, burst_errors);

                    // Recording the histograms of errors for the first read
                    // issued to each bank.

                    // Determine which reads
                    int read_seq = cache_line / bank_group_size;
                    // Determine which bank in the 4-bank group
                    int bank_offset = cache_line % bank_group_size;
                    int bit_flip = err_cnt[cache_line];

                    // Mark this read as 100% if it failed to read.
                    if (bit_flip == -1)
                    {
                        printf(RED "FAILED on reading data!! bank=%d read=%d\n" RESET,
                                bank+bank_offset, read_seq);
                        fpga_fault++;
                        if (fpga_fault > 10)
                            goto fault_recovery;
                        //for (int i = 0; i < 12; i++)
                        //    cout << "cl cnt:" << err_cnt[cache_line] << endl;
                        //exit(-1);
                        bit_flip = CACHE_LINE_BITS;
                    }

                    vector<map<int, int>>::iterator cur_bank_map;
                    cur_bank_map = err_map[read_seq].begin() + bank + bank_offset;
                    (*cur_bank_map)[bit_flip]++;

                    // Record the number of cache lines that have at least 1 bit of errors
                    if (bit_flip > 0)
                        total_bank_cl_errs[bank+bank_offset]++;

                    // Record every cache line's error and location if enabled
                    if (count_enable && bit_flip > 0)
                        write_count_file(row, col, bank+bank_offset, bit_flip,
                                count_file, count_enable, burst_errors);
                }

                /////////////////////////////////////////////////
                // Compare consecutive reads to the same bank
                /////////////////////////////////////////////////
                for (uint bank_idx=0; bank_idx<4; bank_idx++)
                {
                    int rd1 = err_cnt[bank_idx];
                    int rd2 = err_cnt[bank_idx+4];
                    int rd3 = err_cnt[bank_idx+8];

                    if (!(rd1 || rd2 || rd3))
                        error_type[bank+bank_idx][0]++; //Everything ok
                    else if (rd1 && !rd2 && !rd3)
                        // Core error timing related OR (rd2 and rd3) were done
                        // in a more stable voltage condition without overlapping with ACTs
                        error_type[bank+bank_idx][1]++;
                    else if (rd1 == rd2 && rd2 == rd3)
                        error_type[bank+bank_idx][2]++; //Core error - Persistent core errors
                    else
                        error_type[bank+bank_idx][3]++; //other types of inexplicable errors
                }
            }
        }
    }

    // Summary print out
    cout << endl;
    for (uint bank = 0; bank < NUM_BANKS; bank++) {
        if(total_bank_cl_errs[bank] > 0) {
            double total_cl = (nrow * ncol / BURST_LEN) * read_seq_size;
            cout << total_bank_cl_errs[bank] << endl;
            cout << total_cl << endl;
            cout << RED "Bank" << bank << "Error $lines %: " <<
                total_bank_cl_errs[bank] / total_cl << "" RESET << endl;
        }
        else
            cout << GREEN "Bank " << bank << " has no errors " RESET << endl;
    }

    // histogram dump to file
    for (int ri = 0; ri < read_seq_size; ri++) {
        histo_file << "Read " << ri << endl;
        for (int bi = 0; bi < NUM_BANKS; bi++) {
            vector<map<int, int> >::iterator cur_bank_map;
            cur_bank_map = err_map[ri].begin() + bi;
            write_histo_file(histo_file, bi, &(*cur_bank_map), CACHE_LINE_BITS);
        }
    }

    cout << endl;
    histo_file << endl;
    histo_file << endl;

    histo_file<< "Bank ID\tErrorFree\tErrorFirstRead\tSameErrorsThreeReads\tDontKnow"<<endl;
    histo_file<< "|------------------------------------------------------|"<<endl;
    for (uint bank = 0; bank < NUM_BANKS; bank++)
    {
        histo_file << bank <<"\t" << error_type[bank][0] << "\t" <<
            error_type[bank][1] <<"\t" << error_type[bank][2] <<"\t" <<
            error_type[bank][3] << endl;
    }
    cout << endl;
    histo_file << endl;
    histo_file << "Total 0->1 toggles for trrd=4 :" <<toggle01 <<endl;
    histo_file << "Total 1->0 toggles for trrd=4 :" <<toggle10 <<endl;

    fault_recovery:
    if (fpga_fault > 10)
    {
        histo_file << "FAILURE ON FPGA." << endl;
        printf(RED "Abort due to too many failures!!\n" RESET);
    }

    if (cq)
        delete cq;
    return;
}
