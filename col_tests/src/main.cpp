#include "power_test.h"
#include "dimm.h"
#include "node_state.h"
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <ctime>

using namespace std;

enum test_name : uint{
  STOP_LOOP, COL_TEST, NUM_TESTS
};
test_name arg_test_name_match(std::string);

int main(int argc, char *argv[]) {
  clock_t begin = clock();

  // parse arguments
  string dimm_str("");
  string test_name_str("");
  uint8_t pattern(255);
  int bank(0);
  int row(0);
  int column(0);
  int sandbox(0);

  if (argc < 3){
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
                        column = atoi(argv[6]);
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

  //////////////////////////////////
  // STOP LOOP
  //////////////////////////////////

  if (t_name == STOP_LOOP) {
        // Send single stop command to exit from the loop
        Test * my_test = new Test(sandbox);
        my_test->stop_looping();
        my_test->send_commands(false); // Don't send SENDTR
        my_test->read_back();
        delete my_test;

  }// Stop Loop

  else { // run the test
    PowerTest * column_test = new PowerTest(sandbox);
    column_test->cq->insert(genBusDirCMD(BUSDIR::WRITE));
    column_test->cq->insert(genWaitCMD(5));
    column_test->col_init(bank, row, column, pattern);
    column_test->cq->insert(genBusDirCMD(BUSDIR::READ));
    column_test->cq->insert(genWaitCMD(5));
    column_test->end_of_init();
    column_test->col_loop(bank, row, column, pattern);
    column_test->send_commands();

    delete column_test;
  }
  return 0;
}


test_name arg_test_name_match(string s){
    for(uint i = 0; i < s.length(); ++i) {
        s[i] = tolower(s[i]);
    }

    string tests [NUM_TESTS];
    tests[STOP_LOOP] = "stoploop";
    tests[COL_TEST]  = "coltest";

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

