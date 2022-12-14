/*
 * RobustnessDropper.cc
 *
 *  Created on: Oct 5, 2022
 *      Author: xingbo
 */

#include "inet/queueing/filter/RobustnessDropper.h"

#include "inet/common/ModuleAccess.h"
#include <string>
#include <fstream>



namespace inet {
namespace queueing {

Define_Module(RobustnessDropper);


void RobustnessDropper::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        parseHypercycle();
        string tmp1 = par("ingressWindows");
        rawIngressWindows = tmp1;
        string tmp2 = par("globalSafeIntervals");
        rawGlobalSafeIntervals = tmp2;
    } else if (stage == INITSTAGE_QUEUEING) {
        parseIngressWindows();
        parseGlobalSafe();
    }
}

void RobustnessDropper::parseHypercycle(){
    hypercycle = stoll(par("hypercycle")); // in ns
}

void RobustnessDropper::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "hypercycle"))
            parseHypercycle();
        else if (!strcmp(name, "ingressWindows")){
            string tmp = par("ingressWindows");
            rawIngressWindows = tmp;
            parseIngressWindows();
        } else if (!strcmp(name, "globalSafeIntervals")) {
            string tmp = par("globalSafeIntervals");
            rawGlobalSafeIntervals = tmp;
            parseGlobalSafe();
        }
    }
}

void RobustnessDropper::parseIngressWindows(){
    vector<string> rawWindows = splitString(rawIngressWindows, string(" "));
    for(auto& rawWindow : rawWindows){
        vector<string> start_end = splitString(rawWindow, string("-"));
        ingressWindows.push_back(
            Window(stoll(start_end[0]), stoll(start_end[1])));
    }
}

void RobustnessDropper::parseGlobalSafe(){
    vector<string> rawIntervalss = splitString(rawGlobalSafeIntervals, string(" "));
    for(auto& rawInterval : rawIntervalss){
        vector<string> start_end = splitString(rawInterval, string("-"));
        globalSafeIntervals.push_back(
            Window(stoll(start_end[0]), stoll(start_end[1])));
    }
}

long long RobustnessDropper::getModNow() const {
    simtime_t now = simTime();
    long long nsNow = (now * 1e9).dbl();
    return nsNow % hypercycle;
}

bool RobustnessDropper::matchesPacket(const Packet *packet) const{
    long long nsModNow = getModNow();
    if(checkTimeInAnyWindow(nsModNow, ingressWindows) || 
      (checkTimeInAnyWindow(nsModNow, globalSafeIntervals))){
        return true;
    } else {
        return false;
    }
}

bool RobustnessDropper::checkTimeInAnyWindow(long long time_ns, vector<Window> windows) const{
    bool in = false;
    for(auto& window: windows){
        if(time_ns > window.start && time_ns < window.end){
            in = true;
            break;
        }
    }
    return in;
}



void RobustnessDropper::dropPacket(Packet *packet)
{
    // EV_DEBUG << "Dropping packet" << EV_FIELD(ordinalNumber, numPackets) << EV_FIELD(packet) << EV_ENDL;
    // numPackets++;
    // numDropped++;
    PacketFilterBase::dropPacket(packet);
}

// void RobustnessDropper::processPacket(Packet *packet)
// {
//     // numPackets++;
// }


}
}
