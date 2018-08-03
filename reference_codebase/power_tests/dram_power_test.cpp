#include "dram_power_test.h"

/*
 * Helper functions 
 */

float refresh_time = 0;

void refreshBank(fpga_t * fpga, CmdQueue*& cq, uint bank, uint precharge) 
{

  if (cq == nullptr) {
      cq = new CmdQueue();
  } else {
      //printf("Setting to 0..");
      cq->size = 0;
      //printf("size: %u\n", CMD_SIZE*cq->size);
  }

  uint ch = 0;
  uint sub = 4;
  
  //printf("Refreshing bank %d...\n", bank);
  // PRE bank before refreshing
  if (precharge) {
      cq->insert(genWaitCMD(DEF_TRAS));
      cq->insert(genRowCMD(0, bank, MC_CMD::PRE));
      cq->insert(genWaitCMD(DEF_TRP));
      sub += 2;
  }

  for (uint row = 0; row < NUM_ROWS; row++){
      cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
      cq->insert(genWaitCMD(DEF_TRAS));
      cq->insert(genRowCMD(0, bank, MC_CMD::PRE));
      cq->insert(genWaitCMD(DEF_TRP));
      if (CMD_SIZE*cq->size >= DEF_QUEUE_SIZE-sub) {
          //printf("CQ size in if: %u\n", CMD_SIZE*cq->size);
          cq->insert(genStartTR());
          fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0); 
          cq->size = 0;
          sub = 4;
      }
  }
  //printf("Refreshed the whole bank! CQ size: %u\n", CMD_SIZE*cq->size);
  cq->insert(genStartTR());
  //printf("Started the transaction\n");
  fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0); 
  //printf("Sent the commands\n");
}


uint refresh(fpga_t *fpga, CmdQueue*& cq, uint bank_num, uint all, uint retention,
             uint precharge) 
{
  GET_TIME_VAL_DRAM(200);
  GET_TIME_VAL_DRAM(201);
  
    uint refreshed = 0;
    if (all) {
        for (uint bank = 0; bank < NUM_BANKS; bank++) {	 
            GET_TIME_VAL_DRAM(NUM_BANKS + bank);
            if ((TIME_VAL_TO_MS_DRAM(NUM_BANKS + bank) - TIME_VAL_TO_MS_DRAM(bank)) > retention){
                refreshBank(fpga, cq, bank, precharge);
                GET_TIME_VAL_DRAM(bank);
                refreshed = 1;
            }
        }
    } 

    else {   
        GET_TIME_VAL_DRAM(NUM_BANKS + bank_num);
        if ((TIME_VAL_TO_MS_DRAM(NUM_BANKS + bank_num) - TIME_VAL_TO_MS_DRAM(bank_num)) > retention){
            //printf("Refresh @t=%f\n", (TIME_VAL_TO_MS_DRAM(NUM_BANKS + bank_num) 
            //                          - TIME_VAL_TO_MS_DRAM(bank_num)));
            refreshBank(fpga, cq, bank_num, precharge);
            GET_TIME_VAL_DRAM(bank_num);
            refreshed = 1;
        }
    }

    GET_TIME_VAL_DRAM(201);

    refresh_time += (TIME_VAL_TO_MS_DRAM(201) - TIME_VAL_TO_MS_DRAM(200))/1000;

    return refreshed;

}

void writeAllPattern(fpga_t *fpga, uint pattern, CmdQueue*& cq, uint idle)
{
    turnBus(fpga, BUSDIR::WRITE, cq);
    for (uint bank = 0; bank < NUM_BANKS; bank++) {
        for (uint row = 0; row < NUM_ROWS; row++) {
            printf("(%u,%u)\n", bank, row);
            writeRow(fpga, row, bank, pattern, cq);
            if (!idle) {
                refresh(fpga, cq, 0, 1, DEF_RETENTION, 0);
            }
        }
    }
}

/*
 * Test routines
 */

/*
 * START OLD TESTS
 */

