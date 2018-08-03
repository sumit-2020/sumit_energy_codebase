#include "utils.h"
#include <fstream>
#include <iostream>
#include <cassert>

using namespace std;

CmdQueue::CmdQueue() : CmdQueue(init_cap){
}

CmdQueue::CmdQueue(const uint capacity){
	cmds = new Cmd[init_cap];
	size = 0;
	this->capacity = capacity;
}

CmdQueue::~CmdQueue(){
	delete[] cmds;
}

void CmdQueue::insert(const Cmd c){
	if(size == capacity){
		Cmd* tmp = new Cmd[capacity*2];
		
		for(int i = 0; i < size; i++)
			tmp[i] = cmds[i];

		delete[] cmds;
		capacity *=2;
		cmds = tmp;
	}

	cmds[size++] = c;
}

Cmd genRowCMD(uint row, uint bank, MC_CMD c){

	//TODO: check if c is a row command
    Cmd cmd = (Cmd)c;
    cmd <<= 28 - ROW_OFFSET;

	cmd |= bank;

	cmd <<= ROW_OFFSET;
	cmd |= row;

	return cmd;
}

Cmd genWriteCMD(uint col, uint bank, uint8_t pattern){
	Cmd cmd = pattern >> 5; //most significant 3 bits of the pattern
	cmd <<= 3;

	cmd |= bank;
	cmd <<= 5;

	cmd |= pattern & 0x1F; //least significant 5 bits
	cmd <<= COL_OFFSET;	

	cmd |= col;

    Cmd cmd2 = (uint)MC_CMD::WRITE;
    cmd2 <<= 28;
    cmd2 |= cmd;

	return cmd2;
}

Cmd genReadCMD(uint col, uint bank){
	Cmd cmd = bank;
	cmd <<= ROW_OFFSET; //to simplify the decoder we leave some unused bits
	cmd |= col;

    Cmd cmd2 = (uint)MC_CMD::READ;
    cmd2 <<= 28;
    cmd2 |= cmd;

	return cmd2;
}

Cmd genWaitCMD(uint cycles_to_wait){ //min 1, max 1023
	assert(cycles_to_wait >= 1);
    Cmd cmd = (uint)MC_CMD::WAIT;
    cmd <<= 28;
    cmd |= cycles_to_wait;

	return cmd;
}

Cmd genBusDirCMD(BUSDIR dir){
	Cmd cmd = (uint)MC_CMD::SET_BUS_DIR;
	cmd <<= 28;
	cmd |= (uint)dir;

	return cmd;
}

Cmd genStartTR(){
	return (Cmd)((uint)MC_CMD::SEND_TR << 28);
}

Cmd genZQCMD(){
	return (Cmd)((uint)MC_CMD::ZQ << 28);
}

Cmd genRefCMD(){
    return (Cmd)((uint)MC_CMD::REF << 28);
}

Cmd genRefConfigCMD(uint val, MC_CMD c){
    assert(c == MC_CMD::SET_TREFI || c == MC_CMD::SET_TRFC);
    assert(val < 0x10000000);

    Cmd cmd = (uint) c;
    cmd <<= 28;
    cmd |= val;

    return cmd;
}
