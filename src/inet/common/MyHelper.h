// MyHelper.h
#ifndef __INET_MYHELPER_H
#define __INET_MYHELPER_H


#include <string>
#include <vector>

namespace inet{

using std::string;
using std::vector;
using omnetpp::simtime_t;

vector<string> splitString(string txt, string delimiter);

simtime_t str_us_to_s(string& str_us_time);

simtime_t us_to_s(double us_time);
}



#endif
