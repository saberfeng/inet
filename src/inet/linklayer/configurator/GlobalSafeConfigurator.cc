#include "inet/linklayer/configurator/GlobalSafeConfigurator.h"


namespace inet{

Define_Module(GlobalSafeConfigurator);

void GlobalSafeConfigurator::initialize(int stage){
    if (stage == INITSTAGE_LOCAL) {
        string globalSafeInputPath = par("globalSafeInput");
        string ingressInputPath = par("ingressScheduleInput");
        globalSafeInput = globalSafeInputPath;
        ingressScheduleInput = ingressInputPath;
    }
    if (stage == INITSTAGE_QUEUEING) {
        parseIngressFile();
        parseGlobalSafeFile();
        prepareTopology();
//        configureFilterMap();
    }
}

void GlobalSafeConfigurator::prepareTopology(){
    delete topology;
    topology = new Topology();
    extractTopology(*topology);
}

void GlobalSafeConfigurator::parseIngressFile(){
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
        string rawWindows = components[3];

        if(ingressMap.find(nodeId) == ingressMap.end()){ // key nodeId not exist
            ingressMap[nodeId] = FlowMap();
        }
        FlowMap& flowIngressMap = ingressMap.at(nodeId);
        assert(flowIngressMap.find(flowId) == flowIngressMap.end());
        flowIngressMap[flowId] = IngressSchedule(hypercycle, rawWindows);
    }
    ingressInputFile.close();

    initFilterMap();
}

void GlobalSafeConfigurator::initFilterMap(){
    for(const auto& nodeFlowSchedule : ingressMap){
        string nodeId = nodeFlowSchedule.first;
        if(filterMap.find(nodeId) == filterMap.end()){
            filterMap[nodeId] = FlowIdxMap();
        }
        for(const auto& flowSchedule : nodeFlowSchedule.second){
            string flowId = flowSchedule.first;
            if(filterMap.at(nodeId).find(flowId) == filterMap.at(nodeId).end()){
                filterMap.at(nodeId)[flowId] = filterMap.at(nodeId).size();
            }
        }
    }
}

void GlobalSafeConfigurator::configureFilterMap(){
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        cModule *nodeModule = node->module;
        string nodeName = nodeModule->getFullName();
        if(filterMap.find(nodeName) != filterMap.end()){// if we have ingress schedule for this node
            cModule* ieee8021qFilter = nodeModule->findModuleByPath(".bridging.streamFilter.ingress");
//            ieee8021qFilter->par("numStreams") = filterMap.at(nodeName).size();

            cModule* tmpClassifier = ieee8021qFilter->findModuleByPath(".classifier");
            StreamClassifier* classifier = dynamic_cast<StreamClassifier *>(tmpClassifier);
            assert(classifier != nullptr); // make sure the dynamic cast is successful

            cValueMap* mapping = new cValueMap();
            for(const auto& flowIdxMap : filterMap.at(nodeName)){
                mapping->set(flowIdxMap.first.c_str(), flowIdxMap.second);
            }
            classifier->par("mapping") =mapping;
//            parMapping.copyIfShared();
//            parMapping.setObjectValue(mapping);
        }
    }
}


void GlobalSafeConfigurator::parseGlobalSafeFile(){
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
        string rawIntervals = components[3];
        
        if(globalSafeMap.find(nodeId) == globalSafeMap.end()){ // key nodeId not exist
            globalSafeMap[nodeId] = FlowMap();
        }
        FlowMap& flowIntervalMap = globalSafeMap.at(nodeId);
        if(flowIntervalMap.find(flowId) == flowIntervalMap.end()){ // key flowId not exist
            flowIntervalMap[flowId] = IngressSchedule(hypercycle, rawIntervals);
        } else {
            flowIntervalMap.at(flowId).rawWindows += string(",") + rawIntervals;
        }
    }
    globalSafeInputFile.close();
}


}
