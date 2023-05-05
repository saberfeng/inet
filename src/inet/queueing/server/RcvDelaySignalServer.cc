#include "inet/queueing/server/RcvDelaySignalServer.h"

#include "inet/common/PacketEventTag.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag.h"

namespace inet {
namespace queueing {

Define_Module(RcvDelaySignalServer);


RcvDelaySignalServer::~RcvDelaySignalServer()
{
    // delete all packets in the procTimerToPacketMap
    for (auto it = procTimerToPacketMap.begin(); it != procTimerToPacketMap.end(); it++){
        delete it->second;
    }
    // cancel and delete all timers
    for (auto it = procTimerToPacketMap.begin(); it != procTimerToPacketMap.end(); it++){
        cancelAndDeleteClockEvent(it->first);
    }
}

void RcvDelaySignalServer::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        // processingTimer = new ClockEvent("ProcessingTimer");
    }
    int seed = 0;
    randGenerator = std::mt19937(seed);
}


bool RcvDelaySignalServer::isNowInEffect(){
    simtime_t now = simTime();
    return (now >= simtime_t(par("effectStartTime"))) &&
            delayedPackets < int(par("numDelayPackets"));
}

void RcvDelaySignalServer::handleMessage(cMessage *message)
{
    // check if the message is a ClockEvent* in the procTimerToPacketMap
    ClockEvent* processingTimer = check_and_cast<ClockEvent*>(message);
    if (procTimerToPacketMap.find(processingTimer) == procTimerToPacketMap.end()){
        throw cRuntimeError("Unknown message");
    } 
        
    Packet* packet = procTimerToPacketMap[processingTimer];
    // check whether can push packet (whether PacketTransmitter is idle)
    if(consumer->canPushSomePacket(outputGate->getPathEndGate())){
        // if yes, then push the packet
        endProcessingPacket(packet, processingTimer);
        if (canStartProcessingPacket()) { // prepare for the next waiting packet
            Packet* packet = startProcessingPacket();
            scheduleProcessingTimer(packet);
        }
        updateDisplayString();
    } else {
        // if no, then reschedule the timer
        double delay_lowerbound_s = 50 * 8 / 1e9; // 50 bits, 1000M
        double delay_upperbound_s = 1500 * 8 / 1e8; // 1500 bits, 100M
        rescheduleRandomProcessingTimer(packet, processingTimer, 
                                        delay_lowerbound_s, delay_upperbound_s);
    }
}

void RcvDelaySignalServer::scheduleProcessingTimer(Packet* packet)
{
    clocktime_t delayLength;
    if(isNowInEffect()){
        delayLength = par("delayLength");
        delayedPackets++;
    } else {
        delayLength = 0; // out of effect time, no delay
    }

    std::string processingTimerName = std::string("ProcessingTimer-")+packet->getName();
    ClockEvent* processingTimer = new ClockEvent(processingTimerName.c_str());
    procTimerToPacketMap[processingTimer] = packet;

    scheduleClockEventAfter(delayLength, processingTimer);
    if (delayLength > 0){
        std::cout << "Log, Delay Attack, " 
              << this->getParentModule()->getParentModule()->getParentModule()->getName() 
              << "->" << packet->getName() 
              << ", t:" << simTime().ustr(SimTimeUnit::SIMTIME_US)
              << ", delay:" << int(delayLength.dbl()*1e6) << "us"
              << std::endl;
    }
}

void RcvDelaySignalServer::rescheduleRandomProcessingTimer(Packet* packet, 
                                    ClockEvent* processingTimer,
                                    double delay_lowerbound_s, 
                                    double delay_upperbound_s){
    // random delay
    auto distribution = std::uniform_real_distribution<double>(
        delay_lowerbound_s, delay_upperbound_s);
    double delayLength_s = distribution(randGenerator);
    scheduleClockEventAfter(delayLength_s, processingTimer);
}

bool RcvDelaySignalServer::canStartProcessingPacket()
{
    return provider->canPullSomePacket(inputGate->getPathStartGate()) &&
           consumer->canPushSomePacket(outputGate->getPathEndGate());
}

Packet* RcvDelaySignalServer::startProcessingPacket()
{
    Packet* packet = provider->pullPacket(inputGate->getPathStartGate());
    take(packet);
    emit(packetPulledSignal, packet);
    EV_INFO << "Processing packet started" << EV_FIELD(packet) << EV_ENDL;
    return packet;
}

void RcvDelaySignalServer::endProcessingPacket(Packet* packet, ClockEvent* processingTimer)
{
    EV_INFO << "Processing packet ended" << EV_FIELD(packet) << EV_ENDL;
    std::cout << "Log, Dispacth packet, " 
              << this->getParentModule()->getParentModule()->getParentModule()->getName()
              << "->" << packet->getName() 
              << ", t:" << simTime().ustr(SimTimeUnit::SIMTIME_US) << std::endl;
    simtime_t packetProcessingTime = simTime() - processingTimer->getSendingTime();
    simtime_t bitProcessingTime = packetProcessingTime / packet->getBitLength();
    insertPacketEvent(this, packet, PEK_PROCESSED, bitProcessingTime);
    increaseTimeTag<ProcessingTimeTag>(packet, bitProcessingTime, packetProcessingTime);
    processedTotalLength += packet->getDataLength();
    emit(packetPushedSignal, packet);
//    // DEBUG
//    if(isDebugTargetModule()){
//        cout << "**********" << getNedTypeAndFullPath() << " pushing" << endl;
//    }
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;

    // remove the timer from the map
    procTimerToPacketMap.erase(processingTimer);

    packet = nullptr;
}

void RcvDelaySignalServer::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (canStartProcessingPacket()) {
        Packet* packet = startProcessingPacket();
        scheduleProcessingTimer(packet);
    }
}

void RcvDelaySignalServer::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (canStartProcessingPacket()) {
        Packet* packet = startProcessingPacket();
        scheduleProcessingTimer(packet);
    }
}

std::string RcvDelaySignalServer::resolveDirective(char directive) const
{
    // switch (directive) {
    //     case 's':
    //         return processingTimer->isScheduled() ? "processing" : "";
    //     default:
    //         return PacketServerBase::resolveDirective(directive);
    // }
    return "";
}


} // namespace queueing
} // namespace inet
