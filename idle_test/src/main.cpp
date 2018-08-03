#include "main.h"

using namespace std;

int main(int argc, char *argv[]) {

    // Parse arguments
    string test_name_str("");
    uint8_t active_banks(0);
    uint8_t pattern(170);
    int sandbox(0);

    if (argc < 3){
        print_help(argc);
        exit(EXIT_FAILURE);
    }
    else {
        test_name_str = argv[1];
        if (argc > 2) {
            active_banks = atoi(argv[2]);
            if (argc > 3){
                pattern = atoi(argv[3]);
                if (argc > 4){
                    sandbox = atoi(argv[4]);
                    if (sandbox == 1){
                        cout << "The commands will not be sent to the FPGA." <<endl;
                    }
                }
            }
        }
    }

    test_name t_name = arg_test_name_match(test_name_str);  

    if (t_name == STOP_LOOP) {
        // Send single stop command to exit from the loop
        Test * my_test = new Test(sandbox);
        my_test->stop_looping();
        my_test->send_commands(false); // Don't send SENDTR
        my_test->read_back();
        delete my_test;
        
        return 0;
    }// Stop Loop

    int num_banks = 0;

    DramAddr * da [8];
    for (int i = 0 ; i < 8 ; i++){
        da[i] = new DramAddr(0, i);
    }    

    for (int i = 0 ; i < 8 ; i++){
        if (CHECK_BIT(active_banks,i)){
            cout << "Initializing bank " << i << endl;
            Test * idle_init = new Test(sandbox);
            idle_init->turn_bus(BUSDIR::WRITE);
            idle_init->write_entire_row(da[i],pattern);
            idle_init->turn_bus(BUSDIR::READ);
            idle_init->send_commands();
            num_banks ++;
            delete idle_init;
        }
    }
    
    for (int i = 0 ; i < 8 ; i++){
        if (CHECK_BIT(active_banks,i)){
            cout << "Reading from bank " << i << endl;
            Test * idle_init = new Test(sandbox);
            idle_init->read_entire_row(da[i]);
            idle_init->send_commands();
            delete idle_init;
        }
    }

    PowerTest * idle_test = new PowerTest(sandbox);
    idle_test->end_of_init();

    for (int i = 0 ; i < 8 ; i++){
        if (CHECK_BIT(active_banks,i)){
            idle_test->cq->insert(genRowCMD(da[i]->row, da[i]->bank, MC_CMD::ACT));
            idle_test->cq->insert(genWaitCMD(12)); // required to satisfy tFAW
        }
    }
    while(idle_test->cq->size < LOOP_SIZE + INIT_SIZE - num_banks*2 - 3){
        idle_test->cq->insert(genWaitCMD(1023));
    }
    for (int i = 0 ; i < 8 ; i++){
        if (CHECK_BIT(active_banks,i)){
            idle_test->cq->insert(genRowCMD(da[i]->row, da[i]->bank, MC_CMD::PRE));
            idle_test->cq->insert(genWaitCMD(12)); // ACT has it, why not precharge
        }
    } 
    idle_test->send_commands();
   
    if (sandbox == 1){
        idle_test->generate_loop_trace(test_name_str);
    }

    delete idle_test;
    return 0;
}

void print_help(int argc){
    cout << endl;
    cout << "Usage: ./bin/iddtest [dimm_label] [test_name] [pattern] [bank] [row] [num_ops] [sandbox]" << endl;
    cout << endl;
    cout << "  dimm_label is required." << endl;
    cout << endl;
    cout << "  test_name is required." << endl;
    cout << "    test     : Functionality Test" << endl;
    cout << "    stoploop : Sends stop looping command in a special way!" << endl;
    cout << "               This should only be used if the FPGA is in looping state." << endl;
    cout << "    iddx     : Runs the procedure defined in datasheets for the corresponding IDD Test" <<endl;
    cout << "    busdir   : Switches the direction of the bi-directional bus (pattern is used as bus direction)" <<endl;
    cout << "    idlerd   : Read one row and then stay idle." <<endl;
    cout << "    idlewr0  : Write one row, turn bus to read and then stay idle." <<endl;
    cout << "    idlewr1  : Write one row, leave bus at write and then stay idle." <<endl;
    cout << endl;
    cout << "  pattern is optional (default: 0xaa)" << endl;
    cout << "    0: read, 1:write for bus direction test" << endl;
    cout << endl;
    cout << "  bank is optional (default: 0) (ignored for idd4 tests)" << endl;
    cout << endl;
    cout << "  row is optional (default: 0)" << endl;
    cout << endl;
    cout << "  num_ops is an optional parameter for ACT-RDxn-PRE test";
    cout << endl;
    cout << "  sandbox is also optional (default: 0)" << endl;
    cout << "    0: send commands to the fpga" << endl;
    cout << "    1: do not send the commands but print to stdout" << endl;
    cout << endl;
}

test_name arg_test_name_match(string s){
    for(uint i = 0; i < s.length(); ++i) {
        s[i] = tolower(s[i]);
    }

    string tests [NUM_TESTS];
    tests[STOP_LOOP]    = "stoploop";
    tests[IDD0]         = "idd0,actpre";
    tests[IDD1]         = "idd1,1rd,actrdpre";
    tests[IDD4R]        = "idd4r";
    tests[IDD4W]        = "idd4w";
    tests[A2RP]         = "actrdrdpre,act2rdspre";
    tests[A3RP]         = "actrdrdrdpre,act3rdspre";
    // tests[ANRPFT]       = "ftanrp,fixedtime_actnrdpre"; 
    tests[ANRP]         = "anrp,actnrdpre";
    tests[ANWP]         = "anwp,actnwrpre";
    tests[AARP]         = "readentirerow,actrdallpre";
    tests[AAWP]         = "writeentirerow,actwrallpre";
    tests[TEST]         = "test";
    tests[BUSDIRTEST]   = "busdir";
    tests[FORCED_PRECHARGE] = "fpre,forcepre,forcedpre";

    tests[IDLE_RD]      = "idlerd";
    tests[IDLE_WR_0]    = "idlewr0";
    tests[IDLE_WR_1]    = "idlewr1";

    for (int i = 0 ; i < NUM_TESTS ; i++){
        int loc = tests[i].find(s);
        if ( loc != -1){
            return (test_name) i;
        }
    }

    cout << "Could not find the test " << s << ". Please type one of the following: " << endl;
    for (int i = 0 ; i < NUM_TESTS ; i++){
        cout << tests[i] << " " << endl;
    }
    exit(EXIT_FAILURE);
}
