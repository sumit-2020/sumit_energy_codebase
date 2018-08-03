/**
 * @brief Simultaneously tuning trcd and trp
 * @author Kevin Chang <kevincha@cmu.edu>
 **/

#include "test_routine.h"
#include <chrono>

// Test params
#define TEST_NUM_BANKS 8
//#define TEST_NUM_BANKS 1
#define TEST_NUM_ROWS 32736
//#define TEST_NUM_ROWS 4

// DIMM parameters
#define COL_GRAN 8
#define NUM_COL_ROW 1024

using namespace std;
using namespace std::chrono;

int main(int argc, char **argv) {
    if (argc < 8) {
        cout << "$ ./bin (<dimm name> <pattern hex> <trcd cycles> <trp cycles> <test its> <temp> <volt>)" << endl;
        return 0;
    }

    // Begin Setup of FPGA
    fpga_info_list info;
    if (fpga_list(&info) != 0) {
        printf(RED "Error populating fpga_info_list\n" RESET);
        exit(EXIT_FAILURE);
    }
    fpga_t* fpga = fpga_open(0);
    if (!fpga){
        printf(RED "Problem on opening the fpga \n" RESET);
        exit(EXIT_FAILURE);
    }
    fpga_reset(fpga); //keep this
    //////
    cout << YELLOW "=====COMBINED TIMING TEST=====" RESET << endl;

    // Worst-case pattern
    const uint8_t patt_base = (uint8_t)strtol(argv[2], NULL, 0);
    uint8_t patt_inv = (uint8_t)~patt_base;
    cout << "Patt & PattInv=" << hex << (unsigned)patt_base <<
        " " << (unsigned)patt_inv << dec << endl;

    // Timings through args
    string dimm_name(argv[1]);
    const uint arg_trcd = (uint)strtol(argv[3], NULL, 0);
    const uint arg_trp = (uint)strtol(argv[4], NULL, 0);
    const uint total_test_its = (uint)strtol(argv[5], NULL, 0);
    const uint temperature = (uint)strtol(argv[6], NULL, 0);
    const float voltage = strtof(argv[7], NULL);
    cout << "RCD & RP=" << arg_trcd << " " << arg_trp << endl;

    // TODO how to handle wait time?
    uint ms_wait = 0;
    CmdQueue* cq = nullptr;

    // CSV file
    string file_name = "StreamTimingTest_" + dimm_name;
    file_name += "_rcd" + to_string(arg_trcd) + "_rp" + to_string(arg_trp)
        + "_ret" + to_string(ms_wait) + "_temp" + to_string(temperature)
        + "_volt" + to_string(voltage) + "_" + fmtTime() + ".csv";
    cout << "FILE NAME: " << file_name << endl;
    csv::ofstream of_csv(file_name.c_str());
    of_csv.enable_surround_quote_on_str(true, '\"');
    writeHeaderCSV(of_csv);

    // Each iteration takes about 4.6hrs going through all banks & rows for a retention of 64ms
    // 12 mins for a retention of 0ms
    int ret = 0;
    for (int test_it = 0; test_it < total_test_its; test_it++) {
        cout << BLUE "Iteration:" << test_it << RESET << endl;
        for (uint bank = 0; bank < TEST_NUM_BANKS; bank++) {
            cout << "\rBank " << bank;
            fflush(stdout);
            for (uint r = 0; (r+1) < TEST_NUM_ROWS; r++) {
                //high_resolution_clock::time_point t1 = high_resolution_clock::now();
                ret = writeAndReadRow(fpga, test_it, r, r+1, bank, patt_base, cq,
                        arg_trcd, arg_trp, ms_wait, of_csv);
                if (ret == -1)
                    goto exit;
                //high_resolution_clock::time_point t2 = high_resolution_clock::now();
                //auto duration = duration_cast<microseconds>( t2 - t1 ).count();
                // 65ms for wait of 64
                // 3ms for wait of 0
                //cout << "Duration " << duration << endl;
            }
        }
        cout << endl;
    }

exit:
    of_csv.flush();
    of_csv.close();

    // DONE
    usleep(500 * 1000);
    fpga_close(fpga);
}
