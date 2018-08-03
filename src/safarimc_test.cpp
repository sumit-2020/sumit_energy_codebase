/**
 * Main function used for the SIGMETRICS data collection
 *
 * @author Kevin Chang <kevincha@cmu.edu>
 * @author Hasan Hassan <hasanibrahimhasan@gmail.com>
 * @author Abhijith Kashyap <akashyap@andrew.cmu.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <riffa.h>
#include <cassert>
#include <string>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <vector>
#include <getopt.h>

#include "timer.h"
#include "utils.h"
#include "test_routine.h"
#include "parse_cli.h"

// All the command line arguments
bool    count_enable = false ;
int     num_rows = NUM_ROWS ;
int     num_cols = NUM_COLS ;
int     wait_time = 0;
string  dimm ;
string  vdd = "1.5" ;
string  temperature = "20c" ;
string  result_dir = "results" ;
string  test_name ;
int     test_num ;
int     end_delay = 1 ;
int     start_delay ;
bool    start_delay_flag = false ;
string  pattern_string = "0xff 0x00 0xcc 0xaa" ;
vector<uint8_t> pattern ;
bool bypass_verif = false;
//End cli arguments

template <typename T>
std::string int_to_hex(T i)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2)
        << std::hex << i;
    return stream.str();
}

void print_header(ostream& file)
{

    file << "Command line :" << endl ;
    file << "********************************************" << endl ;
    file << "Supply Voltage : \t" << vdd << endl ;
    file << "Temperature : \t\t" << temperature << endl ;
    file << "Count file :\t\t" << (count_enable?"enabled":"disabled") <<endl ;
    file << "Number of rows :\t" << num_rows << endl ;
    file << "Number of columns :\t" << num_cols << endl ;
    file << "Test :\t\t\t" << test_name << " (" << test_num << ")" << endl ;
    file << "DIMM information :\t" << dimm << endl ;
    file << "Wait time :\t\t" << wait_time << endl ;
    file << "Start Delay :\t\t" << start_delay << endl;
    file << "End Delay :\t\t" << end_delay << endl ;
    file << "Output folder :\t\t" << result_dir << endl ;
    file << "Bypass verif : \t\t" << bypass_verif << endl;
    file << "Pattern : \t\t" ;

    for (auto i = pattern.begin(); i != pattern.end(); ++i)
        file << hex << unsigned(*i) << dec << " " ;

    file << endl ;
    file << "********************************************" << endl ;
}

int main(int argc, char **argv) {
    //Parsing the arguments
    int     req_args = 2 ; //test and dimm
    int     iargs = 0 ;
    int     index ; //Used for getopt
    while (iargs != -1) {
        iargs = getopt_long(argc, argv, "hcbv:r:l:t:d:w:p:s:e:o:m:",
                longopts, &index);
        switch (iargs) {
            case 'h':
                print_usage() ;
                break ;
            case 'v':
                vdd = optarg ;
                break ;
            case 'c':
                count_enable = true ;
                break ;
            case 'b':
                bypass_verif = true;
                break;
            case 'r':
                num_rows = atoi(optarg) ;
                break ;
            case 'l':
                num_cols = atoi(optarg) ;
                break ;
            case 't':
                test_name = optarg ;
                req_args-- ;
                break ;
            case 'd':
                dimm = optarg ;
                req_args-- ;
                break ;
            case 'w':
                wait_time = atoi(optarg) ;
                break ;
            case 'p':
                pattern_string = optarg ;
                break ;
            case 's':
                start_delay = atoi(optarg) ;
                start_delay_flag = true ;
                break ;
            case 'e':
                end_delay = atoi(optarg) ;
                break ;
            case 'o':
                result_dir = optarg ;
                break ;
            case 'm':
                temperature = optarg ;
                break ;
        }
    }
    if (req_args) {
        cout << "Mandatory Arguments missing!!" << endl ;
        print_usage();
        exit(EXIT_FAILURE) ;
    }
    test_num = parse_test_name(test_name) ;
    parse_pattern(&pattern_string, &pattern) ;
    if (!start_delay_flag)
        start_delay = parse_start_delay(test_num) ;
    if (-1 == test_num) {
        cout << "Unrecognized test" << endl  ;
        print_usage() ;
        exit(EXIT_FAILURE) ;
    }
    print_header(cout) ;
    //End Parsing arguments

    //Begin Setup of FPGA
    fpga_info_list info;
    // Populate the fpga_info_list struct
    if (fpga_list(&info) != 0) {
        printf("Error populating fpga_info_list\n");
        exit(EXIT_FAILURE);
    }
    printf("Number of devices: %d\n", info.num_fpgas);
    for (int i = 0; i < info.num_fpgas; i++) {
        printf("%d: id:%d\n", i, info.id[i]);
        printf("%d: num_chnls:%d\n", i, info.num_chnls[i]);
        printf("%d: name:%s\n", i, info.name[i]);
        printf("%d: vendor id:%04X\n", i, info.vendor_id[i]);
        printf("%d: device id:%04X\n", i, info.device_id[i]);
    }
    fpga_t* fpga = fpga_open(0);
    if(!fpga){
        printf("Problem on opening the fpga \n");
        exit(EXIT_FAILURE);
    }
    printf("The FPGA has been opened successfully! \n");
    fpga_reset(fpga); //keep this

    //Create the result directory
    result_dir += '/' + test_name + '/' + dimm + '/' ;
    if (system(("mkdir -p " + result_dir).c_str()) == -1) {
        cout << "Failed to mkdir" << endl;
        exit(-1);
    }

    //testRetention(fpga, 128);
    // Test tRCD
    ofstream summary_file;
    ofstream count_file;

    // Go through a list of patterns and trcd values
    for (auto patt_it = pattern.begin(); patt_it != pattern.end(); ++patt_it) {
        uint8_t curr_patt = *patt_it ;
        string fname = test_name+ "_" + vdd + "_" + temperature ;
        fname += "_patt" + int_to_hex(unsigned(curr_patt)) +
            "_" + dimm + "_ret" + to_string(wait_time);
        cout << "Pattern 0x" << hex << unsigned(curr_patt) << dec << endl;
        summary_file.open(result_dir + "/" + fname + "_" +fmtTime());
        print_header(summary_file) ;
        summary_file << "Mode wait_time tRCd tRAS tRP tWR"  << endl << endl;

        /* Using count up for tras and twr */
        for(int tparam = end_delay; tparam <= start_delay; tparam++)
        {
            bool ce = true; //log only for certain tparam values

            // If tparam is one cycle, don't bother logging errors.
            if ((test_num == TRCD_ENUM || test_num == TRP_ENUM) && tparam == 1)
                ce = false;

            string fname_cnt = fname + "_"+ test_name + to_string(tparam);
            if (count_enable && ce)
            {
              count_file.open(result_dir + "/" + fname_cnt + "_cnt_"
                + fmtTime()) ;
            }
            int test_return;

            // Time when the test starts
            cout << fmtTime() << endl;
            switch(test_num)
            {
                case TRAS_ENUM:
                    if (tparam >= DEF_TRAS)
                        bypass_verif = true;
                    cout << "TRAS : " << tparam << endl ;
                    test_return = test_tras_row_order(fpga, wait_time,
                            tparam, summary_file, count_file, curr_patt,
                            count_enable, num_rows, num_cols);
                    break;

                case TWR_ENUM:
                    if (tparam >= DEF_TWR)
                        bypass_verif = true;
                    cout << "TWR : " << tparam << endl ;
                    test_return = test_twr_col_order(fpga, wait_time, tparam,
                            summary_file, count_file, curr_patt,
                            count_enable, num_rows, num_cols);
                    break;

                case TRCD_ENUM:
                    //if (tparam >= DEF_TRCD)
                    // Don't worry about re-reading after each write when below
                    // the nominal voltage
                    if (strtof(vdd.c_str(), NULL) < 1.5)
                        bypass_verif = true;
                    cout << "TRCD : " << tparam << endl ;
                    test_return = test_trcd_col_order(fpga, wait_time, tparam,
                            summary_file, count_file, curr_patt,
                            ce, num_rows, num_cols, bypass_verif);
                    break;

                case TRP_ENUM:
                    //if (tparam >= DEF_TRP)
                    // Don't worry about re-reading after each write when below
                    // the nominal voltage
                    if (strtof(vdd.c_str(), NULL) < 1.5)
                        bypass_verif = true;
                    cout << "TRP : " << tparam << endl ;
                    test_return = test_trp_row_order(fpga, wait_time, tparam,
                            summary_file, count_file, curr_patt,
                            ce, num_rows, num_cols, bypass_verif);
                    break;

                case TRRD_ENUM:
                    cout << "Running TRRD test with default TRRD=4" << endl;
                    test_trrd4ck_col_order(fpga, wait_time, summary_file,
                            count_file, curr_patt, count_enable, num_rows,
                            num_cols);
                    break;

                default:
                    cout << "Unrecognized test. Exiting";
            }

            //Close the file if opened
            if (count_enable && ce)
                count_file.close();

            /* if there are errors test_return will be -1
               No need to test at more relaxed test params after getting
               run with no errors.
               */
            if (test_return == 0)
                break;
        }
        summary_file.close() ; //Close after pattern is done
    }
    printf("The test has been completed \n");
    usleep(500 * 1000); // takes microseconds
    fpga_close(fpga);
}
