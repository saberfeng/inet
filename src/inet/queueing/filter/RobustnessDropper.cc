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
        useRandomFilter = par("useRandomFilter");

        randGenerator = std::mt19937(123);
        randDistribution = std::uniform_int_distribution<>(0, 1);
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
        } else if (!strcmp(name, "useRandomFilter")) {
            useRandomFilter = par("useRandomFilter");
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
    if(rawGlobalSafeIntervals.empty()){
        return; // when applying all-filter policy, no global-safe input
    }
    // globalSafeIntervals.clear();
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
    long long nsNow = now.inUnit(SimTimeUnit::SIMTIME_NS);
    // long long nsNow = now.dbl() * 1e9;
    return nsNow % hypercycle;
}

void RobustnessDropper::updateCurIdx(){
    curIngressWinIdx = (curIngressWinIdx+1) % ingressWindows.size();
}

void RobustnessDropper::logAction(
    string actionName, const Packet *packet, long long nsModNow, 
    const Window& curInWindow, const vector<Window>& gsIntervals) const {

    std::cout << "Log, " << actionName << ", " 
              << this->getParentModule()->getParentModule()->getParentModule()->getParentModule()->getName() 
                    << ".." << this->getName() << "[" << this->getIndex() << "]"
              << "->"  << packet->getName() 
              << ", t:" << simTime().ustr(SimTimeUnit::SIMTIME_US)
              << ", nsModNow:" << nsModNow
              << ", curInWin:" << string(curInWindow)
              << ", gsIntervals:" << gsIntervals
              << std::endl;
}

void RobustnessDropper::logAction(
    string actionName, const Packet *packet, long long nsModNow, 
    const Window& curInWindow) const {

    std::cout << "Log, " << actionName << ", " 
              << this->getParentModule()->getParentModule()->getParentModule()->getParentModule()->getName() 
                    << ".." << this->getName() << "[" << this->getIndex() << "]"
              << "->"  << packet->getName() 
              << ", t:" << simTime().ustr(SimTimeUnit::SIMTIME_US)
              << ", nsModNow:" << nsModNow
              << ", curInWin:" << string(curInWindow)
              << std::endl;
}



bool RobustnessDropper::matchesPacket(const Packet *packet) const{
    long long nsModNow = getModNow();
    string packetName = packet->getName(); // get packet name: FG17-0
    string frameIdxStr = packetName.substr(packetName.find("-")+1); // get frame index
    int curInWindowIdxNew = stoi(frameIdxStr)%ingressWindows.size();

    auto& curInWindow = ingressWindows[curInWindowIdxNew];
    
    long long precision = 1; // 1ns precision
    if(globalSafe.empty()){
        if(grterEqualForPrecision(nsModNow, curInWindow.start, precision) && 
           lessEqualForPrecision(nsModNow, curInWindow.end, precision)){
            logAction(string("Hit ingress window"), packet, nsModNow, curInWindow);
            return true;
        } else {
            if (useRandomFilter){
                if (randDistribution(randGenerator) < 0.5){ // 50% drop rate
                    return true;
                } else {
                    logAction(string("Dropping packet"), packet, nsModNow, curInWindow);
                    return false;
                }
            } else{
                logAction(string("Dropping packet"), packet, nsModNow, curInWindow);
                return false;
            }
        }
    }else{
        auto& gsIntervals = globalSafe.at(curInWindowIdxNew);

        if(grterEqualForPrecision(nsModNow, curInWindow.start, precision) && 
           lessEqualForPrecision(nsModNow, curInWindow.end, precision)){
            logAction(string("Hit ingress window"), packet, nsModNow, curInWindow, gsIntervals);
            return true;
        } else if (checkTimeInAnyWindow(nsModNow, gsIntervals)){
            logAction(string("Hit global safe"), packet, nsModNow, curInWindow, gsIntervals);
            return true;
        } else {
            logAction(string("Dropping packet"), packet, nsModNow, curInWindow, gsIntervals);
            return false;
        }
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
             << EV_ENDL;
    numPackets++;
}


bool RobustnessDropper::grterEqualForPrecision(
    long long left, long long right, long long precision) const {

    long long absDiff = std::abs(left - right);
    if (absDiff <= precision){
        return true;
    } else {
        return left > right;
    }
}

bool RobustnessDropper::lessEqualForPrecision(
    long long left, long long right, long long precision) const {

    long long absDiff = std::abs(right - left);
    if (absDiff <= precision){
        return true;
    } else {
        return left < right;
    }
}


}
}
