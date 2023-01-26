#ifndef __INET_INICOMPLEMENTCONFIGURATOR_H
#define __INET_INICOMPLEMENTCONFIGURATOR_H

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"
#include "inet/common/MyHelper.h"
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>


namespace inet{

using std::string;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::getline;
using std::stoi;
using std::unordered_map;
using std::stringstream;

class INET_API IniComplementConfigurator : public NetworkConfiguratorBase{
    protected:
        virtual void initialize(int stage) override;
        virtual void configure();
//        virtual void skipEmptyLine(ifstream& inputFile);
        virtual void parseHeader(string& header, string& confName, int& lineNum);
        virtual void configureWrapper(vector<string>& lines, string& confName);
        virtual void configureClientApps(vector<string>& lines);
        virtual void configureServerApps(vector<string>& lines);
        virtual void configureEncDecApps(vector<string>& lines);
        virtual void configureStreamIdenApps(vector<string>& lines);
//        virtual void configureGateSchedApps(vector<string>& lines);
        virtual string getNetName();
};

}


#endif
