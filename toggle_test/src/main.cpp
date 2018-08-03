#include "main.h"

using namespace std;

int main(int argc, char *argv[]) {

  // Parse the arguments 
  string dimm("");
  string test("");
  uint row(1200);
  uint8_t pattern1(0x00);
  uint8_t pattern2(0xAA);
  uint8_t pattern3(0xFF);
  int sandbox(0);
  int bank;
  
  if (argc < 2){
    cout << "Not enough arguments" << endl;
    exit(EXIT_FAILURE);
  }
  else {
    dimm = argv[1];
    test = argv[2];
    if (argc > 3){
      row = atoi(argv[3]);
      if (argc > 4){
        pattern1 = atoi(argv[4]);
        if (argc > 5){
          pattern2 = atoi(argv[5]);  
          if (argc > 6){
            pattern3 = atoi(argv[6]);
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

  test_t test_type = which_test(test);

  // Stop Looping
  if (test_type == STOP) {
    cout << "Stopping the loop..." <<endl;
    Test * stop_proc = new Test(sandbox);
    stop_proc->stop_looping();
    stop_proc->send_commands(false); // Don't send SENDTR
    stop_proc->read_back();
    delete stop_proc;

    cout << "Closing the pages..." <<endl;
    Test * close_banks = new Test(sandbox);
    close_banks->buff_wait(3);
    if (close_banks->close_open_banks())
      close_banks->send_commands();
    delete close_banks;
    return 0;
  }

  // Initialize the Row
  // cout << "Initializing the page..." <<endl;
  Test * my_test = new Test(sandbox);
  my_test->buff_wait(3);
  uint8_t patterns[3] = {pattern1,pattern2,pattern3};
  uint    cols[4]     = {  00,  16,  32, 48};
  uint    banks[4]    = {  00,  01,  02, 03};

  switch (test_type) {

    case (R1):
    case (R2):
    case (R3):
    case (R4):
    case (R5):
    case (R6):
    case (R7):
    case (R8):
    case (R9):
    case (R10):
    case (R11):
    case (R12):
    case (R13):
    case (R14):
    case (R15):
    case (R16):
    case (R17):
    case (R18):
    case (R19):
    case (R20):
    {
	   if (test_type == R1){
        banks[0] = 0; banks[1] = 1; banks[2] = 2; banks[3] = 3;
		    cols[0]  = 0; cols[1]  = 1; cols[2]  = 2; cols[3]  = 3;
      }
      
      else if (test_type == R2){
        banks[0] = 0; banks[1] = 1; banks[2] = 2; banks[3] = 3;
        cols[0]  = 0; cols[1]  = 2; cols[2]  = 4; cols[3]  = 6;
      }
    
      else if (test_type == R3){
        banks[0] = 0; banks[1] =  1; banks[2] =  2; banks[3] =  3;
        cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 48;
      }
    
      else if (test_type == R4){
        banks[0] =  0; banks[1] =  1; banks[2] =  2; banks[3] =  3;
        cols[0]  =  0; cols[1]  = 29; cols[2]  = 51; cols[3]  = 67;
      }

      else if (test_type == R5){
        banks[0] = 0; banks[1] = 2; banks[2] = 4; banks[3] = 6;
        cols[0]  = 0; cols[1]  = 1; cols[2]  = 2; cols[3]  = 3;
      }
      
      else if (test_type == R6){
        banks[0] = 0; banks[1] = 2; banks[2] = 4; banks[3] = 6;
        cols[0]  = 0; cols[1]  = 2; cols[2]  = 4; cols[3]  = 6;
      }
    
      else if (test_type == R7){
        banks[0] = 0; banks[1] =  2; banks[2] =  4; banks[3] =  6;
        cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 48;
      }
    
      else if (test_type == R8){
        banks[0] = 0; banks[1] =  2; banks[2] =  4; banks[3] =  6;
        cols[0]  = 0; cols[1]  = 29; cols[2]  = 51; cols[3]  = 67;
      }

      else if (test_type == R9){
        banks[0] = 0; banks[1] = 3; banks[2] = 6; banks[3] = 9;
        cols[0]  = 0; cols[1]  = 1; cols[2]  = 2; cols[3]  = 3;
      }
      
      else if (test_type == R10){
        banks[0] = 0; banks[1] = 3; banks[2] = 6; banks[3] = 9;
        cols[0]  = 0; cols[1]  = 2; cols[2]  = 4; cols[3]  = 6;
      }
    
      else if (test_type == R11){
        banks[0] = 0; banks[1] =  3; banks[2] =  6; banks[3] =  9;
        cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 48;
      }
    
      else if (test_type == R12){
        banks[0] = 0; banks[1] =  3; banks[2] =  6; banks[3] =  9;
        cols[0]  = 0; cols[1]  = 29; cols[2]  = 51; cols[3]  = 67;
      }

      else if (test_type == R13){
        banks[0] = 1; banks[1] = 3; banks[2] = 5; banks[3] = 7;
        cols[0]  = 0; cols[1]  = 1; cols[2]  = 2; cols[3]  = 3;
      }
      
      else if (test_type == R14){
        banks[0] = 1; banks[1] = 3; banks[2] = 5; banks[3] = 7;
        cols[0]  = 0; cols[1]  = 2; cols[2]  = 4; cols[3]  = 6;
      }
    
      else if (test_type == R15){
        banks[0] = 1; banks[1] =  3; banks[2] =  5; banks[3] =  7;
        cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 48;
      }
    
      else if (test_type == R16){
        banks[0] = 1; banks[1] =  3; banks[2] =  5; banks[3] =  7;
        cols[0]  = 0; cols[1]  = 29; cols[2]  = 51; cols[3]  = 67;
      }

      else if (test_type == R17){
        banks[0] = 5; banks[1] = 6; banks[2] = 7; banks[3] = 8;
        cols[0]  = 0; cols[1]  = 1; cols[2]  = 2; cols[3]  = 3;
      }
      
      else if (test_type == R18){
        banks[0] = 5; banks[1] = 6; banks[2] = 7; banks[3] = 8;
        cols[0]  = 0; cols[1]  = 2; cols[2]  = 4; cols[3]  = 6;
      }
    
      else if (test_type == R19){
        banks[0] = 5; banks[1] =  6; banks[2] =  7; banks[3] =  8;
        cols[0]  = 0; cols[1]  = 16; cols[2]  = 32; cols[3]  = 48;
      }
    
      else if (test_type == R20){
        banks[0] = 5; banks[1] =  6; banks[2] =  7; banks[3] =  8;
        cols[0]  = 0; cols[1]  = 29; cols[2]  = 51; cols[3]  = 67;
      }


      my_test->turn_bus(BUSDIR::WRITE);
      
      for (uint index = 0 ; index < 3; index++){

        bank = banks[index];
        DramAddr * da = new DramAddr(row, bank);
        my_test->act(da);
        delete da;
        
        DramAddr * da2 = new DramAddr(cols[index], row, bank);
        my_test->write_column(da2, patterns[index]);
        delete da2;       
      }
      break; 
           
    }

    case (B0C0) :
    case (B0C1) :
    case (B0C2) :
    case (B0CX) :
    case (BXCX) :
    default     : 
     { 
        my_test->turn_bus(BUSDIR::WRITE);
      
        for (uint bank = 1 ; bank < 4 ; bank++){
          DramAddr * da = new DramAddr(row,bank);
          my_test->act(da);
          delete da;
        
          for (uint col_index = 0 ; col_index < 3 ; col_index++){
            DramAddr * da2 = new DramAddr(cols[col_index],row,bank);
            my_test->write_column(da2, patterns[col_index]);
            delete da2;
          }
        }
      break;      
    }

  }
  my_test->turn_bus(BUSDIR::READ);
  // cout << "Reached to the end of initialization." <<endl;
  my_test->end_of_init();
  // cout << "Starting the looping part." <<endl;

  // Test Loops
  DramAddr * da0 = new DramAddr(cols[0], row, banks[0]);
  DramAddr * da1 = new DramAddr(cols[1], row, banks[0]);
  DramAddr * da2 = new DramAddr(cols[2], row, banks[0]);
  
  DramAddr * da3 = new DramAddr(cols[1], row, banks[1]);
  DramAddr * da4 = new DramAddr(cols[2], row, banks[2]);
  DramAddr * da5 = new DramAddr(cols[0], row, banks[1]);

  DramAddr * da6 = new DramAddr(cols[0], row, banks[0]);
  DramAddr * da7 = new DramAddr(cols[1], row, banks[1]);
  DramAddr * da8 = new DramAddr(cols[2], row, banks[2]);


  while(my_test->cq->size < LOOP_SIZE + INIT_SIZE - 50){
    switch (test_type) {

      case (B0C0) : 
        my_test->read_column(da0);
        my_test->read_column(da0);
        my_test->read_column(da0);
        my_test->read_column(da0);
        // cout << "Baseline Loop B0C0 is created." <<endl;
        break;

      case (B0C1) : 
        my_test->read_column(da1);
        my_test->read_column(da1);
        my_test->read_column(da1);
        my_test->read_column(da1);
        // cout << "Baseline Loop B0C1 is created." <<endl;
        break;

      case (B0B1C0) :
        my_test->read_column(da0);
        my_test->read_column(da5);
        my_test->read_column(da0);
        my_test->read_column(da5);
        break;

      case (B0C2) : 
        my_test->read_column(da2);
        my_test->read_column(da2);
        my_test->read_column(da2);
        my_test->read_column(da2);
        // cout << "Baseline Loop B0C2 is created." <<endl;
        break;

      case (B0CX) : 
      case (R1):
      case (R2):
      case (R3):
      case (R4):
      case (R5):
      case (R6):
      case (R7):
      case (R8):
      case (R9):
      case (R10):
      case (R11):
      case (R12):
      case (R13):
      case (R14):
      case (R15):
      case (R16):
      case (R17):
      case (R18):
      case (R19):
      case (R20):
        my_test->read_column(da6);
        my_test->read_column(da7);
        my_test->read_column(da8);
        my_test->read_column(da7);
        // cout << "Baseline Loop B0CX is created." <<endl;
        break;

      case (BXCX) : 
        my_test->read_column(da0);
        my_test->read_column(da1);
        my_test->read_column(da2);
        my_test->read_column(da1);
        // cout << "Baseline Loop BXCX is created." <<endl;
        break;

      case (X0X1X2) : 
        my_test->read_column(da0);
        my_test->read_column(da3);
        my_test->read_column(da4);
        my_test->read_column(da3);
        // cout << "Baseline Loop BXCX is created." <<endl;
        break;

      case (B0C0C1) : 
        my_test->read_column(da0);
        my_test->read_column(da1);
        my_test->read_column(da0);
        my_test->read_column(da1);
        // cout << "Baseline Loop BXCX is created." <<endl;
        break;
    }
  }

  my_test->send_commands();

  delete da0;
  delete da1;
  delete da2;
  delete da3;
  delete da4;
  delete da5;
  delete da6;
  delete da7;
  delete da8;
  delete my_test;

  return 0;
}

test_t which_test(string test)
{
  string test_strs [NUM_TESTS];
  test_strs[STOP]   = "stop,stoploop";
  test_strs[B0C0]   = "b0c0,base0";
  test_strs[B0B1C0] = "b0b1c0";
  test_strs[B0C1]   = "b0c1,base1";
  test_strs[B0C2]   = "b0c2,base2";
  test_strs[B0CX]   = "b0cx";
  test_strs[BXCX]   = "bxcx";
  test_strs[X0X1X2] = "x0x1x2";
  test_strs[B0C0C1] = "b0c0c1";
  test_strs[R1]     = "r1";
  test_strs[R2]     = "r2";
  test_strs[R3]     = "r3";
  test_strs[R4]     = "r4";
  test_strs[R5]     = "r5";
  test_strs[R6]     = "r6";
  test_strs[R7]     = "r7";
  test_strs[R8]     = "r8";
  test_strs[R9]     = "r9";
  test_strs[R10]    = "r10";
  test_strs[R11]    = "r11";
  test_strs[R12]    = "r12";
  test_strs[R13]    = "r13";
  test_strs[R14]    = "r14";
  test_strs[R15]    = "r15";
  test_strs[R16]    = "r16";
  test_strs[R17]    = "r17";
  test_strs[R18]    = "r18";
  test_strs[R19]    = "r19";
  test_strs[R20]    = "r20";

  for (int i = ZERO ; i < NUM_TESTS ; i++){
      int loc = test_strs[i].find(test);
      if ( loc != -1 ){
          return (test_t) i;
      }
  }
  cout << "[ERROR] Test is not defined!" << endl;
  exit(EXIT_FAILURE);
}