void doRefTest(fpga_t *fpga, CmdQueue*& cq, uint test_name, uint pattern) 
{
    // Write all 1's
    //writeAllPattern(fpga, pattern, cq, 0); 

    //printf("Test number: %u\n", test_name);
    switch(test_name) {
        case IDLE_TEST:
            // Test 0: No commands no refresh, measure with
            // different patterns (0s, 1s)
            idleTest();
            break;
        case IDLE_TEST_REF:
            // Test 1: Just refresh
            idleTestRefresh(fpga, cq);
            break;
        case CLOSED_ROW_READ_MISS:
            // Test 2: ACT-RD-PRE
            closedRowReadMiss(fpga, cq, pattern);
            break;
        case OPEN_ROW_READ_MISS:
            // Test 3: ACT-RD-PRE
            openRowReadMiss(fpga, cq, pattern);
            break;
        case CLOSED_ROW_WRITE_MISS:
            // Test 5: ACT-WR-PRE
	    closedRowWriteMiss(fpga, cq, pattern);
	    break;
        case OPEN_ROW_WRITE_MISS:
	    openRowWriteMiss(fpga, cq, pattern);
	    break;
        default:
            printf("Before default\n");
            idleTest();
    }

}


void closedRowWriteMiss(fpga_t *fpga, CmdQueue*& cq, uint pattern)
{
  printf("GOT INTO THE CLOSED ROW WRITE MISS TEST\n");
  uint row = 0; // Always write to same row                                                         
  uint col = 0; // Always going to write to column zero                                             
  uint bank = 0;

  for (bank = 0; bank < NUM_BANKS; bank++){
    for (row = 0; row < NUM_ROWS; row++) {
      //printf("bank, row: %u %u\n", bank, row);
      //refresh(fpga, cq, row % NUM_BANKS, 0, DEF_RETENTION, 0);
      writeRow(fpga, row, bank, pattern, cq);
    }
  }
}

void closedRowReadMiss(fpga_t *fpga, CmdQueue*& cq, uint pattern)
{
    printf("GOT INTO THE CLOSED ROW READ MISS TEST\n");
    uint row = 0;
    uint col = 0;
    uint bank = 0;
    
    for (bank = 0; bank < NUM_BANKS; bank++){
      for (uint row = 0; row < NUM_ROWS; row++) {
	//printf("bank, row: %u %u\n", bank, row);
	//refresh(fpga, cq, row % NUM_BANKS, 0, DEF_RETENTION, 0); 
	readAndCompareRow(fpga, row, bank, pattern, cq);
      }
    }
}


void openRowReadMiss(fpga_t *fpga, CmdQueue*& cq, uint pattern)
{
    printf("GOT INTO THE OPEN ROW READ MISS TEST\n");
    uint row = 0;
    uint col = 0;
    uint bank = 0;

    for (uint bank = 0; bank < NUM_BANKS; bank++) {
      for (row = 0; row < NUM_ROWS; row++) {
	//printf("Bank, Row : %u %u\n", bank, row);
	refresh(fpga, cq, row % NUM_BANKS, 0, DEF_RETENTION, row % NUM_BANKS == bank); 
	openRowRead(fpga, row, bank, pattern, DEF_TRCD, cq);
      } 
    }
}

void openRowWriteMiss(fpga_t * fpga, CmdQueue*& cq, uint pattern)
{

  printf("GOT INTO THE OPEN ROW WRITE MISS TEST\n");
  uint row = 0;
  uint col = 0;
  uint bank = 0;
  
  for(uint bank = 0;bank < NUM_BANKS; bank++) {
    for (row = 0; row < NUM_ROWS; row++) {
      //printf("Bank, Row : %u %u\n", bank, row);
      refresh(fpga, cq, row % NUM_BANKS, 0, DEF_RETENTION, row % NUM_BANKS == bank);
      openRowWrite(fpga, row, bank, pattern, DEF_TRCD, cq);
    }
  }

}

/*
 * END OLD TESTS
 */

/*
 * START NEW TESTS WITHOUT REFRESH AND WRITE ALL MECHANISM
 */

void doTest(fpga_t *fpga, CmdQueue*& cq, uint test_name, uint pattern) 
{
    uint startBank = 0;
    uint endBank = 0;
    uint startRow = 0;
    uint endRow = 0;
    uint numRws = 500000;
    uint policy = CLOSE;
    uint num_cols = NUM_COLS;


    switch (test_name) {
        case IDLE_TEST:
            // Test 0: No commands no refresh, measure with
            // different patterns (0s, 1s)
            idleTest();
            break;
        case IDLE_TEST_REF:
            // Test 1: No commands, just refresh
            idleTestRefresh(fpga, cq);
            break;
        case PRE_TEST:
            // Test 2: Just precharges
            prechargeTest(fpga, cq, startBank, endBank);
            break;
        case WRITE_TEST:
            // Test 3: Just writes
            rwTest(fpga, cq, pattern, startBank, endBank,
                   startRow, endRow, numRws, policy, WR, num_cols);
            break;
        case READ_TEST:
            // Test 4: Just reads
            rwTest(fpga, cq, pattern, startBank, endBank,
                   startRow, endRow, numRws, policy, RD, num_cols);
            break;
        case ACT_PRE_TEST: 
            // Test 5: ACT-PRE repeatedly
           actPreTest(fpga, cq, pattern, startBank, endBank,
                      startRow, endRow, numRws, policy, num_cols);
            // actPreTestSingleRow(fpga, cq, startBank, startRow, 
            //     numRws, num_cols);
            break;
        default:
            printf("Before default\n");
            idleTest();
    }

}

