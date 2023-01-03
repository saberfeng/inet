
#include "inet/common/misc/DelaySignalSource.h"

namespace inet {

Define_Module(DelaySignalSource);

void DelaySignalSource::initialize()
{
    startTime = par("startTime");
    endTime = par("endTime");
    signal = registerSignal(par("signalName"));
    scheduleAt(startTime, new cMessage("timer"));
}

void DelaySignalSource::handleMessage(cMessage *msg)
{
    if (endTime < 0 || simTime() < endTime) {
        double value = par("value");
        emit(signal, value);
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

