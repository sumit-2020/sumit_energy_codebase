#include "main.h"

using namespace std;

int main(int argc, char *argv[]) {

  // Parse the arguments
  string dimm("");
  string test("");
  string addr("");
  uint row(1200);
  uint8_t pattern1(0x00);
  uint8_t pattern2(0xAA);
  string operation("");
  int sandbox(0);
  int bank;
  int num_banks;
  int num_cols;

  if (argc < 3){
    print_help();
    exit(EXIT_FAILURE);
  }
  else {
    dimm = argv[1];
    test = argv[2];
    addr = argv[3];
    if (argc > 4){
      row = atoi(argv[4]);
      if (argc > 5){
        pattern1 = atoi(argv[5]);
        if (argc > 6){
          pattern2 = atoi(argv[6]);
          if (argc > 7){
            operation = argv[7];
            if (argc > 8){
              sandbox = atoi(argv[8]);
              if (sandbox == 1){
                cout << "The commands will not be sent to the FPGA." <<endl;
              }
            }
          }
        }
      }
    }
  }

  test_t test_type = which_test(test);
  addr_t addr_set  = which_addr(addr);
  operation_t op_type = which_operation(operation);

  // Stop Looping
  if (test_type == STOP) {
    cout << "Stopping the loop..." <<endl;
    Test * stop_proc = new Test(sandbox);
    stop_proc->stop_looping();
    stop_proc->send_commands(false); // Don't send SENDTR
    stop_proc->read_back();
    delete stop_proc;
    return 0;
  }

  // Initialize the Row
  // cout << "Initializing the page..." <<endl;
  uint8_t patterns[2] = {pattern1,pattern2};
  uint    cols[4]     = {  00,  16,  32, 48};
  uint    banks[4]    = {  00,  01,  02, 03};

  switch (addr_set) {
    case (R01):
      banks[0] = 0; banks[1] = 1; banks[2] = 2; banks[3] = 3;
      cols[0]  = 0; cols[1]  = 8; cols[2]  = 16; cols[3]  = 24;
      break;

    case (R02):
      banks[0] = 0; banks[1] = 1; banks[2] = 2; banks[3] = 3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 48;
      break;

    case (R03):
      banks[0] = 0; banks[1] =  1; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 128; cols[2]  = 256; cols[3]  = 384;
      break;

    case (R04):
      banks[0] = 0; banks[1] =  1; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 232; cols[2]  = 464; cols[3]  = 696;
      break;

    case (R05):
      banks[0] = 0; banks[1] = 2; banks[2] = 4; banks[3] = 6;
      cols[0]  = 0; cols[1]  = 8; cols[2]  = 16; cols[3]  = 24;
      break;

    case (R06):
      banks[0] = 0; banks[1] = 2; banks[2] = 4; banks[3] = 6;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 48;
      break;

    case (R07):
      banks[0] = 0; banks[1] =  2; banks[2] =  4; banks[3] =  6;
      cols[0]  = 0; cols[1]  = 128; cols[2]  = 256; cols[3]  = 384;
          break;

    case (R08):
      banks[0] = 0; banks[1] =  2; banks[2] =  4; banks[3] =  6;
      cols[0]  = 0; cols[1]  = 232; cols[2]  = 464; cols[3]  = 696;
      break;

    case (R09):
      banks[0] = 0; banks[1] = 3; banks[2] = 6; banks[3] = 9;
      cols[0]  = 0; cols[1]  = 8; cols[2]  = 16; cols[3]  = 24;
      break;

    case (R10):
      banks[0] = 0; banks[1] = 3; banks[2] = 6; banks[3] = 9;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 48;
      break;

    case (R11):
      banks[0] = 0; banks[1] =  3; banks[2] =  6; banks[3] =  9;
      cols[0]  = 0; cols[1]  = 128; cols[2]  = 256; cols[3]  = 384;
      break;

    case (R12):
      banks[0] = 0; banks[1] =  3; banks[2] =  6; banks[3] =  9;
      cols[0]  = 0; cols[1]  = 232; cols[2]  = 464; cols[3]  = 696;
      break;

    case (R13):
      banks[0] = 1; banks[1] = 3; banks[2] = 5; banks[3] = 7;
      cols[0]  = 0; cols[1]  = 8; cols[2]  = 16; cols[3]  = 24;
      break;

    case (R14):
      banks[0] = 1; banks[1] = 3; banks[2] = 5; banks[3] = 7;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 48;
      break;

    case (R15):
      banks[0] = 1; banks[1] =  3; banks[2] =  5; banks[3] =  7;
      cols[0]  = 0; cols[1]  = 128; cols[2]  = 256; cols[3]  = 384;
      break;

    case (R16):
      banks[0] = 1; banks[1] =  3; banks[2] =  5; banks[3] =  7;
      cols[0]  = 0; cols[1]  = 232; cols[2]  = 464; cols[3]  = 696;
      break;

    case (R17):
      banks[0] = 5; banks[1] = 6; banks[2] = 7; banks[3] = 8;
      cols[0]  = 0; cols[1]  = 8; cols[2]  = 16; cols[3]  = 24;
      break;

    case (R18):
      banks[0] = 5; banks[1] = 6; banks[2] = 7; banks[3] = 8;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 48;
      break;

    case (R19):
      banks[0] = 5; banks[1] =  6; banks[2] =  7; banks[3] =  8;
      cols[0]  = 0; cols[1]  = 128; cols[2]  = 256; cols[3]  = 384;
      break;

    case (R20):
      banks[0] = 5; banks[1] =  6; banks[2] =  7; banks[3] =  8;
      cols[0]  = 0; cols[1]  = 232; cols[2]  = 464; cols[3]  = 696;
      break;

    case (D00):
      banks[0] = 0; banks[1] =  0; banks[2] =  0; banks[3] =  0;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D01):
      banks[0] = 1; banks[1] =  1; banks[2] =  1; banks[3] =  1;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D02):
      banks[0] = 2; banks[1] =  2; banks[2] =  2; banks[3] =  2;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D03):
      banks[0] = 3; banks[1] =  3; banks[2] =  3; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D04):
      banks[0] = 4; banks[1] =  4; banks[2] =  4; banks[3] =  4;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D05):
      banks[0] = 5; banks[1] =  5; banks[2] =  5; banks[3] =  5;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D06):
      banks[0] = 6; banks[1] =  6; banks[2] =  6; banks[3] =  6;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D07):
      banks[0] = 7; banks[1] =  7; banks[2] =  7; banks[3] =  7;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D08):
      banks[0] = 0; banks[1] =  1; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D09):
      banks[0] = 1; banks[1] =  2; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D10):
      banks[0] = 0; banks[1] =  2; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D11):
      banks[0] = 2; banks[1] =  3; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D12):
      banks[0] = 0; banks[1] =  3; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D13):
      banks[0] = 3; banks[1] =  4; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D14):
      banks[0] = 0; banks[1] =  4; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D15):
      banks[0] = 4; banks[1] =  5; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D16):
      banks[0] = 0; banks[1] =  5; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D17):
      banks[0] = 4; banks[1] =  6; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D18):
      banks[0] = 0; banks[1] =  6; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D19):
      banks[0] = 6; banks[1] =  7; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (D20):
      banks[0] = 0; banks[1] =  7; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (B51):
      banks[0] = 5; banks[1] =  1; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (B73):
      banks[0] = 7; banks[1] =  3; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;

    case (B34):
      banks[0] = 3; banks[1] =  4; banks[2] =  2; banks[3] =  3;
      cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 64;
      break;
  }

  DramAddr * da0;
  DramAddr * da1;
  DramAddr * da2;
  DramAddr * da3;

  switch (test_type) {
    case SBDC:
      da0 = new DramAddr(cols[0], row, banks[0]);
      da1 = new DramAddr(cols[1], row, banks[0]);
      da2 = new DramAddr(cols[1], row, banks[0]);
      da3 = new DramAddr(cols[1], row, banks[1]);
      num_banks = 1;
      num_cols  = 2;
      break;

    case DBDC:
      da0 = new DramAddr(cols[0], row, banks[0]);
      da1 = new DramAddr(cols[1], row, banks[1]);
      da2 = new DramAddr(cols[0], row, banks[0]);
      da3 = new DramAddr(cols[1], row, banks[1]);
      num_banks = 2;
      num_cols  = 2;
      break;

    case BICI: //Bank Interleaving w/ Column Interleaving
      da0 = new DramAddr(cols[0], row, banks[0]);
      da1 = new DramAddr(cols[0], row, banks[1]);
      da2 = new DramAddr(cols[1], row, banks[0]);
      da3 = new DramAddr(cols[1], row, banks[1]);
      num_banks = 2;
      num_cols  = 2;
      break;

    case SBSC:
      da0 = new DramAddr(cols[0], row, banks[0]);
      da1 = new DramAddr(cols[0], row, banks[0]);
      da2 = new DramAddr(cols[0], row, banks[0]);
      da3 = new DramAddr(cols[0], row, banks[0]);
      num_banks = 1;
      num_cols  = 1;
      break;
  }

  Test * my_test = new Test(sandbox);

  ////////////////////////////////////////////////////////////
  //  INITIALIZATION PART
  ////////////////////////////////////////////////////////////
  my_test->turn_bus(BUSDIR::WRITE);

  // Activate all the banks
  for (int bi = 0 ; bi < num_banks ; bi++ ){
    my_test->cq->insert(genRowCMD(row, banks[bi], MC_CMD::ACT));
    my_test->cq->insert(genWaitCMD(DEF_TRCD));
  }

  // Initialize the columns with the given patterns
  for (int ci = 0 ; ci < num_cols ; ci++){
    for (int bi = 0 ; bi < num_banks ; bi++){
      if (test_type == BICI)
        my_test->cq->insert(genWriteCMD(cols[ci], banks[bi], patterns[bi], AUTO_PRECHARGE::NO_AP));
      else 
        my_test->cq->insert(genWriteCMD(cols[ci], banks[bi], patterns[ci], AUTO_PRECHARGE::NO_AP));
      my_test->cq->insert(genWaitCMD(DEF_TCL + DEF_TBURST));
    }
  }

  // Precharge all the banks
  for (int bi = 0 ; bi < num_banks ; bi++ ){
    my_test->cq->insert(genRowCMD(row, banks[bi], MC_CMD::PRE));
    my_test->cq->insert(genWaitCMD(DEF_TRP));
  }

  // Finalize the initialization part
  my_test->turn_bus(BUSDIR::READ);

  ////////////////////////////////////////////////////////////
  //  LOOPING PART
  ////////////////////////////////////////////////////////////
  uint unit_cmd_cnt = 0;
  if (op_type == READ)
    unit_cmd_cnt = 2;
  
  else if (op_type == WRITE)
    unit_cmd_cnt = 6;

  for (int i = 0 ; i < num_banks ; i++ ){
    my_test->cq->insert(genRowCMD(row, banks[i], MC_CMD::ACT));
  }
  my_test->cq->insert(genWaitCMD(DEF_TRCD));

  switch (test_type) {
    case SBDC:
    case DBDC:
      while(my_test->cq->size < LOOP_SIZE + INIT_SIZE - num_banks - unit_cmd_cnt*2 - 3){
        switch (op_type){
          case READ: 
            my_test->read_column(da0);
            my_test->read_column(da1);
            break;
          case WRITE:
            my_test->write_column_turnbus(da0,patterns[0]);
            my_test->write_column_turnbus(da1,patterns[1]);
            break;
        }
      }
    break;

    case BICI:
      while(my_test->cq->size < LOOP_SIZE + INIT_SIZE - num_banks - unit_cmd_cnt*4 - 3){
        switch (op_type){
          case READ:
            my_test->read_column(da0);
            my_test->read_column(da1);
            my_test->read_column(da2);
            my_test->read_column(da3);
            break;
          case WRITE:
            my_test->write_column_turnbus(da0, patterns[0]);
            my_test->write_column_turnbus(da1, patterns[1]);
            my_test->write_column_turnbus(da2, patterns[0]);
            my_test->write_column_turnbus(da3, patterns[1]);
            break;
        }
      }
      break;

    case SBSC:
      while(my_test->cq->size < LOOP_SIZE + INIT_SIZE - num_banks - unit_cmd_cnt - 3){
        switch (op_type){
          case READ:
            my_test->read_column(da0);
            break;
          case WRITE:
            my_test->write_column_turnbus(da0,patterns[0]);
            break;
        }
      }
      break;
  }


  for (int i = 0 ; i < num_banks ; i++ ){
    my_test->cq->insert(genRowCMD(row, banks[i], MC_CMD::PRE));
  }
  my_test->cq->insert(genWaitCMD(DEF_TRP));
  my_test->send_commands();

  delete da0;
  delete da1;
  delete da2;
  delete da3;
  delete my_test;
  return 0;
}

