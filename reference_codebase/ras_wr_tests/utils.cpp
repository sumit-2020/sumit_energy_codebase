#include "utils.h"

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
