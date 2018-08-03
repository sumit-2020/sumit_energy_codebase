/*
   File containing the helper functions to parse the command line
   inputs.

Author : Abhijith Kashyap
akashyap@cmu.edu
parse_test_name : Parses the test name and maps it to a number
parse_pattern : Parses the pattern and returns a vector of uint8_t
print_usage : prints the help message
*/


#include "parse_cli.h"
int parse_test_name(string &test_name)
{
    //Figure out the test
    std::map<string,int> test_map ;

    test_map.insert(pair<string,int>("trcd",TRCD_ENUM )) ;
    test_map.insert(pair<string,int>("tras",TRAS_ENUM )) ;
    test_map.insert(pair<string,int>("twr" ,TWR_ENUM  )) ;
    test_map.insert(pair<string,int>("tref",TREF_ENUM )) ;
    test_map.insert(pair<string,int>("trp" ,TRP_ENUM  )) ;
    test_map.insert(pair<string,int>("trrd" ,TRRD_ENUM  )) ;

    int test_num ;
    std::map<string,int>::iterator test_it ;
    test_it = test_map.find(test_name) ;
    if (test_it != test_map.end())
        test_num = test_it->second ;
    else
        test_num = -1 ;

    return test_num ;
}


void parse_pattern(string *pattern_string, vector<uint8_t> *pattern)
{
    const char* pStart ;
    pStart = pattern_string->c_str() ;
    char* pEnd ;
    for (long i = strtol(pStart, &pEnd, 0);
            pStart != pEnd;
            i = strtol(pStart, &pEnd, 0))
    {
        pStart =   pEnd ;
        pattern->push_back((uint8_t) i) ;
    }
}

int parse_start_delay(int test_num)
{
    map<int, int> start_map =
    {
        {TRCD_ENUM,DEF_TRCD} ,
        {TRAS_ENUM,DEF_TRAS} ,
        {TWR_ENUM ,DEF_TWR } ,
        {TREF_ENUM,1       } ,
        {TRP_ENUM ,DEF_TRP } ,
        {TRRD_ENUM,1       } ,
    } ;

    return start_map[test_num] ;
}




void print_usage() {
    cout << "Usage: safari_mc_test [-hc --vdd <v> --nrow <num> --ncol <num>] \n\
        -t <test> -d <dimm_info_string> [--ret <wait/retention_time>] \n\
        --patt \"<8 bit pattern>\" --start <start_delay>  --end <final_delay> \
        -o <output_folder>\n\n" << endl ;
    cout << "\t-h       Print this help message." <<endl ;
    cout << "\t--vdd    Supply Voltage. DEFAULT : 1.5" << endl ;
    cout << "\t-c       Print out to count file. DEFAULT : No count file." <<endl ;
    cout << "\t--nrow   Number of Rows. DEFAULT : " << NUM_ROWS << endl ;
    cout << "\t--ncol   Number of Cols. DEFAULT : " << NUM_COLS << endl ;
    cout << "\t-t       Name of test - [trcd:tras:twr:tref:trp:trrd]" << endl ;
    cout << "\t-d       DIMM details - Example samsungy10" << endl ;
    cout << "\t--wait    Time in ms to wait between operations\n \
        \t\t\tBehavior varies between tests. DEFAULT : 0ms" << endl ;
    cout << "\t--patt   8 Bit patterns to test - Must be given as string" << endl ;
    cout << "\t\t\tExample : --patt \"0xff 0xcc 0xaa\"" << endl ;
    cout << "\t--start  Delay value to start the exp at. \
        DEFAULT : Will use DEF_* value if nothing is specified" << endl ;
    cout << "\t--end    Delay value to end the exp at. \
        DEFAULT : 1 ck" << endl ;
    cout << "\t--temp   Temperature. DEFAULT : 20c" << endl ;
    cout << "\t\t\tAlways terminate with \"c\"" << endl ;
    cout << "\t-o       Output folder (Will be created if it doesn't exists)" << endl ;
    cout << "\t\t DEFAULT : results" << endl ;
    cout << endl ;

}

