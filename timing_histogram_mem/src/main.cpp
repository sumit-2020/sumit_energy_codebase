/**
 * @brief Create an error histogram for various t_rcd and t_rp values
 * @author Giray Yaglikci <ayaglikc@andrew.cmu.edu>
 **/

#include "timing_histogram_mem.h"
#include "timing_histogram.h"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace std::chrono;

#define FAIL_IN_ARGUMENTS -1
#define COMPLETE_SUCCESS  0
#define LOW_TRCD 248
#define LOW_TRP 249
#define NO_IDEA 250
#define FPGA_NOT_FOUND 251
#define FPGA_NOT_ACCESSIBLE 252

// Global variable to store histogram of error occurance in each beat (out of 8 in a burst) for every cache line
extern vector<uint64_t> zero_err;
extern vector<uint64_t> one_err;
extern vector<uint64_t> two_err;
extern vector<uint64_t> three_err;
extern vector<uint64_t> more_err;

template <typename T>
std::string int_to_hex(T i)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2)
        << std::hex << i;
    return stream.str();
}

int main(int argc, char **argv) {
    if (argc < 8) {
        cout << "$ ./bin (<dimm name> <pattern hex> <trcd cycles> <trp cycles> <test its> <temp> <volt>)" << endl;
        return FAIL_IN_ARGUMENTS;
    }

    fpga_t* fpga;
    #ifndef SANDBOX
    // Begin Setup of FPGA
    fpga_info_list info;
    if (fpga_list(&info) != 0) {
        detail_print(RED "Error populating fpga_info_list\n" RESET);
        exit(FPGA_NOT_FOUND);
    }
    fpga = fpga_open(0);
    if (!fpga){
        detail_print(RED "Problem on opening the fpga \n" RESET);
        exit(FPGA_NOT_ACCESSIBLE);
    }
    fpga_reset(fpga); //keep this
    #endif
    //////
    cout << YELLOW "=====TIMING HISTOGRAM=====" RESET << endl;

    // Worst-case pattern
    const uint8_t patt_base = (uint8_t)strtol(argv[2], NULL, 0);
    uint8_t patt_inv = (uint8_t)~patt_base;
    cout << "Patt & PattInv=" << hex << (unsigned)patt_base <<
        " " << (unsigned)patt_inv << dec << endl;

    // Timings through args
    string dimm_name(argv[1]);
    const uint trcd = (uint8_t)strtol(argv[3], NULL, 0);
    const uint trp = (uint8_t)strtol(argv[4], NULL, 0);
    const uint total_test_its = (uint)strtol(argv[5], NULL, 0);
    const uint temperature = (uint)strtol(argv[6], NULL, 0);
    const float voltage = strtof(argv[7], NULL);

    cout << "RCD & RP=" << trcd << " " << trp << endl;

    // TODO how to handle wait time?
    uint ms_wait = 0;
    CmdQueue* cq = new CmdQueue();

    // Generate the file name
    string file_name = "out_fix/" + dimm_name + "_rcd" + to_string(trcd) + "_rp"
        + to_string(trp) + "_ret" + to_string(ms_wait) + "_temp"
        + to_string(temperature) + "_volt" + to_string(voltage) + "_fix.hist";
    cout << RESET "File name: " << file_name << endl;
    dimm_hist hist =  initialize_histogram(file_name);

    // Histogram file on per beat error occurance
    ofstream beat_histo;
    string beat_histo_fname = "out_fix/" + dimm_name + "_rcd" + to_string(trcd) + "_rp"
        + to_string(trp) + "_ret" + to_string(ms_wait) + "_temp"
        + to_string(temperature) + "_volt" + to_string(voltage)
        + "_patt0x" + int_to_hex(unsigned(patt_base)) + "_" +fmtTime() + "_fix.csv";
    cout << "Beat histo file name: " << beat_histo_fname << endl;
    beat_histo.open(beat_histo_fname);

    /**
     * @brief histogram store 1 byte per cacheline. DIMM has
     * 8 banks, each is holding 32768 (2^15) rows. Therefore,
     * the histogram allocates 4 MB/bank which can fit in the
     * last level cache of the host machine.
     *
     * Because of the locality concerns, the iterations of the
     * experiment moved into the loop of among-banks traversal.
    */
    uint bank_index = 0;
    uint total_err = 0;
    for ( dimm_hist::iterator bank_iter = hist.begin() ; bank_iter != hist.end() ; bank_iter++ ){
        string log_text = "\rBank " + to_string(bank_index) + " latencies (ms) : ";
        for (uint8_t test_it = 0 ; test_it < total_test_its; test_it++){
            timespec iteration_timer_start = start_timer();
            uint row_index = 0;
            for ( bank_hist::iterator row_iter = (*bank_iter).begin() ; row_iter != (*bank_iter).end() ; row_iter++ ){
                // we need another iterator for the next row
                bank_hist::iterator next_row_iter = row_iter;
                ++next_row_iter;

                // sending and receiving a pair of rows. Unfortunately C needs this kind of dummy wrapping
                std::pair <row_hist, row_hist> row_hists;
                row_hists.first  = *row_iter;
                row_hists.second = *next_row_iter;
                //row_hists = wr_rd_2rows(fpga, row_hists, row_index, bank_index, patt_base, cq, trcd, trp, ms_wait);
                row_hists = wr_rd_2rows_fix(fpga, row_hists, row_index, bank_index, patt_base, cq, trcd, trp, ms_wait);
                *row_iter       = row_hists.first;
                *next_row_iter  = row_hists.second;
                row_iter++;
                row_index +=2;

                // sum up # of total errors
                total_err = std::accumulate(row_hists.first.begin(), row_hists.first.end(), total_err);
                total_err = std::accumulate(row_hists.second.begin(), row_hists.second.end(), total_err);
            }
            double iteration_latency = stop_timer(iteration_timer_start, false);
            log_text += to_string(iteration_latency) + " ";
            cout << log_text;
            fflush(stdout);
        }
        bank_index ++;
    }

    // Record histo iteration count
    log_iteration_count("voltage_scaling_error_histo.log", file_name, total_test_its);
    dump_to_file(hist, file_name);
    dump_beat_histo(beat_histo);
    beat_histo.close();
    delete cq;
    cout << RESET "Test Completed!" << endl;

    #ifndef SANDBOX
    usleep(500 * 1000);
    fpga_close(fpga);
    #endif

    #ifdef DEBUG
    for (int i = 0; i < 8; i++) {
        cout << "Zero err count in Beat " << i << "=" << zero_err[i] << endl;
        cout << "1 err count in Beat " << i << "=" << one_err[i] << endl;
        cout << "2 err count in Beat " << i << "=" << two_err[i] << endl;
        cout << "3 err count in Beat " << i << "=" << three_err[i] << endl;
        cout << ">3 err count in Beat " << i << "=" << more_err[i] << endl;
    }
    #endif

    if (total_err > 0)
    {
        double average_err = total_err / (TEST_NUM_ROWS * TEST_NUM_BANKS);
        detail_print(RED "Test finished: There are %lf errors per row on avg!\n" RESET, average_err);
        return NO_IDEA;
    }

    detail_print(GREEN "Passed" RESET);
    return 0;
}