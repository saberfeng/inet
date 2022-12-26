#ifndef __INET_GLOBALSAFECONFIGURATOR_H
#define __INET_GLOBALSAFECONFIGURATOR_H

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"
#include "inet/common/MyHelper.h"
#include "inet/protocolelement/redundancy/StreamClassifier.h"
#include "inet/queueing/filter/RobustnessDropper.h"
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

        // key1:nodeId, key2:flowId, value:filterIndex
        using FlowIdxMap = unordered_map<string, int>;
        using NodeFilterMap = unordered_map<string, FlowIdxMap>;
        NodeFilterMap filterMap;

        using FlowIdxRevMap = unordered_map<int, string>;
        using NodeFilterRevMap = unordered_map<string, FlowIdxRevMap>;
        NodeFilterRevMap filterRevMap; // just reversed filterMap, use filterIdx as key, flowId as value

        virtual void initialize(int stage) override;
        virtual void prepareTopology();
        virtual void parseGlobalSafeFile();
        virtual void parseIngressFile();
        // establish mapping between flowId & filterId for each node
        virtual void initFilterMap(); 
        virtual void initFilterRevMap();
        virtual void configureFilterMap();
        virtual void configIngressSchedGlobalSafe();
        /**
         * Computes the network configuration for all nodes in the network.
         * The result of the computation is only stored in the configurator.
         */

};

}


#endif
