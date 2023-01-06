/*
 * DelaySignalListenerOwner.h
 *
 *  Created on: Jan 6, 2023
 *      Author: xingbo
 */

#ifndef INET_COMMON_DELAYSIGNALLISTENEROWNER_H_
#define INET_COMMON_DELAYSIGNALLISTENEROWNER_H_

#include "inet/common/INETDefs.h"


namespace inet{

class INET_API DelaySignalListenerOwner{
public:
    virtual void updateEffectParam(
            simtime_t newDelayLength,
            simtime_t newEffectStartTime,
            simtime_t newEffectDuration) = 0;
};

}



#endif /* INET_COMMON_DELAYSIGNALLISTENEROWNER_H_ */
