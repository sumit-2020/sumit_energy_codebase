#include "main.h"

using namespace std;

int main(int argc, char *argv[]) {

  // Parse the arguments 
  string dimm("");
  string test("");
  uint row(0);
  uint bank(0);
  int sandbox(0);

  if (argc < 3){
    cout << "Not enough arguments" << endl;
    exit(EXIT_FAILURE);
  }
  else {
    test = argv[1];
    if (argc > 2){
      bank = atoi(argv[2]);
      if (argc > 3){
        row = atoi(argv[3]);
        if (argc > 4){
          sandbox = atoi(argv[4]);
          if (sandbox == 1){
            cout << "The commands will not be sent to the FPGA." <<endl;
          }
        }
      }
    }
  }

  test_t test_type = which_test(test);
  Test * my_test = new Test(sandbox);
  my_test->buff_wait(3);

  DramAddr * da = new DramAddr(row,bank);
  switch (test_type) {
    case (ACT) :
      my_test->act(da, true);
    break;

    case (PRE) :
      my_test->pre(da, true);
    break;

    case (STOP) :
      my_test->stop_looping();
    break;
  }

  my_test->send_commands();
  my_test->read_back();

  delete my_test;
  return 0;
}

test_t which_test(string test)
{
  string test_strs [NUM_TESTS];
  test_strs[STOP] = "stop,stoploop";
  test_strs[ACT]  = "act,activate";
  test_strs[PRE]  = "pre,precharge";

  for (int i = ZERO ; i < NUM_TESTS ; i++){
      int loc = test_strs[i].find(test);
      if ( loc != -1 ){
          return (test_t) i;
      }
  }
  cout << "[ERROR] Test is not defined!" << endl;
  exit(EXIT_FAILURE);
}