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
    if (procTimerToPacketMap.find(processingTimer) != procTimerToPacketMap.end()){
        // if so, then it is a processing timer
        Packet* packet = procTimerToPacketMap[processingTimer];
        endProcessingPacket(packet, processingTimer);
        if (canStartProcessingPacket()) { // prepare for the next waiting packet
            Packet* packet = startProcessingPacket();
            scheduleProcessingTimer(packet);
        }
        updateDisplayString();
    } else
        throw cRuntimeError("Unknown message");
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

//    auto processingBitrate = bps(par("processingBitrate"));
//    delayLength += s(packet->getTotalLength() / processingBitrate).get();
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
