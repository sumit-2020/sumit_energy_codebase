#include <stdio.h>
#include <riffa.h>
#include "timer.h"
#include <cassert>
#include <string.h>
#include <iostream>
#include <cmath>

#include "utils.h"

using namespace std;

//Note that capacity of the command buffer is 1024 commands
void writeRow(fpga_t* fpga, uint row, uint bank, uint8_t pattern, CmdQueue*& cq){
	int ch = 0; //riffa channel should always be 0

	if(cq == nullptr)
		cq = new CmdQueue();
	else
		cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation on each call

	//Precharge target bank (just in case if its left activated)
	cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

	//Wait for tRP
	cq->insert(genWaitCMD(5));//2.5ns have already been passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

	//Activate target row
	cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

	//Wait for tRCD
	cq->insert(genWaitCMD(5));

	//Write to the entire row
	for(int i = 0; i < NUM_COLS; i+=8){ //we use 8x burst mode
		cq->insert(genWriteCMD(i, bank, pattern));

		//We need to wait for tCL and 4 cycles burst (double data-rate)
		cq->insert(genWaitCMD(6 + 4));
	}

	//Wait some more in any case
	cq->insert(genWaitCMD(3));

	//Precharge target bank
	cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

	//Wait for tRP
	cq->insert(genWaitCMD(5));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

	//START Transaction
	cq->insert(genStartTR());

	fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

void readAndCompareRow(fpga_t* fpga, const uint row, const uint bank, const uint8_t pattern, CmdQueue*& cq ){
	int ch = 0; //riffa channel should always be 0

	if(cq == nullptr)
		cq = new CmdQueue();
	else
		cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

	//Precharge target bank (just in case if its left activated)
	cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

	//Wait for tRP
	cq->insert(genWaitCMD(5));//2.5ns have already been passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns


	//Activate target row
	cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

	//Wait for tRCD
	cq->insert(genWaitCMD(5));

	//Read the entire row
	for(int i = 0; i < NUM_COLS; i+=8){ //we use 8x burst mode
		cq->insert(genReadCMD(i, bank));

		//We need to wait for tCL and 4 cycles burst (double data-rate)
		cq->insert(genWaitCMD(6 + 4));
	}

	//Wait some more in any case
	cq->insert(genWaitCMD(3));

	//Precharge target bank
	cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

	//Wait for tRP
	cq->insert(genWaitCMD(5));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

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
			if(rbuf8[j] != pattern)
				fprintf(stderr, "Error at Col: %d, Row: %u, Bank: %u, DATA: %x \n", i, row, bank, rbuf8[j]);
		}
	}
}

void turnBus(fpga_t* fpga, BUSDIR b, CmdQueue* cq = nullptr){
	int ch = 0; //riffa channel should always be 0

	if(cq == nullptr)
		cq = new CmdQueue();
	else
		cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

	cq->insert(genBusDirCMD(b));

	//WAIT
	cq->insert(genWaitCMD(5));

	//START Transaction
	cq->insert(genStartTR());

	fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

//Observe error at provided retention time
//retention in milliseconds
void testRetention(fpga_t* fpga, const int retention){

	uint8_t pattern = 0xff;
	bool dimm_done = false;

	CmdQueue* cq = nullptr;

	GET_TIME_INIT(2);

	//writing the entire row takes approximately 5 ms
	uint group_size = ceil(retention/5.0f); //number of rows to be written in a single iteration
	uint cur_row_write = 0;
	uint cur_bank_write = 0;
	uint cur_row_read = 0;
	uint cur_bank_read = 0;

    printf("\n");

	while(!dimm_done){
	    //printf("%c[2K\r", 27);
        printf("Current Bank: %d, Row: %d\n", cur_bank_write, cur_row_write);
        //fflush(stdout);

		turnBus(fpga, BUSDIR::WRITE, cq);
		GET_TIME_VAL(0);
		for(int i = 0; i < group_size; i++){
			writeRow(fpga, cur_row_write, cur_bank_write, pattern, cq);
			cur_row_write++;

			if(cur_row_write == NUM_ROWS){ //NUM_ROWS
				cur_row_write = 0;
				cur_bank_write++;
			}

			if(cur_bank_write == NUM_BANKS){ //NUM_BANKS
				//we are done with the entire DIMM
				dimm_done = true;
				break;
			}
		}

		turnBus(fpga, BUSDIR::READ, cq);
		do{
			GET_TIME_VAL(1);
		}while((TIME_VAL_TO_MS(1) - TIME_VAL_TO_MS(0)) < retention);

		//read back and compare
		for(int i = 0; i < group_size; i++){
			readAndCompareRow(fpga, cur_row_read, cur_bank_read, pattern, cq);

			cur_row_read++;

			if(cur_row_read == NUM_ROWS){ //NUM_ROWS
				cur_row_read = 0;
				cur_bank_read++;
			}

			if(cur_bank_read == NUM_BANKS){ //NUM_BANKS
				//we are done with the entire DIMM
				break;
			}
		}
	}

    printf("\n");
}


void printHelp(){
    cout << "A sample application that tests retention time of DRAM cells using SafariMC" << endl;
	cout << "Usage: ./ret_test [REFRESH INTERVAL]" << endl;
	cout << "The Refresh Interval should be positive integer" << endl;
}

int main(int argc, char* argv[]){
	fpga_t* fpga;
	fpga_info_list info;
	int fid = 0; //fpga id
	int ch = 0; //channel id

	if(argc != 2 || strcmp(argv[1], "--help") == 0){
		printHelp();
		return -2;
	}

    string s_ref(argv[1]);
    int refresh_interval = 0;

    try{
        refresh_interval = stoi(s_ref);
    }catch(...){
        printHelp();
        return -3;
    }

    if(refresh_interval <= 0){
        printHelp();
        return -4;
    }

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

	fpga_reset(fpga); //keep this, recovers FPGA from some unwanted state

    printf("Starting Retention Time Test @ %d ms! \n", refresh_interval);
	testRetention(fpga, refresh_interval);

	printf("The test has been completed! \n");
	fpga_close(fpga);

	return 0;
}
