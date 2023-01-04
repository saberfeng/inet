#include "inet/common/DelaySignalListener.h"

namespace inet {

void DelaySignalListener::receiveSignal(cComponent *source, simsignal_t signalID, intval_t moduleId, cObject *details){
    // check if signal is for its owner, 
    // if yes, extract delayLength and validInterval and store in owner
    simsignal_t delaySignalId = cComponent::registerSignal("delaySignal");
    if(delaySignalId != signalID){ // only receive delaySignal
        return;
    }

    auto ownerId = owner->getId();
    if(ownerId != moduleId){ // only process signal targeting its owner
        return;
    }
    
    cValueMap* detailMap = check_and_cast<cValueMap*>(details);
    assert(detailMap->containsKey("delayLength") && detailMap->containsKey("effectDuration"));
    double delayLength = detailMap->get("delayLength").doubleValue();
    double effectDuration = detailMap->get("effectDuration").doubleValue();
}

}
