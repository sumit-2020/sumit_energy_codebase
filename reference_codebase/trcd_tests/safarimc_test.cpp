#include <stdio.h>
#include <stdlib.h>
#include <riffa.h>
#include <cassert>
#include <string>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "timer.h"
#include "utils.h"
#include "test_routine.h"

/* Helper functions */
const string fmtTime(){
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%m%d_%H%M", &tstruct);
    return buf;
}

template <typename T>
std::string int_to_hex(T i)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2)
        << std::hex << i;
    return stream.str();
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "usage: <dimm info string> <wait/retention time> <result folder name>" << endl;
        return -1;
    }
    string dimm = argv[1];
    int wait_time = atoi(argv[2]);
    string result_dir = argv[3];

    fpga_t* fpga;
    fpga_info_list info;
    int fid = 0; //fpga id
    int ch = 0; //channel id

    // Populate the fpga_info_list struct
    if (fpga_list(&info) != 0) {
        printf("Error populating fpga_info_list\n");
        return -1;
    }
    printf("Number of devices: %d\n", info.num_fpgas);
    for (int i = 0; i < info.num_fpgas; i++) {
        printf("%d: id:%d\n", i, info.id[i]);
        printf("%d: num_chnls:%d\n", i, info.num_chnls[i]);
        printf("%d: name:%s\n", i, info.name[i]);
        printf("%d: vendor id:%04X\n", i, info.vendor_id[i]);
        printf("%d: device id:%04X\n", i, info.device_id[i]);
    }

    fpga = fpga_open(fid);

    if(!fpga){
        printf("Problem on opening the fpga \n");
        return -1;
    }
    printf("The FPGA has been opened successfully! \n");

    fpga_reset(fpga); //keep this

    //testRetention(fpga, 128);
    /* Test tRCD */
    ofstream rcd_f, rcd_cnt_f;
    //uint8_t pattern[1] = {0xff};
    uint8_t pattern[4] = {0xff, 0x00, 0xcc, 0xaa}; //Dropping pattern 33. similar to cc

    // Go through a list of patterns and trcd values
    for (uint pid = 0; pid < std::extent<decltype(pattern)>::value; pid++)
    {
        string fname = "raw_rcdtest";
        fname += "_patt" + int_to_hex(unsigned(pattern[pid])) +
            "_" + dimm + "_ret" + to_string(wait_time);
        cout << "Pattern 0x" << hex << unsigned(pattern[pid]) << dec << endl;
        rcd_f.open(result_dir + "/" + fname + "_" +fmtTime());
        rcd_f << "Mode wait_time tRCd tRAS tRP tWR"  << endl << endl;
        //for(uint trcd = 5; trcd >= 1; trcd--)
        for(uint trcd = 3; trcd >= 1; trcd--)
        //for(uint trcd = 2; trcd >= 2; trcd--)
        {
            printf("tRCD: %u \n", trcd);

            string fname_cnt = fname + "_rcd" + to_string(trcd);
            rcd_cnt_f.open(result_dir + "/" + fname_cnt + "_cnt_" + fmtTime());

            test_trcd_col_order(fpga, wait_time, trcd, rcd_f, rcd_cnt_f, pattern[pid]);
            rcd_cnt_f.close();
        }
        rcd_f.close();
    }

    printf("The test has been completed \n");

    usleep(500 * 1000); // takes microseconds

    fpga_close(fpga);
}
