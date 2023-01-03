#ifndef __INET_DELAYSIGNALLISTENER_H
#define __INET_DELAYSIGNALLISTENER_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API DelaySignalListener : public cListener
{
  public:
    DelaySignalListener(cModule* module){}

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif