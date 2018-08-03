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
	Cmd cmd = bank;

	cmd <<= ROW_OFFSET;
	cmd |= row;
	cmd <<= CMD_OFFSET;

	cmd|= (uint)c;

	return cmd;
}

Cmd genWriteCMD(uint col, uint bank, uint8_t pattern){
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

Cmd genReadCMD(uint col, uint bank){
	Cmd cmd = bank;
	cmd <<= ROW_OFFSET;
	cmd |= col;
	cmd <<= CMD_OFFSET;
	cmd|= (uint)MC_CMD::READ;

	return cmd;
}

Cmd genWaitCMD(uint cycles_to_wait){ //min 1, max 1023
	assert(cycles_to_wait >= 1);
	Cmd cmd = cycles_to_wait;
	cmd <<= CMD_OFFSET;
	cmd |= (uint)MC_CMD::WAIT;

	return cmd;
}

Cmd genBusDirCMD(BUSDIR dir){
	Cmd cmd = (uint)dir;
	cmd <<= CMD_OFFSET;
	cmd |= (uint)MC_CMD::SET_BUS_DIR;

	return cmd;
}

Cmd genStartTR(){
	return (Cmd)MC_CMD::SEND_TR;
}

Cmd genZQCMD(){
	return (Cmd)MC_CMD::ZQ;
}