void idleTest(void)
{
    // Stays idle...
    return;
}

void idleTestRefresh(fpga_t *fpga, CmdQueue*& cq) 
{
    printf("GOT INTO THE IDLE REFRESH TEST\n");
    for (uint i = 0; i < 300000000; i++) {
        refresh(fpga, cq, i % NUM_BANKS, 0, DEF_RETENTION, 0);
    }
}

void prechargeTest(fpga_t *fpga, CmdQueue*& cq, uint startBank, uint endBank)
{
    printf("GOT INTO THE PRECHARGE TEST\n");
    for (uint i = 0; i < 100000; i++) {
        // Sanity check the bank nums
        assert(startBank <= endBank);
        assert(startBank < NUM_BANKS);
        assert(endBank < NUM_BANKS);
        // Precharge a range of banks
        for (uint bank = startBank; bank <= endBank; bank++) {
            printf("Precharging bank %u\n", bank);
            precharge(fpga, cq, bank);
        }
    }

}

// Can specify banks to write to, rows to write, number of writes to a row, 
// and row close/open policy
void rwTest(fpga_t *fpga, CmdQueue*& cq, uint pattern,
            uint startBank, uint endBank, uint startRow, uint endRow, 
            uint numRws, uint policy, uint rw, uint num_cols)
{
    printf("GOT INTO THE RW TEST, doing a %s\n", (rw == RD) ? "read" : "write");
    // Sanity check the bank nums and row nums
    assert(startBank <= endBank);
    assert(startBank < NUM_BANKS);
    assert(endBank < NUM_BANKS);
    assert(startRow <= endRow);
    assert(startRow < NUM_ROWS);
    assert(endRow < NUM_ROWS);

    /*
    turnBus Write
    loop over banks
        loop over rows
            if (open)
                precharge here
                wait tRP
                activate
                wait tRCD
            loop to write this row n times
                if (close) 
                    activate
                    wait trcd
                loop to write(row, bank)
                    wait TCL + TBURST
                if (close)
                    precharge
                    wait trp
            if (open) wait TRAS - tRCD - TCl - TBURST 
     */

     int ch = 0;

     if (cq == nullptr) cq = new CmdQueue();
     else cq->size = 0;

     if (rw == WR) turnBus(fpga, BUSDIR::WRITE, cq);
     else turnBus(fpga, BUSDIR::READ, cq);

     // Loop over banks
     for (uint bank = startBank; bank <= endBank; bank++) {

         if (bank > startBank && policy == OPEN) {
             // Precharge - on open row policy, last bank will
             // still be open and must be precharged
             cq->insert(genRowCMD(0, bank-1, MC_CMD::PRE));
             // Wait TRP
             cq->insert(genWaitCMD(DEF_TRP));
         }
 
         // Loop over rows
         for (uint row = startRow; row <= endRow; row++) {

             if (policy == OPEN) {
                 if (row > startRow) {
                     // Precharge
                     cq->insert(genRowCMD(0, bank, MC_CMD::PRE));
                     // Wait TRP
                     cq->insert(genWaitCMD(DEF_TRP));
                 }
                 // Activate row
                 cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
                 // Wait TRCD
                 cq->insert(genWaitCMD(DEF_TRCD));
             }

             for (uint i = 0; i < numRws; i++) {

                 if (policy == CLOSE) {
                     // Activate row
                     cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
                     // Wait TRCD
                     cq->insert(genWaitCMD(DEF_TRCD));
                 }

                 // Write whole row
                 for (int j = 0; j < num_cols; j += BURST_LEN) {
                     if (rw == WR) {
                         cq->insert(genWriteCMD(j, bank, pattern));
                     } else {
                         cq->insert(genReadCMD(j, bank));
                     }

                     // Wait TCL + TBURST 
                     cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
                 }

                 // Wait some more
                 cq->insert(genWaitCMD(3));
                 
                 if (policy == CLOSE) {
                     // Precharge
                     cq->insert(genRowCMD(0, bank, MC_CMD::PRE));
                     // Wait TRP
                     cq->insert(genWaitCMD(DEF_TRP));
                 }

                 // Send commands for the write
                 cq->insert(genStartTR());
                 fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
                 cq->size = 0;

                 //if (rw == RD) {
                 //    receive(fpga, ch, pattern, row, bank, i, num_cols); 
                 //}

             }

             if (policy == OPEN) {
                 cq->insert(genWaitCMD(3));
             }
         }
     }

     // Precharge
     cq->insert(genRowCMD(0, endBank, MC_CMD::PRE));
     // Wait TRP
     cq->insert(genWaitCMD(DEF_TRP));
     // Send the remaining commands
     cq->insert(genStartTR());
     fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
     cq->size = 0;
}

