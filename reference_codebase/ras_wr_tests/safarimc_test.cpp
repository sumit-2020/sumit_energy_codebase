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

	string rp_result_dir = result_dir + "/" + dimm + "/rp_results" ;
	string wr_result_dir = result_dir + "/" + dimm + "/wr_results" ;
	string ras_result_dir = result_dir + "/" + dimm + "/ras_results" ;

	//system(("mkdir -p " + rp_result_dir).c_str()) ;
	system(("mkdir -p " + wr_result_dir).c_str()) ;
	system(("mkdir -p " + ras_result_dir).c_str()) ;

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
	/* Test tRP */

	ofstream time_log ;
	time_log.open("wr_ras_test"+fmtTime()) ;
	time_log << "Start Time " << endl ;
	time_log << fmtTime() << endl;

	ofstream rp_f, rp_cnt_f;
	ofstream wr_f, wr_cnt_f;
	ofstream ras_f, ras_cnt_f;
	uint8_t pattern0[4] = {0xff, 0x00, 0xcc, 0xaa}; //Dropping pattern 33. similar to cc
	uint8_t pattern1[4] = {0x00, 0xff, 0x55, 0x33}; //Inverted pattern0

	//uint8_t pattern0[1] = {0xaa}; //Dropping pattern 33. similar to cc
	//uint8_t pattern1[1] = {0x33}; //Inverted pattern0

	/*
	// Go through a list of patterns and trp values
	for (uint pid = 0; pid < std::extent<decltype(pattern0)>::value; pid++)
	{
	string fname = "raw_rptest";
	fname += "_patt" + int_to_hex(unsigned(pattern0[pid])) + "_" + int_to_hex(unsigned(pattern1[pid])) +
	"_" + dimm + "_ret" + to_string(wait_time);
	cout << "Pattern 0x" << hex << unsigned(pattern0[pid]) << "_" << hex << unsigned(pattern1[pid]) << dec << endl;
	rp_f.open(rp_result_dir + "/" + fname + "_" +fmtTime());
	rp_f << "Mode wait_time tRCd tRAS tRP tWR"  << endl << endl;

	for(uint trp = 5; trp >= 1; trp--)
	{
	printf("tRP: %u \n", trp);

	string fname_cnt = fname + "_rp" + to_string(trp);
	rp_cnt_f.open(rp_result_dir + "/" + fname_cnt + "_cnt_" + fmtTime());

	test_trp_row_order(fpga,wait_time,trp, rp_f, rp_cnt_f, pattern0[pid], pattern1[pid]) ;
	rp_cnt_f.close();
	}
	rp_f.close();
	}
	usleep(500 * 1000); // takes microseconds
	fpga_reset(fpga); //keep this
	 */
	//Test tRAS




	for (uint pid = 0; pid < std::extent<decltype(pattern0)>::value; pid++)
	{
		string fname = "raw_rastest";
		fname += "_patt" + int_to_hex(unsigned(pattern0[pid])) + "_" + int_to_hex(unsigned(pattern1[pid])) +
			"_" + dimm + "_ret" + to_string(wait_time);
		cout << "Pattern 0x" << hex << unsigned(pattern0[pid]) << "_" << hex << unsigned(pattern1[pid]) << dec << endl;
		ras_f.open(ras_result_dir + "/" + fname + "_" +fmtTime());
		ras_f << "Mode wait_time tRCd tRAS tRP tWR"  << endl << endl;

		for(uint tras = 1; tras <= DEF_TRAS; tras++)
		{
			printf("tRAS: %u \n", tras);

			string fname_cnt = fname + "_ras" + to_string(tras);
			ras_cnt_f.open(ras_result_dir + "/" + fname_cnt + "_cnt_" + fmtTime());

			int test_result ;

			test_result = test_tras_row_order_ret(fpga,wait_time,tras, ras_f, ras_cnt_f, pattern0[pid]) ;
			ras_cnt_f.close();
			if (test_result == 0) 
				break ;
			if (tras > 9) //Coarse grain search initially
				tras ++ ;
		}
		ras_f.close();
	}

	usleep(500 * 1000); // takes microseconds
	fpga_reset(fpga); //keep this
	
	time_log << "RAS END Time " << endl ;
	time_log << fmtTime() << endl;
	/*
	//Test tWR
	for (uint pid = 0; pid < std::extent<decltype(pattern0)>::value; pid++)
	{
		string fname = "raw_wrtest";
		fname += "_patt" + int_to_hex(unsigned(pattern0[pid])) + "_" + int_to_hex(unsigned(pattern1[pid])) +
			"_" + dimm + "_ret" + to_string(wait_time);
		cout << "Pattern 0x" << hex << unsigned(pattern0[pid]) << "_" << hex << unsigned(pattern1[pid]) << dec << endl;
		wr_f.open(wr_result_dir + "/" + fname + "_" +fmtTime());
		wr_f << "Mode wait_time tRCd tRAS tRP tWR"  << endl << endl;

		for(uint twr = 0; twr <= DEF_TWR; twr++)
		{
			printf("tWR: %u \n", twr);

			string fname_cnt = fname + "_wr" + to_string(twr);
			wr_cnt_f.open(wr_result_dir + "/" + fname_cnt + "_cnt_" + fmtTime());

			int test_result ;

			test_result = test_twr_col_order(fpga,wait_time,twr, wr_f, wr_cnt_f, pattern0[pid]) ;
			wr_cnt_f.close();
			if (test_result == 0)
				break ;
		}
		wr_f.close();
	}
	*/
	printf("The test has been completed \n");

	usleep(500 * 1000); // takes microseconds
	
	time_log << "WR END Time " << endl ;
	time_log << fmtTime() << endl;

	fpga_close(fpga);
}
