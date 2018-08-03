#include "test.h"

Test::Test(int sandbox){
    this->sandbox = (sandbox == 1);

    if (!this->sandbox){
        int fid = 0;
        fpga_info_list info;
        if (fpga_list(&info) != 0) {
            printf("Error populating fpga_info_list\n");
            exit(EXIT_FAILURE);
        }
        this->fpga = fpga_open(fid);
        if(!(this->fpga)){
            printf("Problem on opening the fpga \n");
            exit(EXIT_FAILURE);
        }
        printf("The FPGA has been opened successfully! \n");
        fpga_reset(this->fpga);
    }
    else
        printf("Sandbox mode is active\n");

    this->cq = new CmdQueue(2048);
    this->ns = new NodeState();
}

Test::~Test(){
    // printf("End of Test\n");
    delete this->ns;
	delete this->cq;
	if (!this->sandbox)
    	fpga_close(this->fpga);
	usleep(500 * 1000); // takes microseconds
}

void Test::act_pre (DramAddr * a) {
    this->act(a);
    this->pre(a);
}

void Test::act (DramAddr * a) {
    this->cq->insert(genRowCMD(a->row, a->bank, MC_CMD::ACT));
    this->cq->insert(genWaitCMD(DEF_TRAS));
}

void Test::pre (DramAddr * a) {
    this->cq->insert(genRowCMD(a->row, a->bank, MC_CMD::PRE));
    this->cq->insert(genWaitCMD(DEF_TRP));
}

void Test::buff_wait(uint n){
    for (int i = 0 ; i < n ; i++){
        this->cq->insert(genWaitCMD(1));
    }
}

void Test::act_rd_pre (DramAddr * a) {
    this->act(a);

    this->cq->insert(genReadCMD(0, a->bank));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));

    this->pre(a);
}

void Test::act_rd_rd_pre (DramAddr * a){
    this->act(a);
    this->cq->insert(genWaitCMD(DEF_TRCD));

    // First Read
    this->cq->insert(genReadCMD(0, a->bank));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));

    // Second Read
    this->cq->insert(genReadCMD(120, a->bank));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));

    this->pre(a);
    this->cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
}

void Test::act_rd_rd_rd_pre (DramAddr * a){
    this->act(a);
    this->cq->insert(genWaitCMD(DEF_TRCD));

    // First Read
    this->cq->insert(genReadCMD(0, a->bank));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));

    // Second Read
    this->cq->insert(genReadCMD(120, a->bank));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));

    // Third Read
    this->cq->insert(genReadCMD(240, a->bank));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));

    this->pre(a);
    this->cq->insert(genWaitCMD(DEF_TRP)); //Wait for tRP
}

void Test::write_column(DramAddr * da, uint8_t pattern){
    this->cq->insert(genWriteCMD(da->col, da->bank, pattern, AUTO_PRECHARGE::NO_AP));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
}

void Test::write_column_turnbus(DramAddr * da, uint8_t pattern){
    this->turn_bus(BUSDIR::WRITE);
    this->cq->insert(genWriteCMD(da->col, da->bank, pattern, AUTO_PRECHARGE::NO_AP));
    this->cq->insert(genWaitCMD(3));
    this->turn_bus(BUSDIR::READ);
}

void Test::read_column(DramAddr * da){
    this->cq->insert(genReadCMD(da->col, da->bank, AUTO_PRECHARGE::NO_AP));
    this->cq->insert(genWaitCMD(3));
}

void Test::read_entire_row(DramAddr * a){
    this->cq->insert(genRowCMD(a->row, a->bank, MC_CMD::ACT));
    this->cq->insert(genWaitCMD(DEF_TRCD-1));
    for(int i = 0; i < NUM_COLS; i+=8){
        this->cq->insert(genReadCMD(i, a->bank, AUTO_PRECHARGE::NO_AP));
        this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    }
    this->cq->insert(genRowCMD(a->row, a->bank, MC_CMD::PRE));
    this->cq->insert(genWaitCMD(DEF_TRP-1));
}

void Test::write_entire_row(DramAddr * a, uint8_t pattern){
    this->cq->insert(genRowCMD(a->row, a->bank, MC_CMD::ACT));
    this->cq->insert(genWaitCMD(DEF_TRCD-1));
    for(int i = 0; i < NUM_COLS; i+=8){
        this->cq->insert(genWriteCMD(i, a->bank, pattern, AUTO_PRECHARGE::NO_AP));
        this->cq->insert(genWaitCMD(DEF_TCL+DEF_TBURST-1));
    }
    this->cq->insert(genRowCMD(a->row, a->bank, MC_CMD::PRE));
    this->cq->insert(genWaitCMD(DEF_TRP-1));
}

