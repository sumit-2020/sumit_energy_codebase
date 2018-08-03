/**
 * SoftMC API implementation
 *
 * @author Hasan Hassan <hasanibrahimhasan@gmail.com>
 * @author Kevin Chang <kevincha@cmu.edu>
 * @author Abhijith Kashyap <akashyap@andrew.cmu.edu>
 */
#include <stdio.h>
#include "softmc_api.h"

/** DRAM Command Handlers **/
Cmd genRowCMD(uint row, uint bank, MC_CMD c, PRECHARGE pc){
    if( c == MC_CMD::ACT || c == MC_CMD::PRE ){
        Cmd cmd = (uint)MC_CMD::DDR;
        cmd <<= 32 - CMD_OFFSET - BANK_OFFSET - ROW_OFFSET;
        cmd |= c == MC_CMD::ACT ? 0x23 : 0x22;//to set CKE(1) CS(0)[assumed it should be Low] RAS(0) CAS(1) WE_ACT(1) : WE_PRE(0)   
        if(c == MC_CMD::PRE && pc == PRECHARGE::ALL){//All banks PRECHARGE
            cmd <<= BANK_OFFSET + ROW_OFFSET - COL_OFFSET;
            cmd |= 0x1;// To set A[10] for PRECHARGE all banks
            cmd <<= COL_OFFSET;
        }
        else{//Single bank PRECHARGE or ACTIVATE
            cmd <<= BANK_OFFSET;
            cmd |= bank;
            cmd <<= ROW_OFFSET;
            cmd |= row;
        }
        // printf("%d Bank %d Row %d Precharge %d                \t: 0x%08x \n",c,bank,row,pc,cmd);
        return cmd;
    }

    assert(false && "Got an unexpected MC_CMD type!");
}

/**
 * @brief Create a write command transaction to the FPGA.
 * CMD packet structure:
 * |------------|---------|------------|-------------|--------|-----------|
 *  pattern[7:5] bank[2:0] pattern[4:0] 3 unused bits col[6:0] cmd (3bits)
 **/
Cmd genWriteCMD(uint col, uint bank, uint8_t pattern, AUTO_PRECHARGE ap){
    Cmd cmd = 1;
    cmd <<=31 - SIGNAL_OFFSET - BANK_OFFSET - ROW_OFFSET;

    cmd |= pattern >> 2; //most significant 6 bits of the pattern
    cmd <<= SIGNAL_OFFSET;

    cmd |= 0x24;//to set CKE(1) CS(0)[assumed it should be Low] RAS(1) CAS(0) WE(0)
    cmd <<=BANK_OFFSET;

    cmd |= bank;
    cmd <<= 2;

    cmd |= pattern & 0x3; //least significant 2 bits of the pattern 
    if(ap == AUTO_PRECHARGE::AP){
        cmd <<= 4;
        cmd |= 0x1;//to set A[10] 1
        cmd <<= COL_OFFSET;
    }
    else{
        cmd <<= ROW_OFFSET - 2;
    }

    cmd |= col;

    // printf("WR Bank %d Col %d Pattern 0x%02x Precharge %d \t: 0x%08x \n",bank,col,pattern,ap,cmd);
    return cmd;
}

Cmd genReadCMD(uint col, uint bank, AUTO_PRECHARGE ap){ 
    Cmd cmd = 0x25;//to set CKE(1) CS(0)[assumed it should be Low] RAS(1) CAS(0) WE(1)
    cmd <<= BANK_OFFSET;

    cmd |= bank;
    if(ap == AUTO_PRECHARGE::NO_AP)
        cmd <<= ROW_OFFSET; //to simplify the decoder we leave some unused bits
    else{
        cmd <<= ROW_OFFSET - COL_OFFSET; //to simplify the decoder we leave some unused bits
        cmd |= 0x1;//To set A[10] which is auto precharge bit
        cmd <<= COL_OFFSET;
    }
    
    cmd |= col;

    Cmd cmd2 = (uint)MC_CMD::READ;
    cmd2 <<= 28;
    cmd2 |= cmd;
    // printf("RD Bank %d Col %d Precharge %d                \t: 0x%08x \n",bank,col,ap,cmd2);
    return cmd2;
}

Cmd genWaitCMD(uint cycles_to_wait){ //min 1, max 1023
    assert(cycles_to_wait >= 1);
    assert(cycles_to_wait <= 1023);
        
    Cmd cmd = (uint)MC_CMD::WAIT;
    cmd <<= 28;
    cmd |= cycles_to_wait;

    // printf("WAIT %d                                       \t: 0x%08x \n",cycles_to_wait,cmd);

    return cmd;
}

Cmd genBusDirCMD(BUSDIR dir){
    Cmd cmd = (uint)MC_CMD::SET_BUS_DIR;
    cmd <<= 28;
    cmd |= (uint)dir;

    // printf("BUSDIR %d                                     \t: 0x%08x \n",dir,cmd);
    return cmd;
}

Cmd genStartTR(){
    return (Cmd)((uint)MC_CMD::SEND_TR << 28);
}

Cmd genZQCMD(){
    Cmd cmd = (uint)MC_CMD::ZQ;
    cmd <<= 32 - CMD_OFFSET - BANK_OFFSET - ROW_OFFSET;
    cmd |= 0x26;//to set CKE(1) CS(0) RAS(1) CAS(1) WE(0)
    cmd <<= BANK_OFFSET + ROW_OFFSET;

    return cmd;

    //return (Cmd)((uint)MC_CMD::ZQ << 28);
}

Cmd genStopCMD(){
    Cmd cmd = (uint)MC_CMD::STOP;
    cmd <<= 28;

    return cmd;
}

