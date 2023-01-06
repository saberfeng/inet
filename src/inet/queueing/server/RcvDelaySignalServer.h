#ifndef __INET_RCVDELAYSIGNALSERVER_H
#define __INET_RCVDELAYSIGNALSERVER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketServerBase.h"
#include "inet/common/DelaySignalListener.h"
#include "inet/common/DelaySignalListenerOwner.h"
#include <iostream>

namespace inet{
namespace queueing {

using std::cout;
using std::endl;

class INET_API RcvDelaySignalServer : public ClockUserModuleMixin<PacketServerBase>, public cListener
{
    protected:
      cMessage *serveTimer = nullptr;
      ClockEvent *processingTimer = nullptr;
      Packet *packet = nullptr;
      simtime_t delayLength;
      simtime_t effectStartTime;
      simtime_t effectDuration;


    protected:
      virtual void initialize(int stage) override;
      virtual void handleMessage(cMessage *message) override;
      virtual void scheduleProcessingTimer();
      virtual bool canStartProcessingPacket();
      virtual void startProcessingPacket();
      virtual void endProcessingPacket();
      virtual bool isDebugTargetModule();
      virtual bool isNowInEffect();

    public:
      virtual ~RcvDelaySignalServer();

      virtual void handleCanPushPacketChanged(cGate *gate) override;
      virtual void handleCanPullPacketChanged(cGate *gate) override;

      virtual std::string resolveDirective(char directive) const override;
      virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t moduleId, cObject *details) override;
};

} // namespace queueing
} // namespace inet


#endif