void Test::act_wr_pre (DramAddr * a, uint8_t pattern){
    this->act(a);

    this->cq->insert(genWriteCMD(0, a->bank, pattern));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));

    this->cq->insert(genWaitCMD(3)); //Wait some more in any case
    this->pre(a);
}

void Test::act_wr_wr_pre (DramAddr * a, uint8_t pattern){
    this->act(a);

    this->cq->insert(genWriteCMD(0, a->bank, pattern));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));

    this->cq->insert(genWriteCMD(64, a->bank, pattern));
    this->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));

    this->cq->insert(genWaitCMD(3)); //Wait some more in any case
    this->pre(a);
}

void Test::turn_bus(BUSDIR dir){
    this->cq->insert(genBusDirCMD(dir));
    this->cq->insert(genWaitCMD(5)); //wait for the bus to turn
}

bool Test::close_open_banks(){
    this->buff_wait(3);
    std::cout << "Banks to be precharged: ";
    bool active_bank_flag = false;
    for (int i = 0 ; i < 8 ; i++){
        std::string bank_state = this->ns->get("bank"+std::to_string(i));
        if (bank_state.find("act") != -1){
            DramAddr * da = new DramAddr(0,i);
            this->pre(da);
            active_bank_flag = true;
            std::cout << da->bank << " ";
        }
    }
    std::cout << std::endl;

    return active_bank_flag;
}

void Test::send_commands(bool add_startTr){
    if (add_startTr)
        this->cq->insert(genStartTR());

    if (this->cq->size < INIT_SIZE) {
        std::cout << "Program does not have a looping part!" << std::endl;
    }
    // printf("Command Queue contains %d commands.\n", this->cq->size);
    if (this->sandbox){
        std::cout << "Sandbox mode. No command was sent to the FPGA. But " << this->cq->size << " commands are generated!"<< std::endl;
        // this->cq->dump();
    }
    else{
        int send_cnt = fpga_send(this->fpga, this->ch, (void *) this->cq->cmds , CMD_SIZE * this->cq->size, 0 , 1 , 5000 );
        if (send_cnt <= 0) {
            std::cout << "FPGA send has timed out!" << std::endl;
            delete this;
            exit(EXIT_FAILURE);
        }
        else {
            std::cout << send_cnt << " Bytes are sent to the FPGA!" << std::endl;
        }
    }
    this->ns->update_record(this->sandbox);
}

void Test::generate_loop_trace(std::string test_name_str){
    if (this->cq->size > INIT_SIZE){
        std::ofstream myfile ("./trace_out/"+test_name_str+".trace");
        if (myfile.is_open())
            myfile << "cmd,len\n";
            myfile << "INITIAL,PRECHARGED_STANDBY\n";
            for (int i = INIT_SIZE ; i < this->cq->size ; i++)
                myfile << this->cq->get_instr_str(this->cq->cmds[i]) + "\n";
    }
}

void Test::read_back(){
    uint rbuf[BUFF_SIZE];
    std::cout << "Read Data: " << std::endl;
    int num_recv = 0;
    if (!this->sandbox){
        do {
            num_recv = fpga_recv(this->fpga, 0, (void*)rbuf, BUFF_SIZE, 1000);
            for (int i = 0 ; i < num_recv ; i++){
                std::cout << std::hex << rbuf[i] << " ";
            }
            std::cout << std::endl;
        } while(num_recv > 0);
    }
}

bool Test::end_of_init(){
    bool ret = this->cq->expand_init();
    return ret;
}

bool Test::end_of_loop(uint wait_param){
    if (wait_param > 0)
        return this->cq->expand_loop(wait_param);
    else
        return this->cq->size <= (INIT_SIZE + LOOP_SIZE);
}

void Test::stop_looping(){
    this->cq->insert(genStopCMD());
}

void Test::wait_for(int remaining_wait){
    while (remaining_wait > 0) {
        int wait_for = remaining_wait;
        if (wait_for > 1023)
            wait_for = 1023;
        this->cq->insert(genWaitCMD(wait_for));
        remaining_wait -= wait_for;
    }
}
