#ifndef __INET_DELAYSIGNALLISTENER_H
#define __INET_DELAYSIGNALLISTENER_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API DelaySignalListener : public cListener
{
    public:
        DelaySignalListener(cModule* owner):owner(owner){}
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t moduleId, cObject *details) override;
    private:
        cModule* owner; // owner of this listner
};

} // namespace inet

#endif