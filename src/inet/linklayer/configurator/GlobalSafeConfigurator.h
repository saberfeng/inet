#ifndef __INET_GLOBALSAFECONFIGURATOR_H
#define __INET_GLOBALSAFECONFIGURATOR_H

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

class INET_API GlobalSafeConfigurator : public NetworkConfiguratorBase{
    protected:
        string globalSafeInput;
        string ingressScheduleInput;

        class IngressSchedule{
            public:
                IngressSchedule(){}
                IngressSchedule(int hypercycle, string rawWindows):
                        hypercycle(hypercycle),rawWindows(rawWindows){}
                int hypercycle;
                // for ingress windows, rawWindows is like: "201000-322000 701000-822000"
                // for global safe intervals, rawWindows: "201000-322000:121000-122000 122000-334000,701000-822000:121000-122000 122000-321000"
                string rawWindows;
        };
        // key1: nodeId, key2:flowId
        using FlowMap = unordered_map<string, IngressSchedule>;
        using NodeMap = unordered_map<string, FlowMap>;
        NodeMap ingressMap;
        NodeMap globalSafeMap;

        virtual void initialize(int stage) override;
        virtual void parseGlobalSafeFile();
        virtual void parseIngressFile();
        /**
         * Computes the network configuration for all nodes in the network.
         * The result of the computation is only stored in the configurator.
         */

};

}


#endif