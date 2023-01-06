#ifndef __INET_DELAYSIGNALSOURCE_H
#define __INET_DELAYSIGNALSOURCE_H

#include "inet/common/INETDefs.h"
#include "inet/queueing/contract/ISignalSource.h"

namespace inet {

class INET_API DelaySignalSource : public cSimpleModule, public virtual ISignalSource
{
  protected:
    simtime_t startTime;
    int numActionToDo;
    int numActionDone = 0;

  public:
    DelaySignalSource() {}

  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void finish() override;
};

} // namespace inet

#endif
