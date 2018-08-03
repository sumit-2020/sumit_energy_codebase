#include "main.h"

using namespace std;

int main(int argc, char *argv[]) {
    clock_t begin = clock();

    string dimm_str("");
    string test_name_str("");
    uint8_t pattern(170);
    uint bank(0);
    uint row(0);
    uint num_ops(0);
    int sandbox(0);

    if (argc < 3){
        print_help(argc);
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
                            sandbox = atoi(argv[7]);
                            if (sandbox == 1){
                                cout << "The commands will not be sent to the FPGA." <<endl;
                            }
                        }
                    }
                }
            }
        }
    }

    test_name t_name = arg_test_name_match(test_name_str);
    ////////////////////////////////////////////////////////
    if (t_name == STOP_LOOP) {
        // Send single stop command to exit from the loop
        Test * my_test = new Test(sandbox);
        my_test->stop_looping();
        my_test->send_commands(false); // Don't send SENDTR
        my_test->read_back();
        delete my_test;

    }// Stop Loop

    else if (t_name == TEST) {
        Dimm * my_dimm = new Dimm(dimm_str);
        for (int i = 0 ; i < 8 ; i++){
            DramAddr * da = new DramAddr(0,i);
            Test * my_test = new Test(sandbox);
            my_test->buff_wait(3);
            my_test->turn_bus(BUSDIR::WRITE);
            my_test->write_entire_row(da,pattern<<i);
            my_test->send_commands();
            delete my_test;
        }

        for (int i = 0 ; i < 8 ; i++){
            DramAddr * da = new DramAddr(0,i);
            Test * my_test = new Test(sandbox);
            my_test->buff_wait(3);
            my_test->turn_bus(BUSDIR::READ);
            my_test->read_entire_row(da);
            my_test->send_commands();
            my_test->read_back();
            delete my_test;
        }
    }// Functional Test
    ////////////////////////////////////////////////////////
    else if (t_name == IDD4R || t_name == IDD4W){
        PowerTest * idd4_test = new PowerTest(sandbox);
        idd4_test->turn_bus(BUSDIR::WRITE);

        if (t_name == IDD4R){
            idd4_test->idd4r_init(row,pattern); // initialize the rows with col[0]=0x00, col[1]=pattern
            idd4_test->turn_bus(BUSDIR::READ);
        }

        idd4_test->end_of_init();
        switch (t_name){
            case IDD4R : idd4_test->idd4r_loop(row);           break;
            case IDD4W : idd4_test->idd4w_loop(row,pattern);   break;
        }
        idd4_test->send_commands();
        // clock_t end = clock();
        // cout << "Time lapsed until sending the loop command: " << double(end - begin) / CLOCKS_PER_SEC << endl;
        if (sandbox == 1){
            idd4_test->generate_loop_trace(test_name_str);
        }
        delete idd4_test;
    }// IDD4 Tests
    ////////////////////////////////////////////////////////
    else if (t_name == BUSDIRTEST) {
        PowerTest * busdirtest = new PowerTest(sandbox);
        switch (num_ops){
            case 0: busdirtest->turn_bus(BUSDIR::READ);  break;
            case 1: busdirtest->turn_bus(BUSDIR::WRITE); break;
            default :
                cout << "invalid num_ops! 0: read, 1: write, else: invalid" << endl;
                exit(EXIT_FAILURE);
        }
        busdirtest->send_commands();
        delete busdirtest;
    } //BUS DIRECTION TEST
    ////////////////////////////////////////////////////////
    else {
        cout << "Test procedure is being generated" << endl;
        Dimm * my_dimm = new Dimm(dimm_str);
        DramAddr * da = new DramAddr(row,bank);
        PowerTest * init_test = new PowerTest(sandbox);
        init_test->turn_bus(BUSDIR::WRITE);
        init_test->write_entire_row(da,pattern);
        init_test->turn_bus(BUSDIR::READ);
        init_test->send_commands();
        delete init_test;

        PowerTest * idd_test = new PowerTest(sandbox);
        idd_test->end_of_init();
        // my_dimm->dump_latencies();
        switch(t_name) {
            // Datasheet defined procedures
            case IDD0   : idd_test->idd0_loop(my_dimm->latencies, da);  break;
            case IDD1   : idd_test->idd1_loop(my_dimm->latencies, da);  break;
            // Custom procedures
            case A2RP   : idd_test->act_rd_rd_pre(da);                        break;
            case A3RP   : idd_test->act_rd_rd_rd_pre(da);                     break;
            case ANRPFT : idd_test->act_rdn_pre_fix_time(da,num_ops);         break;
            case ANRP   : idd_test->act_rdn_pre(da,num_ops);                  break;
            case ANWP   : idd_test->act_wrn_pre(da,num_ops,pattern);          break;
            case ANWPFT : idd_test->act_wrn_pre_fix_time(da,num_ops,pattern); break;
            case AARP   : idd_test->read_entire_row(da);                      break;
        }

        idd_test->send_commands();
        clock_t end = clock();
        cout << "Time lapsed until sending the loop command: " << double(end - begin) / CLOCKS_PER_SEC << endl;
        // if (sandbox == 1){
            // idd_test->generate_loop_trace(test_name_str);
        // }
        delete idd_test;
    }
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
    tests[ANRP]         = "anrp,actnrdpre";
    tests[ANWP]         = "anwp,actnwrpre";
    tests[ANRPFT]       = "ftanrp,fixedtime_actnrdpre";
    tests[ANWPFT]       = "ftanwp,fixedtime_actnwrpre";
    tests[AARP]         = "readentirerow,actrdallpre";
    tests[AAWP]         = "writeentirerow,actwrallpre";
    tests[TEST]         = "test";
    tests[BUSDIRTEST]   = "busdir";

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
