#include "dimm.h"

using namespace std;

void Dimm::find_chip(){
  string file_path("../dimm_specs/dimm_model_match.csv");
  ifstream in(file_path.c_str());
  if (!in.is_open()) {
    cout << "Matcher file "+file_path+" cannot be found" << endl;
    exit(EXIT_FAILURE);
  }

  string line;
  while (getline(in,line)){
    std::vector<std::string> cols = this->string_split(line,',');
    if (this->dimm.compare(cols.at(0)) == 0){
      // found the line for the dimm
      this->chip = cols.at(2);
      cout << "- Chip is " << this->chip << endl;
      return;
    }
  }

  cout << "Chip of dimm " << this->dimm << " can not be found in " << file_path << endl;
  exit(EXIT_FAILURE);

}

void Dimm::fetch_parameters(){
  string file_path = "../dimm_specs/" + this->chip + ".chip";
  ifstream in(file_path.c_str());
  if (!in.is_open()) {
    cout << "Chip specification file "+file_path+" cannot be found" << endl;
    exit(EXIT_FAILURE);
  }

  string line;
  cout << "- Latency values for "<<this->dimm<<":"<<endl;
  while (getline(in,line)){
    vector<string> cols = this->string_split(line,',');
    if (cols.at(0).substr(0,1).compare("t") == 0){
      string name = cols.at(0);
      cout << "   Fetching " << name << " ";
      string val_str = cols.at(1);
      string unit = cols.at(2);
      float value = stof(val_str);
      int cycles = 0;

      if (unit == "ns"){
        cycles = ceil((value * 1000.0) / DEF_TCK);
        printf("%s = %fns -> %d cycles -> %fns \n",name.c_str(),value, cycles, (((float)cycles*DEF_TCK)/1000));
      }
      else {
        cycles = ceil(value);
        printf("%s = %fCK -> %d cycles -> %fns \n",name.c_str(),value, cycles, (((float)cycles*DEF_TCK)/1000));
      }
      this->latencies[name]=value;
    }
  }
  cout << "- Fetched all latency values." << endl;
}

void Dimm::dump_latencies(){
  cout << "Latency parameters for " << this->dimm << ": ";
  for (map<string,int>::const_iterator it = this->latencies.begin() ;
    it != this->latencies.end() ; it++)
  {
    cout << it->first << "=" << (it->second * DEF_TCK / 1000) << "ns ";
  }
  cout << endl;
}

std::vector<std::string> Dimm::string_split(std::string s, const char delimiter)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);

    std::vector<std::string> output;

    while (end <= std::string::npos)
    {
      output.emplace_back(s.substr(start, end-start));

      if (end == std::string::npos)
        break;

      start=end+1;
      end = s.find_first_of(delimiter, start);
    }

    return output;
}
