#include "inet/linklayer/configurator/GlobalSafeConfigurator.h"


namespace inet{

Define_Module(GlobalSafeConfigurator);

void GlobalSafeConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        string globalSafeInputPath = par("globalSafeInput");
        string ingressInputPath = par("ingressScheduleInput");
        globalSafeInput = globalSafeInputPath;
        ingressScheduleInput = ingressInputPath;
    }
    if (stage == INITSTAGE_QUEUEING) {
        parseIngressFile();
        parseGlobalSafeFile();
    }
}

void GlobalSafeConfigurator::parseIngressFile(){
    cout<<"**************" << ingressScheduleInput << endl;
    std::ifstream ingressInputFile(ingressScheduleInput);
    string line;
    getline(ingressInputFile, line); // skip the first header line
    while(getline(ingressInputFile, line)){
        if(line == string("")){
            continue;
        } 
        vector<string> components = splitString(line, string(","));
        string nodeId = components[0];
        string flowId = components[1];
        int hypercycle = stoi(components[2]);
        string rawWindows = components[4];

        if(ingressMap.find(nodeId) == ingressMap.end()){ // key nodeId not exist
            ingressMap[nodeId] = FlowMap();
        }
        auto flowIngressMap = ingressMap.at(nodeId);
        assert(flowIngressMap.find(flowId) == flowIngressMap.end());
        flowIngressMap[flowId] = IngressSchedule(hypercycle, rawWindows);
    }
    ingressInputFile.close();
}

void GlobalSafeConfigurator::parseGlobalSafeFile(){
    cout<<"**************" << globalSafeInput << endl;
    std::ifstream globalSafeInputFile(globalSafeInput);
    string line;
    getline(globalSafeInputFile, line); // skip the 
    while(getline(globalSafeInputFile, line)){
        if(line == string("")){
            continue;
        } 
        vector<string> components = splitString(line, string(","));
        string nodeId = components[0];
        string flowId = components[1];
        int hypercycle = stoi(components[2]);
        string rawIntervals = components[4];
        
        if(globalSafeMap.find(nodeId) == globalSafeMap.end()){ // key nodeId not exist
            globalSafeMap[nodeId] = FlowMap();
        }
        auto flowIntervalMap = globalSafeMap.at(nodeId);
        if(flowIntervalMap.find(flowId) == flowIntervalMap.end()){ // key flowId not exist
            flowIntervalMap[flowId] = IngressSchedule(hypercycle, rawIntervals);
        } else {
            flowIntervalMap.at(flowId).rawWindows += string(",") + rawIntervals;
        }
    }
    globalSafeInputFile.close();
}


}