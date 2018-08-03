#ifndef SAFARIMC_UTILS_H
#define SAFARIMC_UTILS_H

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#define BANK_OFFSET 3
#define ROW_OFFSET 16
#define CMD_OFFSET 4
#define COL_OFFSET 10
#define SIGNAL_OFFSET 6

#define CMD_SIZE 2 //2 words

#define NUM_ROWS 32768
#define NUM_COLS 1024
#define NUM_BANKS 8
#define NUM_CACHELINES 128

#define INIT_SIZE 512 // Initialization Code capacity is 512 commands
#define LOOP_SIZE 1535 // {2k commands} - {init commands} - {SEND_TR command}
#define FPGA_CYCLE 2.5

#define BURST_LEN 8
#define CACHE_LINE_BYTES 64
#define CACHE_LINE_BITS 512
#define COLS_PER_CACHELINE 8

// Default timing parameters in cycles (2.5ns / cycle)
//#define DEF_TRCD 5 // Defined in the makefile
//#define DEF_TRP  5 // Defined in the makefile
#define DEF_TRAS 14
#define DEF_TWR  6
#define DEF_TCL  6
#define DEF_TBURST  4
#define DEF_TCK 2500 //ps
#define DEF_TRFC 120

// Precharge cmd: all banks in the rank or just one bank
#define PRE_ALL_BANK 1
#define PRE_BANK     0

// Color code
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

// Debug macro code
#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

#define debug_print(fmt, ...) \
    do { if (DEBUG_TEST) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, ## __VA_ARGS__); } while (0)

#define detail_print(fmt, ...) \
    do { fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, ## __VA_ARGS__); } while (0)

typedef uint32_t uint;

#endif //SAFARIMC_UTILS_H
