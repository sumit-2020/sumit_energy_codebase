#include<bitset>
#include "main.h"

using namespace std;

int main(int argc, char *argv[]) {
    // Parse arguments
    string dimm_str("");
    string test_name_str("");
    if (argc < 3){
        print_help(argc);
        exit(EXIT_FAILURE);
    }
    else {
        dimm_str = argv[1];
        test_name_str = argv[2];
    }

    test_name t_name = arg_test_name_match(test_name_str);
    cout << "Is that a real DIMM, or sandbox mode?" << endl;
    cout << "- DIMM name is " << dimm_str << endl;
    for(uint i = 0; i < dimm_str.length(); ++i) {
        dimm_str[i] = tolower(dimm_str[i]);
    }
    int sandbox_index = dimm_str.find("sandbox");
    bool sandbox = sandbox_index > -1;
    if (sandbox)
      cout << "- Sandbox mode is active" << endl;
    else
      cout << "- NOT sandbox" << endl;
    /////////////////////////////////////////////////////////
    // STOP LOOP
    ////////////////////////////////////////////////////////
    if (t_name == STOP) {
        // Send single stop command to exit from the loop
        Test * my_test = new Test(sandbox);
        my_test->stop_looping();
        my_test->send_commands(false); // Don't send SENDTR
        my_test->read_back();
        delete my_test;
        return 0;
    }// Stop Loop

    cout << "Fetching DIMM parameters from csv file" << endl;
    Dimm * my_dimm = new Dimm(dimm_str);
    /////////////////////////////////////////////////////////
    // Initialize the rows with datasheet-defined
    // data patterns
    /////////////////////////////////////////////////////////
    cout << "Initializing the rows";
    if (t_name == SUMITFCFS){
        cout << " for SUMITFCFS test..." << endl;
        PowerTest * init_test = new PowerTest(sandbox);
        init_test->turn_bus(BUSDIR::WRITE);
        init_test->fcfs_init(0,0x33);
        init_test->turn_bus(BUSDIR::READ);
        init_test->send_commands();
        delete init_test;
    }
    else if (t_name == SUMITFRFCFS){
        cout << " for SUMITFRFCFS test..." << endl;
        PowerTest * init_test = new PowerTest(sandbox);
        init_test->turn_bus(BUSDIR::WRITE);
        init_test->frfcfs_init(0,0x33);
        init_test->turn_bus(BUSDIR::READ);
        init_test->send_commands();
        delete init_test;
    }
    else if (t_name == SUMITAHB){
        cout << " for SUMITAHB test..." << endl;
        PowerTest * init_test = new PowerTest(sandbox);
        init_test->turn_bus(BUSDIR::WRITE);
        init_test->ahb_init(0,0x33);
        init_test->turn_bus(BUSDIR::READ);
        init_test->send_commands();
        delete init_test;
    }
    else if (t_name == SUMITFRFCFSPRIORHIT){
        cout << " for SUMITFRFCFSPRIORHIT test..." << endl;
        PowerTest * init_test = new PowerTest(sandbox);
        init_test->turn_bus(BUSDIR::WRITE);
        init_test->frfcfspriorhit_init(0,0x33);
        init_test->turn_bus(BUSDIR::READ);
        init_test->send_commands();
        delete init_test;
    }
    else if (t_name == IDD4R || t_name == IDD4W){
        cout << " for idd4 test..." << endl;
        PowerTest * init_test = new PowerTest(sandbox);
        init_test->turn_bus(BUSDIR::WRITE);
        init_test->idd4r_init(0,0x33);
        init_test->turn_bus(BUSDIR::READ);
        init_test->send_commands();
        delete init_test;
    }
    else if (t_name == IDD0 || t_name == IDD1 || t_name == IDD2N || t_name == IDD3N || t_name == TEST) {
        cout << endl;
        int rows [2] = {0x000, 0x078};
        int patterns [2] = {0x00, 0x33};
        for (int bank = 0 ; bank < 8 ; bank++){
            cout << "- Initializing rows: ";
            for (int row_index = 0 ; row_index < 2 ; row_index++){
                cout  << rows[row_index] << " ";
                DramAddr * da = new DramAddr(rows[row_index], bank);
                PowerTest * init_test = new PowerTest(sandbox);
                init_test->turn_bus(BUSDIR::WRITE);
                init_test->write_entire_row(da,patterns[row_index]);
                init_test->turn_bus(BUSDIR::READ);
                init_test->send_commands();
                delete init_test;
            }
            cout << "of bank " << bank << endl;
        }
        if (t_name == TEST){
            PowerTest * init_test = new PowerTest(sandbox);
            DramAddr * da = new DramAddr();
            init_test->read_entire_row(da);
            init_test->send_commands();
            init_test->read_back();
            delete init_test;
            return 0;
        }
    }
    else if (t_name == IDD7) {
        PowerTest * idd7 = new PowerTest(sandbox);
        idd7->turn_bus(BUSDIR::WRITE);
        idd7->idd7_init();
        idd7->turn_bus(BUSDIR::READ);
        idd7->send_commands();
        delete idd7;
    }

    /////////////////////////////////////////////////////////
    // OTHER IDD TESTS
    ////////////////////////////////////////////////////////
    cout << "Creating the loop instructions..." << endl;
    PowerTest * idd_test = new PowerTest(sandbox);

    idd_test->end_of_init();

    // my_dimm->dump_latencies();
    switch(t_name) {
        // Datasheet defined procedures
        case IDD0  : idd_test->idd0_datasheet_loop(my_dimm->latencies);  break;
        case IDD1  : idd_test->idd1_datasheet_loop(my_dimm->latencies);  break;
        case IDD2N : idd_test->idd2n_datasheet_loop(); break;
        case IDD3N : idd_test->idd3n_datasheet_loop(); break;
        case IDD4R : idd_test->idd4r_loop(0); break;
        case IDD4W : idd_test->idd4w_loop(0,0x33); break;
        case IDD2P : idd_test->idd2p_loop(); break;
        case IDD2Q : idd_test->idd2q_loop(); break;
        case IDD3P : idd_test->idd3p_loop(); break;
        case IDD6  : idd_test->idd6_loop(); break;
        case IDD6ET: idd_test->idd6et_loop(); break;
        case IDD7  : idd_test->idd7_loop(); break;
        case IDD8  : idd_test->idd8_loop(); break;
	case SUMITFCFS : idd_test->sumitfcfs_loop(); break;
	case SUMITFRFCFS : idd_test->sumitfrfcfs_loop(); break;
	case SUMITAHB : idd_test->sumitahb_loop(); break;
	case SUMITFRFCFSPRIORHIT : idd_test->sumitfrfcfspriorhit_loop(); break;
        case IDD5B : 
            uint tRFC = DEF_TRFC;
            if (my_dimm->latencies.find("tRFC") != my_dimm->latencies.end())
                tRFC = my_dimm->latencies.find("tRFC")->second;
            idd_test->idd5b_loop(tRFC); 
        break;
    }

    idd_test->send_commands();
    cout << "Command sequence has been sent to the FPGA." << endl;
    if (sandbox == 1){
        idd_test->generate_loop_trace(test_name_str);
    }
	
    uint rbuf[32];
    int num_recv = 0 ;
    for(int u=1;u<=80;u++){
        for(int a=0;a<16;a++)rbuf[a]=0;
    	num_recv = fpga_recv(idd_test->fpga, 0, (void*)rbuf, 32, 1000);
    	if (num_recv != 16){
     		   detail_print(RED "Received %d words instead of 16.\n" RESET, num_recv);
		   cout<<"\nReceived "<<num_recv<<" words instead of 16 in the "<<u<<"th reading iteration\n";
     		   return -1 ;
   		 }	
  	if(1){
		   cout<<"\nPrinting what was recieved "<<u<<"th :";
   		   for(int y=0;y<16;y++){cout<<std::bitset<32>(rbuf[y])<<" ";rbuf[y]=0;}
		   cout<<"\n";
		}
    }

delete idd_test;

    return 0;
}