Cmd genRefCMD(){
    Cmd cmd = (uint)MC_CMD::REF;
    cmd <<= 32 - CMD_OFFSET - BANK_OFFSET - ROW_OFFSET;

    cmd |= 0x21;//to set CKE(1) CS(0) RAS(0) CAS(0) WE(1)

    cmd <<= BANK_OFFSET + ROW_OFFSET;

    return cmd;
    //return (Cmd)((uint)MC_CMD::REF << 28);
}

Cmd genRefConfigCMD(uint val, MC_CMD c){
    assert(c == MC_CMD::SET_TREFI || c == MC_CMD::SET_TRFC);
    assert(val < 0x10000000);

    Cmd cmd = (uint) c;
    cmd <<= 28;
    cmd |= val;

    return cmd;
}

Cmd genPowerDownCMD(uint val){
    Cmd cmd = (uint) 0x8;
    cmd <<= 32 - CMD_OFFSET - BANK_OFFSET - ROW_OFFSET; 
    if (val == 0)
        cmd |= 0x28;//to set CKE(0) CS(1) RAS(0) CAS(0) WE(0)
    else if (val == 1)
        cmd |= 0x08;//to set CKE(1) CS(1) RAS(0) CAS(0) WE(0)
    else
        assert(false && "PowerDown command has 2 modes: 0 and 1. Function can not get another value as argument.");
    cmd <<= BANK_OFFSET + ROW_OFFSET;
    return cmd;
}

Cmd genResetCMD(uint val){
    Cmd cmd = (uint) 0x8;
    cmd <<= 32 - CMD_OFFSET - BANK_OFFSET - ROW_OFFSET; 
    if (val == 0)
        cmd |= 0x06;//to set CKE(0) CS(0) RAS(1) CAS(1) WE(0)
    else if (val == 1)
        cmd |= 0x2E;//to set CKE(1) CS(1) RAS(1) CAS(1) WE(0)
    else
        assert(false && "Reset command has 2 modes: 0 and 1. Function can not get another value as argument.");
    cmd <<= BANK_OFFSET + ROW_OFFSET;

    return cmd;
}

Cmd genSelfRefCMD(uint val){
    Cmd cmd = (uint) 0x8;
    cmd <<= 32 - CMD_OFFSET - BANK_OFFSET - ROW_OFFSET; 
    if (val == 0)
        cmd |= 0x06;//to set CKE(0) CS(0) RAS(1) CAS(1) WE(0)
    else if (val == 1)
        cmd |= 0x0E;//to set CKE(0) CS(1) RAS(1) CAS(1) WE(0)
    else
        assert(false && "Self Refresh command has 2 modes: 0 and 1. Function can not get another value as argument.");
    cmd <<= BANK_OFFSET + ROW_OFFSET;

    return cmd;
}

Cmd genRdaCMD(){
    Cmd cmd = (uint) 0x8;
    cmd <<= 32 - CMD_OFFSET - BANK_OFFSET - ROW_OFFSET; 
    cmd |= 0x25;//to set CKE(1) CS(0) RAS(1) CAS(0) WE(1)
    cmd <<= BANK_OFFSET + ROW_OFFSET;
    return cmd;
}

/**
 * @brief Obsolete function. The HW handles zq calibration automatically.
 **/
void calibZQ(fpga_t* fpga, CmdQueue*& cq)
{
    int ch = 0; //riffa channel should always be 0
    if(cq == nullptr)
        cq = new CmdQueue();
    //reuse the provided CmdQueue to avoid dynamic allocation for each call
    else
        cq->size = 0;

    cq->insert(genZQCMD());

    //WAIT
    cq->insert(genWaitCMD(64)); //at least 64 cycles we should wait

    //START Transaction
    // cq->insert(genStartTR());

    std::cout << "Command Queue of Stop Loop" << std::endl;
    cq->dump();
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

/**
 * @briefa Turn the bus direction based on read or write
 **/
void turnBus(fpga_t* fpga, BUSDIR b, CmdQueue* cq = nullptr)
{
    int ch = 0; //riffa channel should always be 0
    if(cq == nullptr)
        cq = new CmdQueue();
    //reuse the provided CmdQueue to avoid dynamic allocation for each call
    else
        cq->size = 0;

    cq->insert(genBusDirCMD(b));
    cq->insert(genWaitCMD(5)); //wait for the bus to turn

    //START Transaction
    cq->insert(genStartTR());
    fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
}

bool CmdQueue::expand_init(){
    if (this->size > INIT_SIZE) {
        std::cout << "Code segment is yuuuge for the init procedure" << std::endl;
        return false;
    }
    else {
        // std::cout << (INIT_SIZE-size) << " of wait(1) commands are inserted!" << std::endl;
        while (size < INIT_SIZE) {
            this->insert(genWaitCMD(1));
        }
    }
    return true;
}

bool CmdQueue::expand_loop(uint wait_param){
    if ( this->size >= LOOP_SIZE + INIT_SIZE ) {
        std::cout << "Code segment is yuuuge for the loop" << std::endl;
        return false;
    }
    else {
        uint injected_wait_cnt = LOOP_SIZE + INIT_SIZE - this->size;
        double wait_per_iter = ( (double) injected_wait_cnt * FPGA_CYCLE ) / 1000;
        std::cout << std::dec << injected_wait_cnt << " of wait(" << wait_param;
        std::cout << ") commands are inserted! Loop will wait for " << wait_per_iter;
        std::cout << "ms. per iteration." << std::endl;
        while (size < INIT_SIZE + LOOP_SIZE) {
            this->insert(genWaitCMD(wait_param));
        }
    }
    return true;
}