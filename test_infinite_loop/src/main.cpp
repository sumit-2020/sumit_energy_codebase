#include "test_power_infloopcomp.h"
#include <algorithm>
#include <string.h>

#define NUM_THREADS 1

using namespace std;

PowerTest * pt;

pthread_mutex_t cout_mutex = PTHREAD_MUTEX_INITIALIZER ;
pthread_mutex_t flexbuf_mutex = PTHREAD_MUTEX_INITIALIZER;

struct reader_args {
    int thread_id;
};

void * thread_function(void * _args ){
    struct reader_args * args = (struct reader_args *) _args;
    pthread_mutex_lock(&cout_mutex);
    cout << "Thread " << args->thread_id << " is listening to the read buffer.\n";
    pthread_mutex_unlock(&cout_mutex);
    pt->log_read_data(flexbuf_mutex, cout_mutex);
}

test_name arg_test_name_match(string s){
    for(uint i = 0; i < s.length(); ++i) {
        s[i] = tolower(s[i]);
    }    

    string tests [NUM_TESTS];
    tests[ACT_PRE]          = "idd0,actpre";
    tests[ACT_RD_PRE]       = "idd1,1rd,actrdpre";
    tests[ACT_RD_RD_PRE]    = "2rds,act2rdspre,actrdrdpre";
    tests[ACT_WR_PRE]       = "1wr,actwrpre";
    tests[ACT_WR_WR_PRE]    = "2wrs,act2wrspre,actwrwrpre";
    tests[ACT_WR_RD_PRE]    = "wrrd,actwrrdpre";
    tests[ACT_WR_RD_RD_PRE] = "wrrdrd,actwrrdrdpre";
    tests[VERIFY]           = "verify";

    for (int i = 0 ; i < NUM_TESTS ; i++){
        cout << "Search " << s << " in " << tests[i] << " : ";
        int loc = tests[i].find(s);
        if ( loc != -1){
            cout << "found at index " << loc << endl;
            return (test_name) i;
        }
        cout << endl;
    }

    cout << "Could not find the test " << s << ". Please type one of the following: " << endl;
    for (int i = 0 ; i < NUM_TESTS ; i++){
        for (int j = 0 ; j < 4 ; j++){
            cout << tests[i][j] << " ";
        }
        cout << endl;
    }
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    string arg_test_name(argv[1]);
    uint8_t pattern(atoi(argv[2]));

    pt = new PowerTest();
    bool inf_read = false;

    pt->insert_addr(0,0); // access to bank 0 / row 0

    cout << "Test name: " << arg_test_name << " , ";
    cout << "Pattern: " << hex << int(pattern) << endl;
    test_name test_no = arg_test_name_match(arg_test_name);
    pt->test_idd( test_no, pattern );
    pt->send_commands();
    
    // pthread_t threads[NUM_THREADS];
    // reader_args *args = (reader_args*) malloc((NUM_THREADS) * sizeof(reader_args));

    // for (int i = 0 ; i < NUM_THREADS ; i++){
    //     args[i].thread_id = i;

    //     int rc = pthread_create(&threads[i], NULL, thread_function , (void *) &args[i]);
    //     if (rc){
    //         printf("ERROR; return code from pthread_create() is %d\n", rc);
    //         exit(EXIT_FAILURE);
    //     }
    // }

    if (inf_read){
        pt->inf_read_back(flexbuf_mutex);
    }
    
    delete pt;
}