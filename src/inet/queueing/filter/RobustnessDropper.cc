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
#include <algorithm>


namespace inet {
namespace queueing {

Define_Module(RobustnessDropper);

std::ostream& operator<<(std::ostream& os, const vector<RobustnessDropper::Window>& wins){
    os << "[";
    for(const auto& win : wins){
        os << string(win) << ",";
    }
    os << "]";
    return os;
}


void RobustnessDropper::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numPackets = 0;
        numDropped = 0;
        curIngressWinIdx = 0;
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
    hypercycle = par("hypercycle"); // in ns
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
    ingressWindows.clear();
    vector<string> rawWindows = splitString(rawIngressWindows, string(" "));
    for(auto& rawWindow : rawWindows){
        if(rawWindow.empty()){
            continue;
        }
        vector<string> start_end = splitString(rawWindow, string("-"));
        assert(start_end.size() == 2);
        ingressWindows.push_back(
            Window(stoll(start_end[0]), stoll(start_end[1])));
    }
}

void RobustnessDropper::parseGlobalSafe(){
    globalSafeIntervals.clear();
    //"201000-322000:121000-122000 122000-334000,701000-822000:121000-122000 122000-321000"
    vector<string> rawMaps = splitString(rawGlobalSafeIntervals, string(","));
    for(auto& rawMap : rawMaps){ //rawMap->"201000-322000:121000-122000 122000-334000"
        if(rawMap.empty()){
            continue;
        }
        vector<string> splittedMap = splitString(rawMap, ":");
        string ingressWin = splittedMap[0];
        string rawIntervals = splittedMap[1];

        // get corresponding ingress window index
        vector<string> splittedInWin = splitString(ingressWin, "-");
        auto it = std::find(ingressWindows.begin(), ingressWindows.end(), Window(
            stoll(splittedInWin[0]), stoll(splittedInWin[1])));
        assert(it != ingressWindows.end());
        int idx = std::distance(ingressWindows.begin(), it);
        globalSafe[idx] = vector<Window>();

        vector<string> rawSplittedIntervals = splitString(rawIntervals, string(" "));
        for(auto& rawInterval : rawSplittedIntervals){
            if(rawInterval.empty()){
                continue;
            }
            vector<string> start_end = splitString(rawInterval, string("-"));
            assert(start_end.size() == 2);
            globalSafe.at(idx).push_back(
                Window(stoll(start_end[0]), stoll(start_end[1])));
        }
    }
}

long long RobustnessDropper::getModNow() const {
    simtime_t now = simTime();
    // long long nsNow = (now * 1e9).dbl(); // 10^19ps, too high precision for SimTime
    long long nsNow = now.dbl() * 1e9;
    return nsNow % hypercycle;
}

void RobustnessDropper::updateCurIdx(){
    curIngressWinIdx = (curIngressWinIdx+1) % ingressWindows.size();
}

bool RobustnessDropper::matchesPacket(const Packet *packet) const{
    long long nsModNow = getModNow();
    auto& curInWindow = ingressWindows[curIngressWinIdx];
    auto& globalSafeIntervals = globalSafe.at(curIngressWinIdx);

    if((nsModNow >= curInWindow.start && nsModNow < curInWindow.end) || 
      (checkTimeInAnyWindow(nsModNow, globalSafeIntervals))){
        return true;
    } else {
        return false;
    }
}

bool RobustnessDropper::checkTimeInAnyWindow(long long time_ns, vector<Window> windows) const{
    bool in = false;
    for(auto& window: windows){
        if(time_ns >= window.start && time_ns < window.end){
            in = true;
            break;
        }
    }
    return in;
}



void RobustnessDropper::dropPacket(Packet *packet)
{
    updateCurIdx();
    EV_DEBUG << "Dropping packet"
             << EV_FIELD(numDropped)
             << EV_FIELD(packet->getFullName())
             << EV_FIELD(ingressWindows)
             << EV_FIELD(globalSafeIntervals)
             << EV_ENDL;
    numPackets++;
    numDropped++;
    PacketFilterBase::dropPacket(packet);
}

void RobustnessDropper::processPacket(Packet *packet)
{
    updateCurIdx();
    EV_DEBUG << "Processing packet"
             << EV_FIELD(numPackets)
             << EV_FIELD(packet->getFullName())
             << EV_FIELD(ingressWindows)
             << EV_FIELD(globalSafeIntervals) << EV_ENDL;
    numPackets++;
}


}
}
