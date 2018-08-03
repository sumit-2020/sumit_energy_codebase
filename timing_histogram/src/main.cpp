/**
 * @brief Create an error histogram for various t_rcd and t_rp values
 * @author Giray Yaglikci <ayaglikc@andrew.cmu.edu>
 **/

#include "timing_histogram.h"

using namespace std;
using namespace std::chrono;

#define FAIL_IN_ARGUMENTS -1 
#define COMPLETE_SUCCESS  0 
#define LOW_TRCD 248
#define LOW_TRP 249 
#define NO_IDEA 250 
#define FPGA_NOT_FOUND 251 
#define FPGA_NOT_ACCESSIBLE 252 

int main(int argc, char **argv) {
    detail_print("This test has been suppressed due to its latency! Using timing_histogram_mem instead\n"); 
    
    if (argc < 8) {
        cout << "$ ./bin (<dimm name> <pattern hex> <trcd cycles> <trp cycles> <test its> <temp> <volt>)" << endl;
        return FAIL_IN_ARGUMENTS;
    }

    fpga_t* fpga;
    #ifndef SANDBOX
    // Begin Setup of FPGA
    fpga_info_list info;
    if (fpga_list(&info) != 0) {
        printf(RED "Error populating fpga_info_list\n" RESET);
        exit(FPGA_NOT_FOUND);
    }
    fpga = fpga_open(0);
    if (!fpga){
        printf(RED "Problem on opening the fpga \n" RESET);
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
    string file_name = "out/" + dimm_name + "_rcd" + to_string(trcd) + "_rp"
        + to_string(trp) + "_ret" + to_string(ms_wait) + "_temp"
        + to_string(temperature) + "_volt" + to_string(voltage) ;
    cout << RESET "File name: " << file_name << endl;

    string hist_file_name = file_name + ".hist";
    string temp_file_name = file_name + ".tmp";
    // File exist check
    struct stat buffer;
    if (stat(hist_file_name.c_str(), &buffer) != 0){	//File does not exist
    	uint file_size = TEST_NUM_BANKS * TEST_NUM_ROWS * TEST_NUM_CACHELINES;
    	int fd = open(hist_file_name.c_str(), O_WRONLY | O_CREAT, 0666);
    	if (ftruncate(fd,file_size) != 0)
    		detail_print("Error occured in file initialization : %s\n" , strerror(errno));
    	fsync(fd);
    	close(fd);
    }

    cp(temp_file_name.c_str(),hist_file_name.c_str());
    /**
     * @brief histogram store 1 byte per cacheline. DIMM has
     * 8 banks, each is holding 32768 (2^15) rows. Therefore,
     * the histogram allocates 4 MB/bank which can fit in the
     * last level cache of the host machine. 
     * 
     * Because of the locality concerns, the iterations of the 
     * experiment moved into the loop of among-banks traversal.
    */
    double average_err = 0.0;
    int num_of_wr_rd_calls = 0;
    for (uint bank_index = 0 ; bank_index < TEST_NUM_BANKS ; bank_index ++){
    	string log_text = "\rBank " + to_string(bank_index) + " latencies (ms) : ";
    	for (uint8_t test_it = 0 ; test_it < total_test_its; test_it++){
    		timespec iteration_timer_start = start_timer();
    		for (uint row_index = 0 ; row_index < TEST_NUM_ROWS ; row_index = row_index + 2 ){
    			average_err += wr_rd_2rows(fpga, bank_index, row_index, patt_base, cq, trcd, trp, ms_wait, temp_file_name);
    			num_of_wr_rd_calls ++;
    		}
    		double iteration_latency = stop_timer(iteration_timer_start, false);
    		log_text += to_string(iteration_latency) + " ";
    		cout << log_text;
    		fflush(stdout);
    	}
    }
    average_err = average_err / num_of_wr_rd_calls;
    detail_print(GREEN "Test Completed with average errors: %lf \n" RESET, average_err);

    if (remove(hist_file_name.c_str()) == -1){
    	detail_print(RED "Old histogram file could not be removed. %s\n" RESET, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (cp(hist_file_name.c_str(),temp_file_name.c_str()) == -1){
    	detail_print(RED "Histogram file could not be updated. %s\n" RESET, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (remove(temp_file_name.c_str()) == -1){
        detail_print(RED "Temporary file could not be removed. %s\n" RESET, strerror(errno));
        exit(EXIT_FAILURE);
    }

    #ifndef SANDBOX
    usleep(500 * 1000);
    fpga_close(fpga);
    #endif

    if (average_err > 0) {
        detail_print(RED "There are errors" RESET);
        return NO_IDEA;
    }
   
    detail_print(GREEN "Passed" RESET);
    return 0;
}