operation_t which_operation(string operation)
{
  string op_strs [NUM_OPS];
  op_strs[WRITE] = "wr,write";
  op_strs[READ]  = "rd,read";

  for (int i = ZERO ; i < NUM_OPS ; i++){
      cout << operation << " is searched in " << op_strs[i] << endl;
      int loc = op_strs[i].find(operation);
      // cout << test << " is searched in " << test_strs[i] << endl;
      if ( loc != -1 ){
          return (operation_t) i;
      }
  }
  cout << "[ERROR] Operation is not defined!" << endl;
  print_help();
  exit(EXIT_FAILURE);
}

test_t which_test(string test)
{
  string test_strs [NUM_TESTS];
  test_strs[STOP] = "stop,stoploop";
  test_strs[SBSC] = "sbsc,singlebanksinglecol";
  test_strs[SBDC] = "sbdc,singlebankdoublecol";
  test_strs[DBDC] = "dbdc,doublebankdoublecol";
  test_strs[BICI] = "bici,bankintercolumninter";  

  for (int i = ZERO ; i < NUM_TESTS ; i++){
      cout << test << " is searched in " << test_strs[i] << endl;
      int loc = test_strs[i].find(test);
      // cout << test << " is searched in " << test_strs[i] << endl;
      if ( loc != -1 ){
          return (test_t) i;
      }
  }
  cout << "[ERROR] Test is not defined!" << endl;
  print_help();
  exit(EXIT_FAILURE);
}

