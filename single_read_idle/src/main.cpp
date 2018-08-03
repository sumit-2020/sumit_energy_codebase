#include "main.h"

using namespace std;

int main(int argc, char *argv[]) {

  // Parse the arguments
  string dimm("");
  string test_str("");
  string addr_str("");
  uint row(1200);
  uint8_t pattern1(0x00);
  uint8_t pattern2(0x00);
  int sandbox(0);

  if (argc < 3){
    print_help();
    exit(EXIT_FAILURE);
  }
  else {
    dimm = argv[1];
    test_str = argv[2];
    addr_str = argv[3];
    if (argc > 4){
      row = atoi(argv[4]);
      if (argc > 5){
        pattern1 = atoi(argv[5]);
        if (argc > 6){
          pattern2 = atoi(argv[6]);
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

  test_t test_type = which_test(test_str);
  addr_arr test_addrs_arr = parse_addresses(addr_str);

  // Stop Looping
  if (test_type == STOP) {
    cout << "Stopping the loop..." <<endl;
    Test * stop_proc = new Test(sandbox);
    stop_proc->stop_looping();
    stop_proc->send_commands(false); // Don't send SENDTR
    stop_proc->read_back();
    delete stop_proc;

    Test * bus_turner = new Test(sandbox);
    bus_turner->turn_bus(BUSDIR::READ);
    bus_turner->send_commands();
    delete bus_turner;

    return 0;
  }
  else {
    vector<int> active_banks; 
    vector<DramAddr *> da_arr; //[test_addrs_arr.size()];
    cout << "Access Pattern: ";
    int index = 0;
    for (addr_arr::iterator it = test_addrs_arr.begin(); it != test_addrs_arr.end() ; it++){
      cout << "Bank" << (*it).first << "Col" << (*it).second << " ";
      da_arr.push_back(new DramAddr((*it).second,row,(*it).first));
      vector<int>::iterator active_banks_it = find (active_banks.begin(), active_banks.end(), (*it).first);
      if (active_banks_it == active_banks.end()){
        active_banks.push_back((*it).first);
      }
    }
    cout << endl;

    PowerTest * my_test = new PowerTest(sandbox);
    
    // Initialize the banks
    bool is_pattern1 = true;
    my_test->turn_bus(BUSDIR::WRITE);
    for (vector<int>::iterator active_banks_it = active_banks.begin() ; active_banks_it != active_banks.end() ; active_banks_it++){
      my_test->cq->insert(genRowCMD(row, *active_banks_it, MC_CMD::ACT));
      cout << "Activating bank " << *active_banks_it << endl;
    }
    my_test->cq->insert(genWaitCMD(DEF_TRCD));

    for (vector<DramAddr *>::iterator it = da_arr.begin(); it != da_arr.end(); it++){
      if (is_pattern1){
        my_test->write_column(*it, pattern1);
        is_pattern1 = false;
      }
      else{
        my_test->write_column(*it, pattern2);
        is_pattern1 = true;
      }
    }
    for (vector<int>::iterator active_banks_it = active_banks.begin() ; active_banks_it != active_banks.end() ; active_banks_it++){
      my_test->cq->insert(genRowCMD(row, *active_banks_it, MC_CMD::PRE));
      cout << "Precharging bank " << *active_banks_it << endl;
    }
    my_test->cq->insert(genWaitCMD(DEF_TRP));
    my_test->turn_bus(BUSDIR::READ);
    my_test->end_of_init();

    // Loop Starts Here

    // Activate
    for (vector<int>::iterator active_banks_it = active_banks.begin() ; active_banks_it != active_banks.end() ; active_banks_it++){
      my_test->cq->insert(genRowCMD(row, *active_banks_it, MC_CMD::ACT));
      cout << "Activating bank " << *active_banks_it << endl;
    }
    my_test->cq->insert(genWaitCMD(DEF_TRCD));

    // Read if read is enabled
    if (test_type == ACTRDPREWAIT || test_type == ACTRDWAITPRE){
      for (vector<DramAddr *>::iterator it = da_arr.begin(); it != da_arr.end(); it++){
        my_test->read_column(*it);
      }
    }

    // Write if write is enabled
    if (test_type == ACTWRPREWAIT || test_type == ACTWRWAITPRE){
      my_test->turn_bus(BUSDIR::WRITE);
      is_pattern1 = true;
      for (vector<DramAddr *>::iterator it = da_arr.begin(); it != da_arr.end(); it++){
        if (is_pattern1){
          my_test->write_column(*it, pattern1);
          is_pattern1 = false;
        }
        else {
          my_test->write_column(*it, pattern2);
          is_pattern1 = true;
        }
      }
      my_test->turn_bus(BUSDIR::READ);
    }

    // Precharge first, wait later
    if (test_type == ACTRDPREWAIT || test_type == ACTPREWAIT){
      for (vector<int>::iterator active_banks_it = active_banks.begin() ; active_banks_it != active_banks.end() ; active_banks_it++){
        my_test->cq->insert(genRowCMD(row, *active_banks_it, MC_CMD::PRE));
        cout << "Precharging bank " << *active_banks_it << endl;
      }
      while (my_test->cq->size < LOOP_SIZE + INIT_SIZE - 2){
        my_test->cq->insert(genWaitCMD(1023));
      }
      cout << "Wait" << endl;
    }

    // Wait first, precharge later
    if (test_type == ACTRDWAITPRE || test_type == ACTWAITPRE){
      while (my_test->cq->size < LOOP_SIZE + INIT_SIZE - active_banks.size() - 3){
        my_test->cq->insert(genWaitCMD(1023));
      }
      cout << "Wait" << endl;
      for (vector<int>::iterator active_banks_it = active_banks.begin() ; active_banks_it != active_banks.end() ; active_banks_it++){
        my_test->cq->insert(genRowCMD(row, *active_banks_it, MC_CMD::PRE));
        cout << "Precharging bank " << *active_banks_it << endl;
      }
      my_test->cq->insert(genWaitCMD(DEF_TRP));
    }
    my_test->send_commands();
    delete my_test;
  }
  
  return 0;
}

test_t which_test(string test)
{
  string test_strs [NUM_TESTS];
  test_strs[STOP] = "stop,stoploop";
  test_strs[ACTRDWAITPRE] = "actrdwaitpre";
  test_strs[ACTRDPREWAIT] = "actrdprewait";
  test_strs[ACTWRWAITPRE] = "actwrwaitpre";
  test_strs[ACTWRPREWAIT] = "actwrprewait";
  test_strs[ACTPREWAIT] = "actprewait";
  test_strs[ACTWAITPRE] = "actwaitpre";

  for (int i = ZERO ; i < NUM_TESTS ; i++){
      int loc = test_strs[i].find(test);
      if ( loc != -1 ){
        return (test_t) i;
      }
  }
  cout << "[ERROR] Test is not defined!" << endl;
  print_help();
  exit(EXIT_FAILURE);
}

void print_help(){
  cout << "Proper Usage: ";
  cout << "./bin/idletest [dimm] [test] [addr] [row] [pattern1] [pattern2] [operation] [sandbox]" << endl;
  cout << "[dimm     ] (DIMM Name) * : (ex) micronb60" << endl;
  cout << "[test     ] (Test Name) * : stop, actrdwaitpre, actrdprewait, actprewait, actwaitpre" << endl;
  cout << "[addr_str ] (Addr Conf) * : [bank0]_[col0]-[bank1]_[col1]-..." << endl;
  cout << "[row      ] (Row)         : (ex) 0 , 1 , 2 , ... , 32768 " << endl;
  cout << "[pattern  ] (Data)        : (ex) 0 , 170 , 255 (default: 0x00)" << endl;
  cout << "[sandbox  ] (Sandbox Mode): Default: 0. If set, doesn't run commands on FPGA" << endl;
  cout << "* Compulsory argument" << endl;
}


addr_arr parse_addresses(string addr_str){
  addr_arr my_addr_arr;

  string addr_delimiter = "-";
  string args_delimiter = "_";
  while (addr_str.length() > 0) {
    size_t addr_pos = addr_str.find(addr_delimiter);
    addr_t bank_col;
    string token = addr_str.substr(0, addr_pos);
    size_t args_pos = token.find(args_delimiter);
    bank_col.first = atoi(token.substr( 0, args_pos + 1 ).c_str());
    token.erase(0, args_pos + 1);
    bank_col.second = atoi(token.c_str());
    if (addr_pos != addr_str.npos)
      addr_str.erase(0, addr_pos + 1);
    else
      addr_str="";
    my_addr_arr.push_back(bank_col);
  }

  return my_addr_arr;
}
