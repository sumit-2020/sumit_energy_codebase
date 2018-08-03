#ifndef SAFARIMC_UTILS_H
#define SAFARIMC_UTILS_H

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#define BANK_OFFSET 3
#define ROW_OFFSET 15
#define CMD_OFFSET 3

#define CMD_SIZE 2 //2 words

//#define NUM_ROWS 32768
//#define NUM_COLS 1024


//#define NUM_ROWS 1024
//#define NUM_COLS 1024

#define NUM_ROWS 16384
#define NUM_COLS 1024

//#define NUM_ROWS 32
//#define NUM_COLS 32

#define NUM_BANKS 8

#define BURST_LEN 8

// Default timing parameters in cycles (2.5ns / cycle)
#define DEF_TRCD 5
#define DEF_TRP  5
#define DEF_TRAS 14
#define DEF_TWR  6
#define DEF_TCL  6
#define DEF_TBURST  4

typedef uint64_t Cmd;
typedef uint32_t uint;

//DO NOT EDIT (unless you change the verilog code)
enum class MC_CMD {
    SEND_TR = 0,
    SET_BUS_DIR = 1,
    ACT = 2,
    PRE = 3,
    READ = 4,
    WRITE = 5,
    WAIT = 6,
    ZQ = 7
};
//END - DO NOT EDIT

enum class BUSDIR	{
    READ = 0,
    WRITE = 2
};

class CmdQueue{
    public:
        CmdQueue();
        CmdQueue(const uint capacity);
        virtual ~CmdQueue();

        void insert(const Cmd c);

        uint size;
        Cmd* cmds;
    private:
        uint capacity;
        const static uint init_cap = 256;
};

#endif //SAFARIMC_UTILS_H
