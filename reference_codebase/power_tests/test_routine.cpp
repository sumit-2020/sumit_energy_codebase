/**
 * Simpl test routines on various timing parameters
 */

#include "test_routine.h"


// Color code
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define RESET "\x1b[0m"

uint errors = 0;
uint prev_errors = 0;
uint bytes_read = 0;


Cmd genRowCMD(uint row, uint bank, MC_CMD c)
{

    //TODO: check if c is a row command
    Cmd cmd = bank;

    cmd <<= ROW_OFFSET;
    cmd |= row;
    cmd <<= CMD_OFFSET;

    cmd|= (uint)c;

    return cmd;
}

Cmd genWriteCMD(uint col, uint bank, uint8_t pattern)
{
    Cmd cmd = pattern >> 5; //most significant 3 bits of the pattern
    cmd <<= 3;

    cmd |= bank;
    cmd <<= 5;

    cmd |= pattern & 0x1F; //least significant 5 bits
    cmd <<= (ROW_OFFSET - 5);

    cmd |= col;
    cmd <<= CMD_OFFSET;
    cmd|= (uint)MC_CMD::WRITE;

    return cmd;
}

Cmd genReadCMD(uint col, uint bank)
{
    Cmd cmd = bank;
    cmd <<= ROW_OFFSET;
    cmd |= col;
    cmd <<= CMD_OFFSET;
    cmd|= (uint)MC_CMD::READ;

    return cmd;
}

Cmd genWaitCMD(uint cycles_to_wait)
{
    //min 1, max 1023
    assert(cycles_to_wait >= 1);
    Cmd cmd = cycles_to_wait;
    cmd <<= CMD_OFFSET;
    cmd |= (uint)MC_CMD::WAIT;

    return cmd;
}

Cmd genBusDirCMD(BUSDIR dir)
{
    Cmd cmd = (uint)dir;
    cmd <<= CMD_OFFSET;
    cmd |= (uint)MC_CMD::SET_BUS_DIR;

    return cmd;
}

Cmd genStartTR()
{
    return (Cmd)MC_CMD::SEND_TR;
}

Cmd genZQCMD()
{
    return (Cmd)MC_CMD::ZQ;
}


