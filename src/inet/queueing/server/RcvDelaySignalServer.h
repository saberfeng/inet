#ifndef __INET_RCVDELAYSIGNALSERVER_H
#define __INET_RCVDELAYSIGNALSERVER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketServerBase.h"

namespace inet{
namespace queueing {
class INET_API RcvDelaySignalServer : public ClockUserModuleMixin<PacketServerBase>
{
    protected:
      cMessage *serveTimer = nullptr;
      ClockEvent *processingTimer = nullptr;
      Packet *packet = nullptr;

    protected:
      virtual void initialize(int stage) override;
      virtual void handleMessage(cMessage *message) override;
      virtual void scheduleProcessingTimer();
      virtual bool canStartProcessingPacket();
      virtual void startProcessingPacket();
      virtual void endProcessingPacket();

    public:
      virtual ~RcvDelaySignalServer();

      virtual void handleCanPushPacketChanged(cGate *gate) override;
      virtual void handleCanPullPacketChanged(cGate *gate) override;

      virtual std::string resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet


#endif
