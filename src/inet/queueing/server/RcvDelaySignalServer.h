#ifndef __INET_RCVDELAYSIGNALSERVER_H
#define __INET_RCVDELAYSIGNALSERVER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketServerBase.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <random>

namespace inet{
namespace queueing {

using std::cout;
using std::endl;
using std::unordered_map;
using std::string;

class INET_API RcvDelaySignalServer : public ClockUserModuleMixin<PacketServerBase>
{
    protected:
      // ClockEvent *processingTimer = nullptr;
      // Packet *packet = nullptr;
      // map storing key value pairs of processing timers and corresponding packets
      std::unordered_map<ClockEvent *, Packet *> procTimerToPacketMap;
      int delayedPackets = 0;
      std::mt19937 randGenerator;

    protected:
      virtual void initialize(int stage) override;
      virtual void handleMessage(cMessage *message) override;
      virtual void scheduleProcessingTimer(Packet* packet);
      virtual void rescheduleRandomProcessingTimer(Packet* packet, 
                                                 ClockEvent* processingTimer,
                                                 double delay_lowerbound_s, 
                                                 double delay_upperbound_s);
      virtual bool canStartProcessingPacket();
      virtual Packet* startProcessingPacket();
      virtual void endProcessingPacket(Packet* packet, ClockEvent* processingTimer);
      virtual bool isNowInEffect();

    public:
      virtual ~RcvDelaySignalServer();

      virtual void handleCanPushPacketChanged(cGate *gate) override;
      virtual void handleCanPullPacketChanged(cGate *gate) override;

      virtual std::string resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet


#endif