// Test that does the same thing as rwTest but it doesn't actually read or write.
// This way you can subtract the power of this test from rwTest to isolate read and
// write power.
void actPreTest(fpga_t *fpga, CmdQueue*& cq, uint pattern,
                uint startBank, uint endBank, uint startRow, uint endRow, 
                uint numRws, uint policy, uint num_cols)
{
    printf("GOT INTO THE ACT-PRE TEST!\n");
    // Sanity check the bank nums and row nums
    assert(startBank <= endBank);
    assert(startBank < NUM_BANKS);
    assert(endBank < NUM_BANKS);
    assert(startRow <= endRow);
    assert(startRow < NUM_ROWS);
    assert(endRow < NUM_ROWS);

    int ch = 0;

    if (cq == nullptr) cq = new CmdQueue();
    else cq->size = 0;

    turnBus(fpga, BUSDIR::WRITE, cq);

    // Loop over banks
    for (uint bank = startBank; bank <= endBank; bank++) {

        if (bank > startBank && policy == OPEN) {
            // Precharge - on open row policy, last bank will
            // still be open and must be precharged
            cq->insert(genRowCMD(0, bank-1, MC_CMD::PRE));
            // Wait TRP
            cq->insert(genWaitCMD(DEF_TRP));
        }

        // Loop over rows
        for (uint row = startRow; row <= endRow; row++) {

            for (int i = 0 ; i < numRws ; i++){
                if (policy == OPEN) {
        
                    // Precharge
                    if (row > startRow){
                        cq->insert(genRowCMD(0, bank, MC_CMD::PRE));
                        // Wait TRP
                        cq->insert(genWaitCMD(DEF_TRP));
                        // Activate row
                    }
                    
                    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
                    // Wait TRCD
                    cq->insert(genWaitCMD(DEF_TRCD));
                }

                if (policy == CLOSE) {
                    // Activate row
                    cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
                    // Wait TRCD
                    cq->insert(genWaitCMD(DEF_TRCD));
                }

                // Wait some more
                cq->insert(genWaitCMD(3));
                    
                if (policy == CLOSE) {
                    // Precharge
                    cq->insert(genRowCMD(0, bank, MC_CMD::PRE));
                    // Wait TRP
                    cq->insert(genWaitCMD(DEF_TRP));
                }

                // Send commands for the write
                // KEY_DIFF: don't receive b/c nothing actually read
                cq->insert(genStartTR());
                fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
                cq->size = 0;   
                
                if (policy == OPEN) {
                    cq->insert(genWaitCMD(3));
                }
            }
        }
    }

    // Precharge
    cq->insert(genRowCMD(0, endBank, MC_CMD::PRE));
    // Wait TRP
    cq->insert(genWaitCMD(DEF_TRP));
    // Send the remaining commands
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
    cq->size = 0;
}

void actPreTestSingleRow(fpga_t *fpga, CmdQueue*& cq,
                uint bank, uint row, uint numRws, uint num_cols)
{
    printf("GOT INTO THE ACT-PRE-SINGLE-ROW TEST!\n");

    int ch = 0;

    if (cq == nullptr) cq = new CmdQueue();
    else cq->size = 0;

    turnBus(fpga, BUSDIR::WRITE, cq);

    // Repeat activate and precharge
    for (int i = 0 ; i < numRws ; i++){
        // Activate row
        cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
        // Wait TRCD
        cq->insert(genWaitCMD(DEF_TRCD));
        // Wait some more
        cq->insert(genWaitCMD(3));
        // Precharge
        cq->insert(genRowCMD(row, bank, MC_CMD::PRE));
        // Wait TRP
        cq->insert(genWaitCMD(DEF_TRP));

        // Send commands for the write
        // KEY_DIFF: don't receive b/c nothing actually read
        cq->insert(genStartTR());
        fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
        cq->size = 0;   
    }

    // Precharge
    cq->insert(genRowCMD(row, bank, MC_CMD::PRE));
    // Wait TRP
    cq->insert(genWaitCMD(DEF_TRP));
    // Send the remaining commands
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
    cq->size = 0;
}


