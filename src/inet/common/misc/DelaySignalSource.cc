
#include "inet/common/misc/DelaySignalSource.h"

namespace inet {

Define_Module(DelaySignalSource);

void DelaySignalSource::initialize()
{
    startTime = par("startTime");
    endTime = par("endTime");
    signal = registerSignal("delaySignal");
    scheduleAt(startTime, new cMessage("timer"));
}

void DelaySignalSource::handleMessage(cMessage *msg)
{
    if (endTime < 0 || simTime() < endTime) {
        int targetModuleId = this->findModuleByPath("TsnDumbbellNetwork.switch1.eth[2].macLayer.server")->getId();

        auto details = new cValueMap();
        details->set("delayLength",0.000001); // 1us
        details->set("effectStartTime", simTime().dbl()); // start apply delay now
        details->set("effectDuration", 1.0); // effect lasts for 1s

        emit(signal, targetModuleId, details);
        scheduleAfter(par("interval"), msg);
    }
    else {
        delete msg;
    }
}

void DelaySignalSource::finish()
{
}

} // namespace inet

