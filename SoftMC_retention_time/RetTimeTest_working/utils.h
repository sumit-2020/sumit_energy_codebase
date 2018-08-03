#ifndef SAFARIMC_UTILS_H
#define SAFARIMC_UTILS_H

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

#define BANK_OFFSET 3
#define ROW_OFFSET 15
#define CMD_OFFSET 3

#define CMD_SIZE 2 //2 words

#define NUM_ROWS 32768
#define NUM_COLS 1024
#define NUM_BANKS 8

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

enum class BUSDIR	{
	READ = 0,
	WRITE = 2
};

//END - DO NOT EDIT

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


class DramAddr{

	public:
		uint row;
		uint bank;

		DramAddr() : DramAddr(0, 0){}
		DramAddr(uint row, uint bank){ this->row = row; this->bank = bank;}
};

Cmd genRowCMD(uint row, uint bank, MC_CMD c);
Cmd genWriteCMD(uint col, uint bank, uint8_t pattern);
Cmd genReadCMD(uint col, uint bank);
Cmd genWaitCMD(uint cycles_to_wait);
Cmd genBusDirCMD(BUSDIR dir);
Cmd genStartTR();
Cmd genZQCMD();


#endif //SAFARIMC_UTILS_H
