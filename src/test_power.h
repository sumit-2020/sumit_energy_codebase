#ifndef _DRAM_POWER_TESTS_H_
#define _DRAM_POWER_TESTS_H_

#include <assert.h>
#include "timer.h"
#include "test_routine.h"

#define IDLE_TEST             0 
#define IDLE_TEST_REF         1
#define CLOSED_ROW_READ_MISS  2
#define OPEN_ROW_READ_MISS    3
#define OPEN_ROW_READ_HIT     4
#define CLOSED_ROW_WRITE_MISS 5
#define PRE_TEST              6
#define WRITE_TEST            7
#define READ_TEST             8
#define ACT_PRE_TEST          9
#define OPEN_ROW_WRITE_MISS   10
#define IDD0_TEST			  11

// Missing parameters in the new codebase
#define DEF_QUEUE_SIZE 512
#define DEF_RETENTION 64 // in ms

static struct timeval _timers_dram[100];

#define GET_TIME_VAL_DRAM(num) gettimeofday(&_timers_dram[num], NULL)
#define TIME_VAL_TO_MS_DRAM(num) (((double)_timers_dram[num].tv_sec*1000.0) + ((double)_timers_dram[num].tv_usec/1000.0))



// Macros for open or closed row policy
#define OPEN  0
#define CLOSE 1

// Write and read macros
#define WR 0
#define RD 1

//extern struct timeval _timers_dram[2 * NUM_BANKS];

using namespace std;

/*
 * Helper Functions 
 */

void refreshBank(fpga_t * fpga,CmdQueue*& cq, uint bank);
uint refresh(fpga_t *fpga, CmdQueue*& cq, uint bank_num, uint all, uint retention);
void writeAllPattern(fpga_t *fpga, uint pattern, CmdQueue*& cq, uint idle);


/*
 * Test Routines
 */

// Old tests
void doRefTest(fpga_t *fpga, CmdQueue*& cq, uint test_name, uint pattern);
void closedRowReadMiss(fpga_t *fpga, CmdQueue*& cq, uint pattern);

void openRowReadMiss(fpga_t *fpga, CmdQueue*& cq, uint pattern);
void openRowWriteMiss(fpga_t * fpga, CmdQueue*& cq, uint pattern);

void openRowReadHit(fpga_t *fpga, CmdQueue*& cq, uint pattern, uint trcd);
void closedRowWriteMiss(fpga_t *fpga, CmdQueue*& cq, uint pattern);
void test(fpga_t* fpga, uint pattern, CmdQueue*& cq); 
void baselineTest(fpga_t *fpga);
// New tests
void doTest(fpga_t *fpga, CmdQueue*& cq, uint test_name, uint pattern);
void idleTest(void);
void idleTestRefresh(fpga_t *fpga, CmdQueue*& cq);
void prechargeTest(fpga_t *fpga, CmdQueue*& cq, uint startBank, uint endBank);
void rwTest(fpga_t *fpga, CmdQueue*& cq, uint pattern,
            uint startBank, uint endBank, uint startRow, uint endRow, 
            uint numRws, uint policy, uint rw, uint num_cols);
void actPreTest(fpga_t *fpga, CmdQueue*& cq, uint pattern,
            uint startBank, uint endBank, uint startRow, uint endRow, 
            uint numRws, uint policy, uint num_cols);

void idd0Test(fpga_t *fpga, CmdQueue*& cq, uint pattern, uint bank, uint row, 
			uint numRws, uint num_cols);

#endif
