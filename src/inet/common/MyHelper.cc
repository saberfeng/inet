// MyHelper.cc
#include "inet/common/MyHelper.h"

namespace inet{


vector<string> splitString(string txt, string delimiter){
    size_t last_pos = 0;
    size_t next_pos = 0;
    vector<string> res;
    while((next_pos = txt.find(delimiter, last_pos)) != string::npos){
        res.push_back(txt.substr(last_pos, next_pos-last_pos));
        last_pos = next_pos + delimiter.length();
    }
    res.push_back(txt.substr(last_pos));
    return res;
}


simtime_t str_us_to_s(std::string& str_us_time){
    return us_to_s(std::stod(str_us_time));
}

simtime_t us_to_s(double us_time){
    return us_time / double(1e6);
}

simtime_t str_ns_to_s(string& str_ns_time){
    return ns_to_s(std::stod(str_ns_time));
}

simtime_t ns_to_s(double ns_time){
    return ns_time / double(1e9);
}

string getNetName(cModule* mod){
    string path = mod->getFullPath(); // TsnNetSmallRing.iniComplementConfigurator
    return splitString(path, ".")[0];
}


}