void openAndCloseRow(fpga_t* fpga, const uint row, const uint bank, const uint delay_tras, const uint delay_trp, CmdQueue*& cq)
{
    int ch = 0;

    if(cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

    for(int i = 0; i < 1; i++){
        //Activate target row
        cq->insert(genRowCMD((row + i*10)%NUM_ROWS, bank, MC_CMD::ACT));

        //Wait for tRAS, skipping (delay = 0) means 2.5ns
        if(delay_tras > 0){
            cq->insert(genWaitCMD(delay_tras));
        }

        //Precharge target bank
        cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

        //Wait for tRP
        if(delay_trp > 0){
            cq->insert(genWaitCMD(5));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns
        }
    }

    //START Transaction
    cq->insert(genStartTR());

    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

void writeColBuffered(fpga_t* fpga, const uint col, const uint row, 
                      const uint bank, const uint8_t pattern, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    //else
    //    cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

    turnBus(fpga, BUSDIR::WRITE, cq);

    //Precharge target bank (just in case if its left activated)
    //cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

    //Wait for tRP
    //cq->insert(genWaitCMD(5));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

    //Activate target row
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

    //Wait for tRCD
    cq->insert(genWaitCMD(5));

    cq->insert(genWriteCMD(col, bank, pattern));

    //We need to wait for tCL and 4 cycles burst (double data-rate)
    cq->insert(genWaitCMD(6 + 4));

    //Wait some more in any case
    cq->insert(genWaitCMD(3));

    //Precharge target bank
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 -> precharge given bank

    //Wait for tRP
    cq->insert(genWaitCMD(5));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

    //START Transaction
    //cq->insert(genStartTR());

    //fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

void writeCol(fpga_t* fpga, const uint col, const uint row, const uint bank, 
              const uint8_t pattern, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

    turnBus(fpga, BUSDIR::WRITE, cq);

    //Precharge target bank (just in case if its left activated)
    //cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

    //Wait for tRP
    //cq->insert(genWaitCMD(5));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

    //Activate target row
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

    //Wait for tRCD
    cq->insert(genWaitCMD(5));

    cq->insert(genWriteCMD(col, bank, pattern));

    //We need to wait for tCL and 4 cycles burst (double data-rate)
    cq->insert(genWaitCMD(6 + 4));

    //Wait some more in any case
    cq->insert(genWaitCMD(3));

    //Precharge target bank
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 -> precharge given bank

    //Wait for tRP
    cq->insert(genWaitCMD(5));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

    //START Transaction
    cq->insert(genStartTR());

    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

//Note that capacity of the command buffer is 256 commands
void writeRow(fpga_t* fpga, uint row, uint bank, uint8_t pattern, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

    turnBus(fpga, BUSDIR::WRITE, cq);
    //Precharge target bank (just in case if its left activated)
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

    //Wait for tRP
    cq->insert(genWaitCMD(5));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

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
// @return number of bit errors in the column
int readAndCompareColCount(fpga_t* fpga, const uint col, const uint row, const uint bank, const uint8_t pattern, const uint trcd, CmdQueue*& cq,
        unsigned burst_errors[])
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

    turnBus(fpga, BUSDIR::READ, cq);
    
    //Activate target row
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

    // Additional wait for tRCD if it's at least 2 cycles
    // (-1 due to the elapse time of the cmd issue cycle)
    if (trcd > 1)
        cq->insert(genWaitCMD(trcd - 1));

    cq->insert(genReadCMD(col, bank));

    // Wait for RAS before precharge
    cq->insert(genWaitCMD(DEF_TRAS));

    //Precharge target bank
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

    //Wait for tRP
    cq->insert(genWaitCMD(DEF_TRP));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

    //START Transaction
    cq->insert(genStartTR());

    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);

    //Receive the data
    uint rbuf[16];
    //fpga_recv(fpga, ch, (void*)rbuf, 16, 0);
		/*edit start*/
		int num_recv = 0 ;
    num_recv = fpga_recv(fpga, ch, (void*)rbuf, 16, 1000);
		if (num_recv != 16)
			return -1 ;
    uint8_t* rbuf8 = (uint8_t *) rbuf;
    int err_cnt = 0;
//    for(int j = 0; j < 64; j++) {
//        uint8_t diff = rbuf8[j] ^ pattern;
//        for (int ci = 0; ci < 8; ci++) {
//            err_cnt += (diff & 1);
//            diff >>= 1;
//        }
//    }

    for(int j = 0; j < 8; j++) {
        int burst_error_cnt = 0 ;
        for (int k = 0; k < 8; k++) {
            uint8_t diff = rbuf8[j] ^ pattern;
            for (int ci = 0; ci < 8; ci++) {
                err_cnt += (diff & 1);
                burst_error_cnt += (diff & 1);
                diff >>= 1;
            }
        }
        burst_errors[j] = burst_error_cnt ;
    }
    // A cache line is only 64 bytes (512 bits)
    assert(err_cnt <= 512);
    return err_cnt;
}

void readColBuffered(fpga_t* fpga, const uint col, const uint row, const uint bank, 
                     const uint8_t pattern, const uint trcd, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    //else
    //    cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

    turnBus(fpga, BUSDIR::READ, cq);
    
    //Activate target row
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

    // Additional wait for tRCD if it's at least 2 cycles
    // (-1 due to the elapse time of the cmd issue cycle)
    if (trcd > 1)
        cq->insert(genWaitCMD(trcd - 1));

    cq->insert(genReadCMD(col, bank));

    // Wait for RAS before precharge
    cq->insert(genWaitCMD(DEF_TRAS));

    //Precharge target bank
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

    //Wait for tRP
    cq->insert(genWaitCMD(DEF_TRP));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

    //START Transaction
    //cq->insert(genStartTR());

    //fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
    
    
    //Receive the data
    //uint rbuf[16];
    //fpga_recv(fpga, ch, (void*)rbuf, 16, 0);

    
    //uint8_t* rbuf8 = (uint8_t *) rbuf;

    /*for(int j = 0; j < 64; j++) {
      if(rbuf8[j] != pattern) {
	printf("Error at Byte: %d, Col: %d, Row: %u, Bank: %u, DATA: %x Correct Data: %x\n",
	       j, col, row, bank, rbuf8[j], pattern);
      }
    }*/
}

void readCol(fpga_t* fpga, const uint col, const uint row, const uint bank, const uint8_t pattern, const uint trcd, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

    turnBus(fpga, BUSDIR::READ, cq);

    //Activate target row
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

    // Additional wait for tRCD if it's at least 2 cycles
    // (-1 due to the elapse time of the cmd issue cycle)
    if (trcd > 1)
        cq->insert(genWaitCMD(trcd - 1));

    cq->insert(genReadCMD(col, bank));

    // Wait for RAS before precharge
    cq->insert(genWaitCMD(DEF_TRAS));

    //Precharge target bank
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

    //Wait for tRP
    cq->insert(genWaitCMD(DEF_TRP));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

    //START Transaction
    cq->insert(genStartTR());

    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
    //Receive the data
    uint rbuf[16];
    fpga_recv(fpga, ch, (void*)rbuf, 16, 0);
}

void readAndCompareCol(fpga_t* fpga, const uint col, const uint row, const uint bank, const uint8_t pattern, const uint trcd, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

    // TURN THE GODDAMN BUS
    turnBus(fpga, BUSDIR::READ, cq);

    //Activate target row
    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));

    // Additional wait for tRCD if it's at least 2 cycles
    // (-1 due to the elapse time of the cmd issue cycle)
    if (trcd > 1)
        cq->insert(genWaitCMD(trcd - 1));

    cq->insert(genReadCMD(col, bank));

    // Wait for RAS before precharge
    cq->insert(genWaitCMD(DEF_TRAS));

    //Precharge target bank
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

    //Wait for tRP
    cq->insert(genWaitCMD(DEF_TRP));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

    //START Transaction
    cq->insert(genStartTR());

    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
    
    //Receive the data
    uint rbuf[16];
    fpga_recv(fpga, ch, (void*)rbuf, 16, 0);

    uint8_t* rbuf8 = (uint8_t *) rbuf;

    printf("Entering check...");
    for(int j = 0; j < 64; j++) {
        if(rbuf8[j] != pattern) {
            printf("Error at Byte: %d, Col: %d, Row: %u, Bank: %u, DATA: %x Correct Data: %x\n",
                    j, col, row, bank, rbuf8[j], pattern);
        }
    }
    printf("Exiting.\n");
}

void openRowRead(fpga_t* fpga,  const uint row, const uint bank, 
                 const uint8_t pattern, const uint trcd, CmdQueue*& cq)
{
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

void readAndCompareRow(fpga_t* fpga, const uint row, const uint bank, const uint8_t pattern, CmdQueue*& cq )
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call
    
    turnBus(fpga, BUSDIR::READ, cq);

    //Precharge target bank (just in case if its left activated)
    cq->insert(genRowCMD(0, bank, MC_CMD::PRE)); //row 1 -> precharge all, row 0 precharge bank

    //Wait for tRP
    cq->insert(genWaitCMD(5));//we have already 2.5ns passed as we issue in next cycle. So, 5 means 6 cycles latency, 15 ns

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
    //printf("ROW: %d\n", row);
    //printf("BANKS: %d\n", bank);
    for(int i = 0; i < NUM_COLS; i+=8){ //we receive a single burst at two times (32 bytes each)
      fpga_recv(fpga, ch, (void*)rbuf, 16, 0);

        //compare with the pattern
      uint8_t* rbuf8 = (uint8_t *) rbuf;
      
      for(int j = 0; j < 64; j++){

	bytes_read +=1;
	
	if(rbuf8[j] != pattern) {
	  printf("Error at Col: %d, Row: %u, Bank: %u, DATA: %x \n", i, row, bank, rbuf8[j]);
	  errors+=1;
	}
      }
    }
    
    if (row == NUM_ROWS-1) {
      printf("Bytes read = %u\n", bytes_read);
      printf("Errors = %d\n", errors);
    }

}

void receive(fpga_t *fpga, int ch, uint pattern, uint row, uint bank, 
             uint numRws, uint num_cols)
{
    uint rbuf[16];


    for (int i = 0; i < num_cols ; i += BURST_LEN){ 
        fpga_recv(fpga, ch, (void*)rbuf, 16, 0);

        uint8_t* rbuf8 = (uint8_t *) rbuf;

	 for (int j = 0; j < 64; j++){

	   bytes_read +=1;

	   //if(rbuf8[j] != pattern) {
	     //printf("Error at NumRws: %d Col: %d, Row: %u, Bank: %u, DATA: %x \n", 
	     //    numRws, i, row, bank, rbuf8[j]);
	     //errors += 1;
	   //}
	 }
    }
    
    if (row == NUM_ROWS-1) {
      printf("Bytes read = %u\n", bytes_read); 
      printf("Errors = %d\n", errors);
    }

}

// Precharge one given bank
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

//Dont need to use anymore, the HW handles zq calibration automatically
void calibZQ(fpga_t* fpga, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0

    if(cq == nullptr)
        cq = new CmdQueue();
    else
        cq->size = 0;//reuse the provided CmdQueue to avoid dynamic allocation for each call

    cq->insert(genZQCMD());

    //WAIT
    cq->insert(genWaitCMD(64)); //at least 64 cycles we should wait

    //START Transaction
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

void turnBus(fpga_t* fpga, BUSDIR b, CmdQueue* cq = nullptr)
{
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
void testRetention(fpga_t* fpga, const int retention)
{
    uint8_t pattern = 0xff;
    bool dimm_done = false;

    CmdQueue* cq = nullptr;

    GET_TIME_INIT(2);

    //writing the entire row takes approximately 5 ms
    uint group_size = retention/5; //number of rows to be written in a single iteration
    uint cur_row_write = 0;
    uint cur_bank_write = 0;
    uint cur_row_read = 0;
    uint cur_bank_read = 0;

    while(!dimm_done){
        //calibZQ(fpga);
        turnBus(fpga, BUSDIR::WRITE, cq);
        GET_TIME_VAL(0);
        // Write some the pattern to a set of rows before the retention
        // time is up
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

        // Wait for the retention time period
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
                printf(". ");
            }

            if(cur_bank_read == NUM_BANKS){ //NUM_BANKS
                //we are done with the entire DIMM
                printf(". \n");
                break;
            }
        }
    }
}

void test_tras(fpga_t* fpga, const uint retention, const uint tras, const uint wait_to_read)
{
    assert(tras > 0); //tras should be at elast one cycle (2.5ns)

    uint8_t pattern = 0xff;
    CmdQueue* cq = nullptr;

    uint delay_trp = 5;
    uint trcd = 6;

    uint group_size = retention*6; //number of rows to be written in a single iteration
    uint cur_row_write = 0;
    uint cur_col_write = 0;
    uint cur_bank_write = 0;

    uint cur_row_oc = 0; //for activate-precharge phase
    uint cur_col_oc = 0;
    uint cur_bank_oc = 0;

    uint cur_row_read = 0;
    uint cur_col_read = 0;
    uint cur_bank_read = 0;
    bool dimm_done = false;

    GET_TIME_INIT(4);

    while(!dimm_done){
        //GET_TIME_VAL(2);
        //calibZQ(fpga, cq);
        turnBus(fpga, BUSDIR::WRITE, cq);
        GET_TIME_VAL(0);
        for(int i = 0; i < group_size; i++){
            writeCol(fpga, cur_col_write, cur_row_write, cur_bank_write, pattern, cq);
            cur_row_write++;

            if(cur_row_write == NUM_ROWS){ //NUM_ROWS
                cur_row_write = 0;
                cur_col_write += 8;
            }

            if(cur_col_write == NUM_COLS){ //NUM_COLS
                cur_bank_write++;
                cur_col_write = 0;
            }

            if(cur_bank_write == NUM_BANKS){
                dimm_done = true;
                break;
            }
        }


        turnBus(fpga, BUSDIR::READ, cq);
        //wait for the retention
        do{
            GET_TIME_VAL(1);
        } while((TIME_VAL_TO_MS(1) - TIME_VAL_TO_MS(0)) < retention);

        GET_TIME_VAL(0);
        for(int i = 0; i < group_size; i++){
            openAndCloseRow(fpga, cur_row_oc, cur_bank_oc, tras - 1, delay_trp, cq);
            cur_row_oc++;

            if(cur_row_oc == NUM_ROWS){ //NUM_ROWS
                cur_row_oc = 0;
                cur_col_oc += 8;
            }

            if(cur_col_oc == NUM_COLS){ //NUM_COLS
                cur_bank_oc++;
                cur_col_oc = 0;
                printf(". ");
                fflush(stdout);
            }

            if(cur_bank_oc == NUM_BANKS){
                break;
            }
        }


        //wait for the retention
        do{
            GET_TIME_VAL(1);
        } while((TIME_VAL_TO_MS(1) - TIME_VAL_TO_MS(0)) < wait_to_read);

        for(int i = 0; i < group_size; i++){
            readAndCompareCol(fpga, cur_col_read, cur_row_read, cur_bank_read, pattern, trcd, cq);
            cur_row_read++;

            if(cur_row_read == NUM_ROWS){ //NUM_ROWS
                cur_row_read = 0;
                cur_col_read += 8;
            }

            if(cur_col_read == NUM_COLS){ //NUM_COLS
                cur_bank_read++;
                cur_col_read = 0;
            }

            if(cur_bank_read == NUM_BANKS){
                break;
            }
        }

        //GET_TIME_VAL(3);
        //printf("%d rows processed in %f ms \n", group_size, (TIME_VAL_TO_MS(3) - TIME_VAL_TO_MS(2)));
    }

    delete cq;
}

// Test if tRCD-induced errors are transient by repeatedly reading the
// same row using different tRCD values
void trcd_transient_test(fpga_t* fpga, const uint retention, const uint trcd)
{
    assert(trcd > 0); //trcd should be at elast one cycle (2.5ns)

    uint8_t pattern[] = {0xff, 0x00};
    uint pid = 0;
    CmdQueue* cq = nullptr;

    uint cur_row_write = 0;
    uint cur_col_write = 0;
    uint cur_bank_write = 0;

    uint cur_row_read = 0;
    uint cur_col_read = 0;
    uint cur_bank_read = 0;

    GET_TIME_INIT(2);

    // One row test on two patterns
    for (int pid = 0; pid < 2; pid++)
    {
        printf("***FUNC:%s\trcd:%d\t\tret:%dms\tpattern: 0x%x***\n", __func__, trcd, retention,
                pattern[pid]);
        turnBus(fpga, BUSDIR::WRITE, cq);
        GET_TIME_VAL(0);
        writeRow(fpga, cur_row_write, cur_bank_write, pattern[pid], cq);
        turnBus(fpga, BUSDIR::READ, cq);

        //wait for the retention
        do{
            GET_TIME_VAL(1);
        } while((TIME_VAL_TO_MS(1) - TIME_VAL_TO_MS(0)) < retention);

        // Read each column?
        for(int col_id = 0; col_id < NUM_COLS; col_id+=BURST_LEN)
        {
            // Default tRCD
            printf("RD COL%d\n", col_id);
            readAndCompareCol(fpga, col_id, cur_row_read, cur_bank_read,
                    pattern[pid], DEF_TRCD, cq);

            // Re-read with fast tRCD
            printf("FAST RD COL%d\n", col_id);
            readAndCompareCol(fpga, col_id, cur_row_read, cur_bank_read,
                    pattern[pid], trcd, cq);

            // Re-read with default tRCD
            printf("RD COL%d\n", col_id);
            readAndCompareCol(fpga, col_id, cur_row_read, cur_bank_read,
                    pattern[pid], DEF_TRCD, cq);

            printf("RD COL%d\n", col_id);
            readAndCompareCol(fpga, col_id, cur_row_read, cur_bank_read,
                    pattern[pid], DEF_TRCD, cq);

            // Fast tRCD
            printf("FAST RD COL%d\n", col_id);
            readAndCompareCol(fpga, col_id, cur_row_read, cur_bank_read,
                    pattern[pid], trcd, cq);
            printf("\n");
        }
    }
}

// Go through as many rows as possible within a specified retention
// period in column fashion (iterate the same column for all rows first
// before switching to the next column).
// |-----retention------|-----retention------|-----retention------|
// |write,write,........|-----wait-----------|read,read,........|
void batch_test_trcd_col_order(fpga_t* fpga, const uint retention, const uint trcd)
{
    assert(trcd > 0); //trcd should be at elast one cycle (2.5ns)

    uint8_t pattern[] = {0xff, 0x00};
    uint pid = 0;
    CmdQueue* cq = nullptr;

    uint group_size = retention*6; //number of rows to be written in a single iteration
    uint cur_row_write = 0;
    uint cur_col_write = 0;
    uint cur_bank_write = 0;

    uint cur_row_read = 0;
    uint cur_col_read = 0;
    uint cur_bank_read = 0;
    bool dimm_done = false;

    GET_TIME_INIT(2);

    while(!dimm_done){
        turnBus(fpga, BUSDIR::WRITE, cq);

        GET_TIME_VAL(0);
        pid = 0;
        for(int i = 0; i < group_size; i++){
            writeCol(fpga, cur_col_write, cur_row_write, cur_bank_write, pattern[pid], cq);
            pid = pid ? 0 : 1;
            cur_row_write++;

            if(cur_row_write == NUM_ROWS){ //NUM_ROWS
                cur_row_write = 0;
                cur_col_write += 8;
            }

            if(cur_col_write == NUM_COLS){ //NUM_COLS
                cur_bank_write++;
                cur_col_write = 0;
            }

            if(cur_bank_write == NUM_BANKS){
                dimm_done = true;
                break;
            }
        }

        turnBus(fpga, BUSDIR::READ, cq);

        //wait for the retention
        do{
            GET_TIME_VAL(1);
        } while((TIME_VAL_TO_MS(1) - TIME_VAL_TO_MS(0)) < retention);

        pid = 0;
        for(int i = 0; i < group_size; i++){
            readAndCompareCol(fpga, cur_col_read, cur_row_read, cur_bank_read, pattern[pid], trcd, cq);
            pid = pid ? 0 : 1;
            cur_row_read++;

            if(cur_row_read == NUM_ROWS){ //NUM_ROWS
                cur_row_read = 0;
                cur_col_read += 8;
            }

            if(cur_col_read == NUM_COLS){ //NUM_COLS
                cur_bank_read++;
                printf(". ");
                fflush(stdout);
                cur_col_read = 0;
            }

            if(cur_bank_read == NUM_BANKS){
                break;
            }
        }
    }
}

// Go through one column at a time across all rows first, then wraps
// around back to the first row.
//             |-----retention------|
// | write col |-----wait-----------| read col (default rcd), read col (fast rcd) |
void test_trcd_col_order(fpga_t* fpga, const uint retention, const uint trcd,
        ofstream& histo_file, ofstream& count_file, uint8_t pattern)
{
    assert(trcd > 0); //trcd should be at elast one cycle (2.5ns)

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
    count_file << "READ    " << retention << "ms" << " tRCD:" << trcd <<
        "Pattern 0x" << hex << unsigned(pattern) << dec << endl;

    // Start going through every bank
    for (uint bank = 0; bank < NUM_BANKS; bank++)
    {
        cout << "Testing bank" << bank << endl;
        int total_bank_cl_errs = 0;
        // Column order
        for (uint col = 0; col < NUM_COLS; col += BURST_LEN)
        {
            cout  << "\rTesting Column: " << col;
            fflush(stdout);
            for (uint row = 0; row < NUM_ROWS; row++)
           // for (uint row = 0; row < 2*NUM_ROWS; row+=2) //Incrementing by 2 as we are covering only 1GB
                                                      //Better layout distribution of the sampled addresses
            {
                //cout << "\r" << row << " ";
                //fflush(stdout);
                //cout << "row" << row << endl;
                turnBus(fpga, BUSDIR::WRITE, cq);
                GET_TIME_VAL(0);
                writeCol(fpga, col, row, bank, pattern, cq);

                //cout << "wait ret" << endl;
                // Wait for the retention
                do{
                    GET_TIME_VAL(1);
                } while((TIME_VAL_TO_MS(1) - TIME_VAL_TO_MS(0)) < retention);

                turnBus(fpga, BUSDIR::READ, cq);
                // Default tRCD read
                //cout << "compare col" << endl;
                unsigned burst_errors[8] = {0,0,0,0,0,0,0,0} ;
                int err_cnt = readAndCompareColCount(fpga, col, row, bank, pattern,
                        DEF_TRCD, cq, burst_errors);
								if (err_cnt == -1)
								{
												row -- ;
												continue ;
								}

                if (err_cnt)
                {
                    printf(RED "FAILED on first read! err count=%d\n" RESET, err_cnt);
                    histo_file << "FAILED on first read!\n";
                    return;
                }

                // Fast tRCD read
                err_cnt = readAndCompareColCount(fpga, col, row, bank, pattern,
                        trcd, cq, burst_errors);
                err_map_bank[err_cnt]++;
								if (err_cnt == -1)
								{
												row -- ;
												continue ;
								}


                // Count every error location
                if (err_cnt > 0)
                {
                    count_file << bank << " " << row << " " << col << " " << err_cnt <<
                        " b " << burst_errors[0] <<
                        ":" << burst_errors[1] <<
                        ":" << burst_errors[2] <<
                        ":" << burst_errors[3] <<
                        ":" << burst_errors[4] <<
                        ":" << burst_errors[5] <<
                        ":" << burst_errors[6] <<
                        ":" << burst_errors[7] <<
                        endl;
                    total_bank_cl_errs++;
                }
            }
        }
        cout << endl;

        // -- Done going through the entire bank --

        // Report some simple stats: % of cache lines with errors
        if (total_bank_cl_errs > 0)
        {
            double total_cl = NUM_ROWS * NUM_COLS / BURST_LEN;
            cout << RED "Bank Error $lines %: " << total_bank_cl_errs / total_cl <<
                "" RESET << endl;
        }

        // Dump out the histogram to a file
        histo_file << "BANK " << bank << " PDF:";
        int cnt;
        for (int berr = 0; berr <= cline_size_bits; berr++)
        {
            cnt = 0;
            if (err_map_bank.find(berr) != err_map_bank.end())
                cnt = err_map_bank[berr];
            histo_file << " " << cnt;
        }
        err_map_bank.clear();
        histo_file << endl;
    }
    histo_file << endl;
}

