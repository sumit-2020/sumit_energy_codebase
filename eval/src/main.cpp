#include "power_test.h"
#include "dimm.h"
#include "node_state.h"
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <ctime>
#include <math.h>

using namespace std;

enum test_name : uint{
  INIT, STOP_LOOP,
  FIXED_PERIOD_RD, FIXED_PERIOD_WR,
  BACK_TO_BACK_RD, BACK_TO_BACK_WR,
  NUM_TESTS
};

test_name arg_test_name_match(string test_name_str);

int main(int argc, char *argv[]) {
  clock_t begin = clock();

  // parse arguments
  string dimm_str("");
  string test_name_str("");
  uint8_t pattern(255);
  int bank(0);
  int row(0);
  int num_ops(0);
  int interval(0);
  int sandbox(0);

  if (argc < 3) exit(EXIT_FAILURE);
  else {
      dimm_str = argv[1];
      test_name_str = argv[2];
      if (argc > 3) pattern = atoi(argv[3]);
      if (argc > 4) bank = atoi(argv[4]);
      if (argc > 5) row = atoi(argv[5]);
      if (argc > 6) num_ops = atoi(argv[6]);
      if (argc > 7) interval = atoi(argv[7]);
      if (argc > 8) sandbox = atoi(argv[8]);

      if (sandbox == 1){
          cout << "The commands will not be sent to the FPGA." <<endl;
      }
  }

  test_name t_name = arg_test_name_match(test_name_str);

  //////////////////////////////////
  // Calculating some parameters
  /////////////////////////////////
  uint step, max_num_ops = 510;
  num_ops = (num_ops > max_num_ops) ? max_num_ops : num_ops;
  if (num_ops > 0){
      step = NUM_CACHELINES / num_ops;
      step = (step < 2)? 2 : step;
  }
  int post_wait = interval / 2;
  post_wait = (post_wait < 3)? 3 : post_wait;
  int pre_wait = interval - post_wait - 1;
  // Put the pre-wait and post-wait here.

  //////////////////////////////////
  // Generating the command sequences
  //////////////////////////////////
  switch (t_name) {
    case STOP_LOOP: {  // Send single stop command to exit from the loop
        Test * my_test = new Test(sandbox);
        my_test->stop_looping();
        my_test->send_commands(false); // Don't send SENDTR
        my_test->read_back();
        delete my_test;
        break;
    }
    case INIT: {
        DramAddr * da = new DramAddr(row,bank);
        Test * init_test = new Test(sandbox);
        init_test->turn_bus(BUSDIR::WRITE);
        init_test->write_entire_row(da, pattern);
        init_test->turn_bus(BUSDIR::READ);
        init_test->send_commands();
        delete init_test;
        break;
    }
    case FIXED_PERIOD_RD: {
        PowerTest * testFRD = new PowerTest(sandbox);
        testFRD->end_of_init();
        testFRD->cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
        testFRD->cq->insert(genWaitCMD(DEF_TRCD-1));
        if (num_ops > 0 ){
            for (uint i = 0; i < num_ops ; i++){
                int col = (i * step) % NUM_CACHELINES;
                if (pre_wait > 0) testFRD->cq->insert(genWaitCMD(pre_wait));
                testFRD->cq->insert(genReadCMD(col, bank, AUTO_PRECHARGE::NO_AP));
                testFRD->cq->insert(genWaitCMD(post_wait));
            }
        }
        else {
            testFRD->cq->insert(genWaitCMD(interval));
        }
        testFRD->cq->insert(genRowCMD(row, bank, MC_CMD::PRE));
        testFRD->cq->insert(genWaitCMD(DEF_TRP-1));
        testFRD->send_commands();
        delete testFRD;
        break;
    }
    case BACK_TO_BACK_RD: {
        PowerTest * testBRD = new PowerTest(sandbox);
        testBRD->end_of_init();
        testBRD->cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
        testBRD->cq->insert(genWaitCMD(DEF_TRCD-1));
        if (num_ops > 0 ){
            for (int i = 0; i < num_ops ; i++){
                int col = (i * step) % NUM_CACHELINES;
                testBRD->cq->insert(
                  genReadCMD(col, bank, AUTO_PRECHARGE::NO_AP)
                );
                testBRD->cq->insert(genWaitCMD(3));
            }
        }
        else {
            testBRD->cq->insert(genWaitCMD(interval));
        }
        testBRD->cq->insert(genRowCMD(row, bank, MC_CMD::PRE));
        testBRD->cq->insert(genWaitCMD(DEF_TRP-1));
        testBRD->send_commands();
        delete testBRD;
        break;
    }
    case FIXED_PERIOD_WR: {
        PowerTest * testFWR = new PowerTest(sandbox);
        testFWR->end_of_init();
        testFWR->turn_bus(BUSDIR::WRITE);
        testFWR->cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
        testFWR->cq->insert(genWaitCMD(DEF_TRCD-1));
        if (num_ops > 0 ){
            for (uint i = 0; i < num_ops ; i++){
                int col = (i * step) % NUM_CACHELINES;
                if (pre_wait > 0) testFWR->cq->insert(genWaitCMD(pre_wait));
                testFWR->cq->insert(
                  genWriteCMD(col, bank, pattern, AUTO_PRECHARGE::NO_AP)
                );
                testFWR->cq->insert(genWaitCMD(post_wait));
            }
        }
        else {
            testFWR->cq->insert(genWaitCMD(interval));
        }
        testFWR->cq->insert(genRowCMD(row, bank, MC_CMD::PRE));
        testFWR->cq->insert(genWaitCMD(DEF_TRP-1));
        testFWR->turn_bus(BUSDIR::READ);
        testFWR->send_commands();
        delete testFWR;
        break;
    }
    case BACK_TO_BACK_WR: {
        PowerTest * testBWR = new PowerTest(sandbox);
        testBWR->end_of_init();
        testBWR->turn_bus(BUSDIR::WRITE);
        testBWR->cq->insert(genRowCMD(row, bank, MC_CMD::ACT));
        testBWR->cq->insert(genWaitCMD(DEF_TRCD-1));
        if (num_ops > 0 ){
            for (int i = 0; i < num_ops ; i++){
                int col = (i * step) % NUM_CACHELINES;
                testBWR->cq->insert(
                  genReadCMD(col, bank, AUTO_PRECHARGE::NO_AP)
                );
                testBWR->cq->insert(genWaitCMD(3));
            }
        }
        else {
            testBWR->cq->insert(genWaitCMD(interval));
        }
        testBWR->cq->insert(genRowCMD(row, bank, MC_CMD::PRE));
        testBWR->cq->insert(genWaitCMD(DEF_TRP-1));
        testBWR->turn_bus(BUSDIR::READ);
        testBWR->send_commands();
        delete testBWR;
        break;
    }
  }// switch t_name

  clock_t end = clock();
  return 0;
}

test_name arg_test_name_match(string s){
    for(uint i = 0; i < s.length(); ++i) {
        s[i] = tolower(s[i]);
    }

    string tests [NUM_TESTS];
    tests[INIT] = "init";
    tests[STOP_LOOP] = "stoploop";
    tests[FIXED_PERIOD_RD] = "fixedperiodrd";
    tests[BACK_TO_BACK_RD] = "backtobackrd";
    tests[FIXED_PERIOD_WR] = "fixedperiodwr";
    tests[BACK_TO_BACK_WR] = "backtobackwr";

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
