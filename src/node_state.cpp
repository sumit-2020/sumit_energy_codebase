#include "node_state.h"

using namespace std;

string NodeState::get(string key){
  nodemap::iterator it = this->node_info.find(key);
  if(it == this->node_info.end())
  {
    cout << "Key " << key << " cannot be found in " << this->path << endl;
    exit(EXIT_FAILURE); 
  }
  return it->second;
}

int NodeState::set(string key, string value)
{
  int retval = 0;
  nodemap::iterator it = this->node_info.find(key);
  if(it != this->node_info.end()){
    retval = 1;
    if (it->second == value){
      cout << key << " value is already " << value << endl;
      retval = 2; 
    }
  }
  this->node_info[key] = value;
  // this->update_record();
  return retval;
}

bool NodeState::update_record(bool sandbox)
{
  if (!sandbox){
    std::string file = this->path;
    std::ofstream info_file;
    info_file.open(file.c_str());
    if (!info_file) {
      std::cout << "Unable to open " << this->path << std::endl;
      exit(EXIT_FAILURE);
    }

    int index = 0;
    info_file <<  ",arg,value" << endl;
    for (  nodemap::iterator it = this->node_info.begin(); 
          it != this->node_info.end(); ++it )
    {  
      if (it->first != "arg"){
        info_file << index << "," << it->first << "," << it->second << endl;
        index ++;
      }
    }
    info_file.close();
  }
  return true;
}
