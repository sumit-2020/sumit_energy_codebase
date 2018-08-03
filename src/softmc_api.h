#ifndef SOFTMC_API_H_
#define SOFTMC_API_H_

#include "utils.h"
#include <riffa.h>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string.h>
#include <bitset>

typedef uint64_t Cmd;
typedef uint32_t uint;

//DO NOT EDIT (unless you change the verilog code)
enum class MC_CMD {
    SEND_TR = 0,
    SET_BUS_DIR = 1,
    SET_TREFI = 2,//3
    SET_TRFC = 3,//4
    ACT = 5,//0101
    PRE = 6,//0110
    READ = 8,
    WRITE = 10,
    WAIT = 4,//12
    ZQ = 14,//1110
    REF = 15,
    DDR = 9, //1001
    STOP = 7
};

enum class BUSDIR   {
    READ = 0,
    WRITE = 2
};

enum class AUTO_PRECHARGE {
    NO_AP = 0,
    AP = 1
};

enum class PRECHARGE {
    SINGLE = 0,
    ALL = 1
};

//END - DO NOT EDIT

class CmdQueue{
    public:
        CmdQueue() : CmdQueue(init_cap) {}
        CmdQueue(const uint capacity_)
        {
            cmds = new Cmd[capacity_];
            size = 0;
            capacity = capacity_;
        }
        ~CmdQueue() { delete[] cmds; }

        void insert(const Cmd c)
        {
            if(size == capacity)
            {
                assert(capacity*2 <= init_cap);
                Cmd* tmp = new Cmd[capacity*2];
                for(int i = 0; i < size; i++)
                    tmp[i] = cmds[i];

                delete[] cmds;
                capacity *=2;
                cmds = tmp;
            }
            cmds[size++] = c;
        }

        void dump()
        {
            int end_of_init = INIT_SIZE;
            if (this->size < INIT_SIZE)
                end_of_init = this->size;
            // std::cout << "Init Code:" << std::endl;
            this->dump_commands(0,end_of_init);
            // std::cout << "End of Init" << std::endl;
            // std::cout << "Loop Code:" << std::endl;
            this->dump_commands(INIT_SIZE,this->size);
            std::cout << "End of Loop" << std::endl;
        }

        void dump_commands(uint start_index, uint stop_index)
        {
            Cmd prev_cmd = cmds[start_index];
            // int cnt = 1;
            for (int i = start_index ; i < stop_index ; i++){
                // if (prev_cmd == cmds[i] && false){
                //     cnt ++;
                // }
                // else{
                    // printf("%03d : %08x \n",i,cmds[i]);
                    // std::cout << std::setfill('0') << std::setw(3) << std::dec << i ;
                    // std::cout << " : " ;
                    std::bitset<32> c (cmds[i]) ;
                    std::cout << c << std::endl;
                    // prev_cmd = cmds[i];
                    // cnt = 1;
                // }
            }
        }

        std::string get_instr_str(Cmd cmd)
        {
            uint opcode = cmd >> 28;
            std::string opcode_str = "";
            int cycles = 1;
            switch (opcode){
                case  0: opcode_str = "send_tr";        break;
                case  1: opcode_str = "set_bus_dir";    break;
                case  2: opcode_str = "set_trefi";      break;
                case  3: opcode_str = "set_trfc";       break;
                case  4: opcode_str = "wait";
                         cycles     = cmd & 1023;       break;
                case  5: opcode_str = "act";            break;
                case  6: opcode_str = "pre";            break;
                case  7: opcode_str = "stop";           break;
                case  8: opcode_str = "read";           break;
                case  9: opcode_str = "actorpre";       break;
                case 10: opcode_str = "write";          break;
                case 14: opcode_str = "zq";             break;
                case 15: opcode_str = "ref";            break;
                default: opcode_str = "unknown";        break;
            }

            return opcode_str + "," + std::to_string(cycles);
        }

        bool expand_init();
        bool expand_loop(uint);

        uint size;
        Cmd* cmds;
    private:
        uint capacity;
        const static uint init_cap = 2048;
};

class DramAddr{

    public:
        uint col;
        uint row;
        uint bank;

        DramAddr() : DramAddr(0, 0, 0){}
        DramAddr(uint bank) : DramAddr(0, 0, bank){}
        DramAddr(uint row, uint bank) : DramAddr( 0, row, bank){}
        DramAddr(uint col, uint row, uint bank){
            assert(bank < NUM_BANKS);
            this->col = col;
            this->row = row;
            this->bank = bank;
        }
};

// FPGA calls
Cmd genRowCMD(uint row, uint bank, MC_CMD c, PRECHARGE ap = PRECHARGE::SINGLE);
Cmd genWriteCMD(uint col, uint bank, uint8_t pattern, AUTO_PRECHARGE ap = AUTO_PRECHARGE::NO_AP);
Cmd genReadCMD(uint col, uint bank, AUTO_PRECHARGE ap = AUTO_PRECHARGE::NO_AP);
Cmd genWaitCMD(uint cycles_to_wait);
Cmd genBusDirCMD(BUSDIR dir);
Cmd genStartTR();
Cmd genZQCMD();
Cmd genStopCMD();
Cmd genRefCMD();
Cmd genRefConfigCMD(uint val, MC_CMD c);

//HPCA Extensions
Cmd genPowerDownCMD(uint val);
Cmd genResetCMD(uint val);
Cmd genSelfRefCMD(uint val);
Cmd genRdaCMD();

// Calls to config the state
void calibZQ(fpga_t* fpga, CmdQueue*& cq);
void turnBus(fpga_t* fpga, BUSDIR b, CmdQueue* cq);

#endif // SOFTMC_API_H_
