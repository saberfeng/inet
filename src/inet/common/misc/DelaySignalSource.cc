
#include "inet/common/misc/DelaySignalSource.h"

namespace inet {

Define_Module(DelaySignalSource);

void DelaySignalSource::initialize()
{
    startTime = par("startTime");
//    endTime = par("endTime");
    numActionToDo = par("numActionToDo");
    scheduleAt(startTime, new cMessage("timer"));
}

void DelaySignalSource::handleMessage(cMessage *msg)
{
    if (numActionToDo < 0 || numActionDone < numActionToDo) {
        cModule* targetModule = this->findModuleByPath("TsnDumbbellNetwork.switch1.eth[2].macLayer.server");

        targetModule->par("effectStartTime") = simTime().dbl(); // apply delay from now
        targetModule->par("effectDuration") = 0.001; // this change expire after 1000us
        targetModule->par("delayLength") = 0.000001; // 1us

        numActionDone++;
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

