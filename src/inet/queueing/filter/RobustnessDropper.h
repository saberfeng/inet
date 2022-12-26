/*
 * RobustnessDropper.h
 *
 *  Created on: Oct 5, 2022
 *      Author: xingbo
 */

#ifndef __INET_ROBUSTNESSDROPPER_H
#define __INET_ROBUSTNESSDROPPER_H

#include <vector>
#include <string>
#include "inet/queueing/base/PacketFilterBase.h"
#include "inet/common/MyHelper.h"
#include <ostream>
#include <unordered_map>

namespace inet{
namespace queueing{


using std::string;
using std::stod;
using std::stoi;
using std::stoll;
using std::to_string;
using std::vector;
using std::unordered_map;

/**
 * GCL Based Dropper module
 */
class INET_API RobustnessDropper : public PacketFilterBase
{
protected:
    long long hypercycle; // in ns
    string rawIngressWindows; //"201000-322000 701000-822000"
    string rawGlobalSafeIntervals; //"201000-322000:121000-122000 122000-334000,701000-822000:121000-122000 122000-321000"

    int numPackets;
    int numDropped;

    class Window{
        public:
            Window(){}
            Window(long long start, long long end):start(start),end(end){}
            long long start; // in ns
            long long end; // in ns
            operator std::string() const
            {
                return to_string(start) + string("-") + to_string(end);
            }
    };
    class WindowKeyCompare
    { // just used as a map key compare function
       bool operator() (const Window& lhs, const Window& rhs) const
       {
           return lhs.start < rhs.start;
       }
    };
    vector<Window> ingressWindows;

    using WindowToIntervalsMap = unordered_map<Window, vector<Window>, WindowKeyCompare>;
    vector<Window> globalSafeIntervals;
    WindowToIntervalsMap globalSafe;

    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;
    virtual void parseHypercycle();
    virtual void parseIngressWindows();
    virtual void parseGlobalSafe();
    virtual bool checkTimeInAnyWindow(long long time, vector<Window> windows) const;
    virtual long long getModNow() const; // current sim time mod hypercycle (in ns)
public:
    virtual bool matchesPacket(const Packet *packet) const override; // match->process, not match->drop

    friend std::ostream& operator<<(std::ostream& os, const vector<RobustnessDropper::Window>& wins);
};

std::ostream& operator<<(std::ostream& os, const vector<RobustnessDropper::Window>& wins);

}
}

#endif
