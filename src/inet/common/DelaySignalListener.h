#ifndef __INET_DELAYSIGNALLISTENER_H
#define __INET_DELAYSIGNALLISTENER_H

#include "inet/common/INETDefs.h"
#include "inet/common/DelaySignalListenerOwner.h"

namespace inet {

class INET_API DelaySignalListener : public cListener
{
    public:
        DelaySignalListener(DelaySignalListenerOwner* owner):owner(owner){}
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t moduleId, cObject *details) override;
    private:
        DelaySignalListenerOwner* owner; // owner of this listener
};

} // namespace inet

#endif
