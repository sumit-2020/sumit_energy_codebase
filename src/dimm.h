#ifndef DIMM_H
#define DIMM_H

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <tgmath.h>
#include "softmc_api.h"

class Dimm {
  public:
    // Latency Values in ns.
    std::string dimm = "";
    std::string chip = "";
    std::map <std::string,int> latencies;

    Dimm(std::string str){
      this->dimm = str;
      this->find_chip();
      this->fetch_parameters();
    }

    void find_chip();
    void fetch_parameters();
    void dump_latencies();
    std::vector<std::string> string_split(std::string s, const char delimiter);
};

#endif
