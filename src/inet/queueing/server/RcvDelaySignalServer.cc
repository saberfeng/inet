#include "inet/queueing/server/RcvDelaySignalServer.h"

#include "inet/common/PacketEventTag.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag.h"

namespace inet {
namespace queueing {

Define_Module(RcvDelaySignalServer);


RcvDelaySignalServer::~RcvDelaySignalServer()
{
    cancelAndDelete(serveTimer);
    cancelAndDeleteClockEvent(processingTimer);
    delete packet;
}

void RcvDelaySignalServer::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        int serveSchedulingPriority = par("serveSchedulingPriority");
        if (serveSchedulingPriority != -1) {
            serveTimer = new cMessage("ServeTimer");
            serveTimer->setSchedulingPriority(serveSchedulingPriority);
        }
        processingTimer = new ClockEvent("ProcessingTimer");
    }
}


bool RcvDelaySignalServer::isNowInEffect(){
    simtime_t now = simTime();
    return (now >= simtime_t(par("effectStartTime"))) &&
            delayedPackets < int(par("numDelayPackets"));
}

void RcvDelaySignalServer::handleMessage(cMessage *message)
{
    if (message == serveTimer) {
        startProcessingPacket();
        scheduleProcessingTimer();
    }
    else if (message == processingTimer) {
        endProcessingPacket();
        if (canStartProcessingPacket()) {
            startProcessingPacket();
            scheduleProcessingTimer();
        }
        updateDisplayString();
    }
    else
        throw cRuntimeError("Unknown message");
}

void RcvDelaySignalServer::scheduleProcessingTimer()
{
    clocktime_t delayLength;
    if(isNowInEffect()){
        delayLength = par("delayLength");
        delayedPackets++;
    } else {
        delayLength = 0; // out of effect time, no delay
    }
//    auto processingBitrate = bps(par("processingBitrate"));
//    delayLength += s(packet->getTotalLength() / processingBitrate).get();
    scheduleClockEventAfter(delayLength, processingTimer);
    if (delayLength > 0){
        std::cout << "Log, Delay Attack, " 
              << this->getParentModule()->getParentModule()->getParentModule()->getName() 
              << "->" << packet->getName() 
              << ", t:" << simTime().ustr(SimTimeUnit::SIMTIME_US)<< std::endl;
    }
}

bool RcvDelaySignalServer::canStartProcessingPacket()
{
    return provider->canPullSomePacket(inputGate->getPathStartGate()) &&
           consumer->canPushSomePacket(outputGate->getPathEndGate());
}

void RcvDelaySignalServer::startProcessingPacket()
{
//    // DEBUG
//    if(isDebugTargetModule()){
//        cout << "**********" << getNedTypeAndFullPath() << " pulling" << endl;
//    }
    packet = provider->pullPacket(inputGate->getPathStartGate());
    take(packet);
    emit(packetPulledSignal, packet);
    EV_INFO << "Processing packet started" << EV_FIELD(packet) << EV_ENDL;
}

void RcvDelaySignalServer::endProcessingPacket()
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
//    // DEBUG
//    if(isDebugTargetModule()){
//        cout << "**********" << getNedTypeAndFullPath() << " handleCanPushPacketChanged" << endl;
//    }
    Enter_Method("handleCanPushPacketChanged");
    if (!processingTimer->isScheduled() && canStartProcessingPacket()) {
        if (serveTimer)
            rescheduleAt(simTime(), serveTimer);
        else {
            startProcessingPacket();
            scheduleProcessingTimer();
        }
    }
}

void RcvDelaySignalServer::handleCanPullPacketChanged(cGate *gate)
{
//    // DEBUG
//    if(isDebugTargetModule()){
//        cout << "**********" << getNedTypeAndFullPath() << " handleCanPullPacketChanged" << endl;
//    }
    Enter_Method("handleCanPullPacketChanged");
    if (!processingTimer->isScheduled() && canStartProcessingPacket()) {
        if (serveTimer)
            rescheduleAt(simTime(), serveTimer);
        else {
            startProcessingPacket();
            scheduleProcessingTimer();
        }
    }
}

std::string RcvDelaySignalServer::resolveDirective(char directive) const
{
    switch (directive) {
        case 's':
            return processingTimer->isScheduled() ? "processing" : "";
        default:
            return PacketServerBase::resolveDirective(directive);
    }
}


} // namespace queueing
} // namespace inet
