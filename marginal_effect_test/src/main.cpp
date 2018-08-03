#include "main.h"

using namespace std;

int main(int argc, char *argv[]) {

    // Get timestamp
    clock_t begin = clock();

    // Parse arguments
    string dimm_str("");
    string test_name_str("");
    uint8_t pattern(170);
    uint bank(0);
    uint row(0);
    uint num_ops(0);
    uint max_num_ops(0);

    if (argc < 3){
        cout << "Argument error. Use this: ./bin/iddtest dimm test_name pattern bank row num_ops max_num_ops" << endl;
        exit(EXIT_FAILURE);
    }
    else {
        dimm_str = argv[1];
        test_name_str = argv[2];
        if (argc > 3) {
            pattern = atoi(argv[3]);
            if (argc > 4){
                bank = atoi(argv[4]);
                if (argc > 5){
                    row = atoi(argv[5]);
                    if (argc > 6){
                        num_ops = atoi(argv[6]);
                        if (argc > 7){
                            max_num_ops = atoi(argv[7]);
                        }
                    }
                }
            }
        }
    }

    test_name t_name = arg_test_name_match(test_name_str);
    for(uint i = 0; i < dimm_str.length(); ++i) {
        dimm_str[i] = tolower(dimm_str[i]);
    }
    int sandbox_index = dimm_str.find("sandbox");
    bool sandbox = sandbox_index > -1;
    int wait_per_op = (num_ops > 0)? (max_num_ops*4) / num_ops : max_num_ops*4; //cycles
    int wait_per_op_wturnbus = (num_ops > 0)? (((max_num_ops*16) / num_ops) - 16): (max_num_ops*16);
    ////////////////////////////////////////////////////////
    // STOP LOOP
    ////////////////////////////////////////////////////////
    if (t_name == STOP_LOOP) {
        // Send single stop command to exit from the loop
        Test * my_test = new Test(sandbox);
        my_test->stop_looping();
        my_test->send_commands(false); // Don't send SENDTR
        my_test->read_back();
        delete my_test;
        return 0;
    }// Stop Loop

    ////////////////////////////////////////////////////////
    // TEST PROCEDURES
    ////////////////////////////////////////////////////////
    int col_addr [num_ops];
    for (int i = 0; i < num_ops ; i++ ){
        col_addr[i] = (i*8) % NUM_COLS;
    }
    // Initialization
    PowerTest * init = new PowerTest(sandbox);
    if (t_name == ACT_NRD_PRE_FT) { 
        int header_footer_cnts = 10;  
        int wrs_per_init = (INIT_SIZE - header_footer_cnts)/2; 
        for (int init_turn = 0; init_turn < num_ops/wrs_per_init; init_turn++){
            init->cq->insert(genBusDirCMD(BUSDIR::WRITE));
            init->cq->insert(genWaitCMD(5));
            init->cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
            init->cq->insert(genWaitCMD(DEF_TRCD-1));
            for (int i = 0; i < wrs_per_init; i++){
                int col_index = (wrs_per_init * init_turn) + i;
                init->cq->insert(genWriteCMD(col_addr[col_index], bank, pattern, AUTO_PRECHARGE::NO_AP));
                init->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST - 1));
            }
            init->cq->insert(genRowCMD(row, bank, MC_CMD::PRE));
            init->cq->insert(genWaitCMD(DEF_TRP-1));
            init->cq->insert(genBusDirCMD(BUSDIR::READ));
            init->cq->insert(genWaitCMD(5));
            init->send_commands();
        }
    }

    // Looping the procedure
    PowerTest * test_proc = new PowerTest(sandbox);
    test_proc->end_of_init();

    if (t_name == ACT_NWR_PRE_FT){
        test_proc->cq->insert(genBusDirCMD(BUSDIR::WRITE));
        test_proc->cq->insert(genWaitCMD(5));
    }
    test_proc->cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
    test_proc->cq->insert(genWaitCMD(DEF_TRCD-1));

    if (t_name == ACT_NRD_PRE_FT){
        for (int i = 0; i < num_ops; i++){
            test_proc->cq->insert(genReadCMD(col_addr[i], bank, AUTO_PRECHARGE::NO_AP));    
            test_proc->cq->insert(genWaitCMD(wait_per_op-1));
        }
    }
    else if (t_name == ACT_NWR_PRE_FT){
        for (int i = 0; i < num_ops; i++){
            test_proc->cq->insert(genWriteCMD(col_addr[i], bank, pattern, AUTO_PRECHARGE::NO_AP));    
            test_proc->cq->insert(genWaitCMD(wait_per_op-1));
        }
    } 
    else if (t_name == ACT_NBWR_PRE_FT){
        for (int i = 0; i < num_ops; i++){
            test_proc->cq->insert(genBusDirCMD(BUSDIR::WRITE));
            test_proc->cq->insert(genWaitCMD(5));
            test_proc->cq->insert(genWriteCMD(col_addr[i], bank, pattern, AUTO_PRECHARGE::NO_AP));    
            test_proc->cq->insert(genWaitCMD(3));
            test_proc->cq->insert(genBusDirCMD(BUSDIR::READ));
            test_proc->cq->insert(genWaitCMD(5));
            int temp_wait = wait_per_op_wturnbus;
	    while (temp_wait > 1023){
                test_proc->cq->insert(genWaitCMD(1023));
                temp_wait -= 1024;
            }
            if (temp_wait > 1){
                test_proc->cq->insert(genWaitCMD(temp_wait-1)); 
            }
        }
    } 
    else {
        cout << "WTF: Unknown test name: " << t_name << endl;
        delete init;
        delete test_proc;
        exit(EXIT_FAILURE);
    }
    if (t_name == ACT_NWR_PRE_FT){
        test_proc->cq->insert(genBusDirCMD(BUSDIR::READ));
        test_proc->cq->insert(genWaitCMD(5));
    }
    test_proc->cq->insert(genRowCMD(row, bank, MC_CMD::PRE));
    test_proc->cq->insert(genWaitCMD(DEF_TRP-1));
    if (sandbox){
        cout << "" << num_ops << ", " << wait_per_op_wturnbus;
        cout << ", " << num_ops * (16+wait_per_op_wturnbus) << endl;
    }
    test_proc->send_commands();
    
    delete init;
    delete test_proc;
}

test_name arg_test_name_match(string s){
    for(uint i = 0; i < s.length(); ++i) {
        s[i] = tolower(s[i]);
    }

    string tests [NUM_TESTS];
    tests[STOP_LOOP] = "stoploop";
    tests[ACT_NRD_PRE_FT] = "ftanrp,fixedtime_actnrdpre"; 
    tests[ACT_NWR_PRE_FT] = "ftanwp,fixedtime_actnwrpre";
    tests[ACT_NBWR_PRE_FT] = "ftanbwp,fixedtime_actnturnbuswrpre";
    
    for (int i = 0 ; i < NUM_TESTS ; i++){
        int loc = tests[i].find(s);
        if ( loc != -1){
            cout << "Test " << i << " is selected: " << tests[i] << endl;
            return (test_name) i;
        }
    }

    cout << "Could not find the test " << s << ". Please type one of the following: " << endl;
    for (int i = 0 ; i < NUM_TESTS ; i++){
        cout << tests[i] << " " << endl;
    }
    exit(EXIT_FAILURE);
}
