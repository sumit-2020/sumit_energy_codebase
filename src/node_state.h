#ifndef NODEINFO_H
#define NODEINFO_H

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "softmc_api.h"

typedef std::pair<std::string,std::string> noderecord;
typedef std::map<std::string,std::string> nodemap;

class NodeState {
  private:
    nodemap node_info;
    std::string homedir;
    std::string path;
  public:

    NodeState(){
      // std::cout << "Creating NodeState " << std::endl;
      std::string line;
      std::ifstream info_file_in;

      this->homedir = getenv("HOME");
      this->path = this->homedir+"/energy_codebase/node.info";
      // std::cout << "Opening " << this->path << std::endl;
      info_file_in.open(this->path.c_str());

      if (!info_file_in) {
          std::cout << "Unable to open " << this->path << std::endl;
          exit(EXIT_FAILURE);
      }
      // std::cout << "Opened!" << std::endl;
      while (getline (info_file_in,line))
      {
        std::string index = line.substr(0                 , line.find(","));
        std::string key   = line.substr(line.find(",")+1  , line.find_last_of(",") - (line.find(",")+1));
        std::string value = line.substr(line.find_last_of(",")+1);
        this->node_info.insert(noderecord(key,value));
        // std::cout << index << ": " << key << " : " << value << std::endl;
      }
      info_file_in.close();
    }

    std::string get(std::string);
    int set(std::string,std::string);
    bool update_record(bool);
};

#endif