addr_t which_addr(string addr_set)
{
  string addr_strs [NUM_ADDR_SETS];
  addr_strs[R01]              = "r1,r01";
  addr_strs[R02]              = "r2,r02";
  addr_strs[R03]              = "r3,r03";
  addr_strs[R04]              = "r4,r04";
  addr_strs[R05]              = "r5,r05";
  addr_strs[R06]              = "r6,r06";
  addr_strs[R07]              = "r7,r07";
  addr_strs[R08]              = "r8,r08";
  addr_strs[R09]              = "r9,r09";
  addr_strs[R10]              = "r10";
  addr_strs[R11]              = "r11";
  addr_strs[R12]              = "r12";
  addr_strs[R13]              = "r13";
  addr_strs[R14]              = "r14";
  addr_strs[R15]              = "r15";
  addr_strs[R16]              = "r16";
  addr_strs[R17]              = "r17";
  addr_strs[R18]              = "r18";
  addr_strs[R19]              = "r19";
  addr_strs[R20]              = "r20";
  addr_strs[D00]              = "d0,d00";
  addr_strs[D01]              = "d1,d01";
  addr_strs[D02]              = "d2,d02";
  addr_strs[D03]              = "d3,d03";
  addr_strs[D04]              = "d4,d04";
  addr_strs[D05]              = "d5,d05";
  addr_strs[D06]              = "d6,d06";
  addr_strs[D07]              = "d7,d07";
  addr_strs[D08]              = "d8,d08";
  addr_strs[D09]              = "d9,d09";
  addr_strs[D10]              = "d10";
  addr_strs[D11]              = "d11";
  addr_strs[D12]              = "d12";
  addr_strs[D13]              = "d13";
  addr_strs[D14]              = "d14";
  addr_strs[D15]              = "d15";
  addr_strs[D16]              = "d16";
  addr_strs[D17]              = "d17";
  addr_strs[D18]              = "d18";
  addr_strs[D19]              = "d19";
  addr_strs[D20]              = "d20";
  addr_strs[B51]              = "b51";
  addr_strs[B73]              = "b73";
  addr_strs[B34]              = "b34";

  for (int i = ZERO ; i < NUM_ADDR_SETS ; i++){
      int loc = addr_strs[i].find(addr_set);
      if ( loc != -1 ){
          return (addr_t) i;
      }
  }
  cout << "[ERROR] Address set is not defined!" << endl;
  print_help();
  exit(EXIT_FAILURE);
}

void print_help(){
  cout << "Proper Usage: ";
  cout << "./bin/toggletest [dimm] [test] [addr] [row] [pattern1] [pattern2] [operation] [sandbox]" << endl;
  cout << "[dimm     ] (DIMM Name) * : (ex) micronb60" << endl;
  cout << "[test     ] (Test Name) * : stop, sbsc, sbdc, dbdc, bici" << endl;
  cout << "[addr     ] (Addr Conf) * : r1-20, d0-20, b51, b73, b34" << endl;
  cout << "[row      ] (Row)         : (ex) 0 , 1 , 2 , ... , 32768 " << endl;
  cout << "[pattern1 ] (Addr0 Data)  : (ex) 0 , 170 , 255 (default: 0x00)" << endl;
  cout << "[pattern2 ] (Addr1 Data)  : (ex) 0 , 170 , 255 (default: 0xAA)" << endl;
  cout << "[operation] (Operation)   : (ex) read, write (default: read)" << endl;
  cout << "[sandbox  ] (Sandbox Mode): Default: 0. If set, doesn't run commands on FPGA" << endl;
  cout << "* Compulsory argument";
}
