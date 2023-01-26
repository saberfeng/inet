//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/redundancy/StreamClassifier.h"

#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamClassifier);

void StreamClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mode = par("mode");
        mapping = check_and_cast<cValueMap *>(par("mapping").objectValue());
        gateIndexOffset = par("gateIndexOffset");
        defaultGateIndex = par("defaultGateIndex");
    }
}

void StreamClassifier::handleParameterChange(const char *name){
    if (name != nullptr) {
        if (!strcmp(name, "mapping")) {
            mapping = check_and_cast<cValueMap *>(par("mapping").objectValue());
        }
    }
}

int StreamClassifier::classifyPacket(Packet *packet)
{
    string packetName = packet->getFullName(); // "best effort-0" "d3-FG1-0" "FG0-0"
    vector<string> splitted = splitString(packetName, "-");
    string flowNameStr;
    if(splitted.size() == 2){
        flowNameStr = splitted[0];
    } else if(splitted.size() == 3){
        flowNameStr = splitted[1];
    } else {
        throw "unsupported packet name";
    }
//    string flowNameStr = splitString(packetName, "-")[1];
    assert(flowNameStr.substr(0,2) == string("FG"));
    const char* flowName = flowNameStr.c_str();

    if (mapping->containsKey(flowName)) {
        int outputGateIndex = mapping->get(flowName).intValue() + gateIndexOffset;
        if (consumers[outputGateIndex]->canPushPacket(packet, outputGates[outputGateIndex]->getPathEndGate()))
            return outputGateIndex;
    }
    return defaultGateIndex;
}

} // namespace inet