void print_help(int argc){
    cout << endl;
    cout << "Usage: ./bin/iddtest [dimm_label] [test_name]" << endl;
    cout << endl;
    cout << "  dimm_label is required. Use sandbox as dimm name for sandbox mode." << endl;
    cout << endl;
    cout << "  test_name is required." << endl;
    cout << "    stop   : Sends stop looping command in a special way!" << endl;
    cout << "             This should only be used if the FPGA is in looping state." << endl;
    cout << "    iddx   : Runs the procedure defined in datasheets for the corresponding IDD Test" <<endl;
    cout << endl;
}

test_name arg_test_name_match(string s){
    for(uint i = 0; i < s.length(); ++i) {
        s[i] = tolower(s[i]);
    }

    string tests [NUM_TESTS];
    tests[STOP]  = "stop";
    tests[IDD0]  = "idd0";
    tests[IDD1]  = "idd1";
    tests[IDD2N] = "idd2n";
    tests[IDD3N] = "idd3n";
    tests[IDD4R] = "idd4r";
    tests[IDD4W] = "idd4w";
    tests[TEST]  = "test";
    tests[IDD2P] = "idd2p";
    tests[IDD2Q] = "idd2q";
    tests[IDD3P] = "idd3p";
    tests[IDD6]  = "idd6";
    tests[IDD6ET]= "idd6et";
    tests[IDD7]  = "idd7";
    tests[IDD8]  = "idd8";
    tests[IDD5B] = "idd5b";
    tests[SUMITFCFS] = "sumitfcfs";
    tests[SUMITFRFCFS] = "sumitfrfcfs";
    tests[SUMITAHB] = "sumitahb";
    tests[SUMITFRFCFSPRIORHIT] ="sumitfrfcfspriorhit";

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
