#ifndef __INET_DELAYSIGNALSOURCE_H
#define __INET_DELAYSIGNALSOURCE_H

#include "inet/common/INETDefs.h"
#include "inet/queueing/contract/ISignalSource.h"
#include "inet/common/MyHelper.h"
#include <iostream>
#include <vector>
#include <random>


namespace inet {

using std::stringstream;

class INET_API DelaySignalSource : public cSimpleModule, public virtual ISignalSource
{
  protected:
    simtime_t startTime;
    int numActionToDo = 0;
    int numActionDone = 0;
    vector<cModule*> delaySvrs;

    std::mt19937 randGenerator;
    std::uniform_int_distribution<> distribution;

  public:
    DelaySignalSource() {}

  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void finish() override;
    void collectApplicableDevices();
};

} // namespace inet

#endif