/*
 * END NEW TESTS WITHOUT REFRESH AND WRITE ALL MECHANISM
 */

// Main test function that calls alls the other tests
void test(fpga_t* fpga, uint pattern, CmdQueue*& cq) 
{ 
    uint ch = 0;

    // Set up timers
    GET_TIME_INIT(2 * NUM_BANKS);
    for (uint bank = 0; bank < NUM_BANKS; bank++) {
      GET_TIME_VAL_DRAM(bank); // Fill all timers[i] with initial time 
    }

    for (uint i = 100; i < 102; i ++) {
      GET_TIME_VAL_DRAM(i);
    }

    for (uint i = 200; i < 202; i ++) {
      GET_TIME_VAL_DRAM(i);
    }

    // OLD TESTS
    //doRefTest(fpga, cq, IDLE_TEST, pattern);
    //doRefTest(fpga, cq, IDLE_TEST_REF, pattern);
    //doRefTest(fpga, cq, CLOSED_ROW_WRITE_MISS, pattern);
    //doRefTest(fpga, cq, CLOSED_ROW_READ_MISS, pattern);
    //doRefTest(fpga, cq, OPEN_ROW_READ_MISS, pattern);
    //doRefTest(fpga, cq, OPEN_ROW_READ_HIT, pattern);  
    //doRefTest(fpga, cq, OPEN_ROW_WRITE_MISS, pattern);
    //doRefTest(fpga, cq, OPEN_ROW_READ_MISS, pattern);
    
    // NEW TESTS
    doTest(fpga, cq, ACT_PRE_TEST, pattern);
    //doTest(fpga, cq, IDLE_TEST_REF, pattern);
    //doTest(fpga, cq, PRE_TEST, pattern);
    //doTest(fpga, cq, WRITE_TEST, pattern);
    //doTest(fpga, cq, READ_TEST, pattern);


    GET_TIME_VAL_DRAM(101);
    float time_t;
    time_t = (TIME_VAL_TO_MS_DRAM(101) - TIME_VAL_TO_MS_DRAM(100));
    printf("Time taken = %f\n", time_t);
    printf("Refresh time = %f\n", refresh_time);
    //doTest(fpga, cq, ACT_PRE_TEST, pattern);
}


// sanity check test to make sure FPGA is actually running some of our commands
void baselineTest(fpga_t *fpga)
{
  //testRetention(fpga, 128);                                                                       
  /* Test tRCD */
  //ofstream rcd_f, rcd_cnt_f;                                                                      
  uint8_t pattern = 0xff;
  CmdQueue * cq = nullptr;
  uint row = 0;
  uint col = 0;
  uint bank = 0;
  uint trcd = DEF_TRCD;
  int riffa_ch = 0;
  //uint8_t pattern[4] = {0xff, 0x00, 0xcc, 0xaa}; //Dropping pattern 33. similar to cc             
  //uint8_t pattern[5] = {0xff, 0x00, 0xcc, 0x33, 0xaa};                                            

  // CQ is null, filled in write col                                                                
  // row 0, col 0, bank 0                                                                           
  for (int i = 0; i < 100; i++){
    turnBus(fpga, BUSDIR::WRITE, cq);
    writeColBuffered(fpga, col, row, bank, pattern, cq);
    turnBus(fpga, BUSDIR::READ, cq);
    readColBuffered(fpga, col, row, bank, pattern, trcd, cq);
    cq->insert(genStartTR());
    fpga_send(fpga, riffa_ch, (void*) cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
    cq->size = 0;

    //Receive the data                                                                              
    uint rbuf[16];
    fpga_recv(fpga, riffa_ch, (void*)rbuf, 16, 0);
    uint8_t* rbuf8 = (uint8_t *) rbuf;
    
    printf("Entering check...\n");
    for(int j = 0; j < 64; j++) {
      if(rbuf8[j] == pattern) {
	printf("Error at Byte: %d, Col: %d, Row: %u, Bank: %u, DATA: %x Correct Data: %x\n",
	       j, col, row, bank, rbuf8[j], pattern);
      }
    }


    
  }
}
