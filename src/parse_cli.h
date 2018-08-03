//Structure for the help options

#ifndef SAFARIMC_PARSE_CLI_H_
#define SAFARIMC_PARSE_CLI_H_

#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <getopt.h>
#include "utils.h"
using namespace std;


#define     TRCD_ENUM   1
#define     TRAS_ENUM   2
#define     TWR_ENUM    3
#define     TREF_ENUM   4
#define     TRP_ENUM    5
#define     TRRD_ENUM   6


const struct option longopts[] =
{
    {"help",    no_argument,       0, 'h'}, //(h)elp
    {"count",   no_argument,       0, 'c'}, //(c)ount file
    {"vdd",     required_argument, 0, 'v'}, //(v)dd
    {"nrow",    required_argument, 0, 'r'}, //(r)ow
    {"ncol",    required_argument, 0, 'l'}, //co(l)
    {"test",    required_argument, 0, 't'}, //REQUIRED
    {"dimm",    required_argument, 0, 'd'}, //REQUIRED
    {"wait",    required_argument, 0, 'w'}, //(w)ait time
    {"patt",    required_argument, 0, 'p'}, //(p)attern
    {"start",   required_argument, 0, 's'}, //(s)tart time
    {"end",     required_argument, 0, 'e'}, //(e)nd time
    {"outdir",  required_argument, 0, 'o'}, //(o)utput dir
    {"temp",    required_argument, 0, 'm'}, //te(m)perature
    {"bypass",  no_argument,       0, 'b'}, //(b)ypass verify read
    {0,0,0,0}
} ;


//Function primitives

int parse_test_name(string &test_name) ;
void parse_pattern(string *pattern_string, vector<uint8_t> *pattern) ;
int parse_start_delay(int test_num) ;
void print_usage() ;
#endif //SAFARIMC_PARSE_CLI_H_
