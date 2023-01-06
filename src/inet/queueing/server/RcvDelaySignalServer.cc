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
        delayLength = par("delayLength");
        effectStartTime = par("effectStartTime");
        effectDuration = par("effectDuration");
        subscribe("delaySignal", this);
    }
}

void RcvDelaySignalServer::receiveSignal(cComponent *source, simsignal_t signalID, intval_t moduleId, cObject *details){
    simsignal_t delaySignalId = cComponent::registerSignal("delaySignal");
    if(delaySignalId != signalID){ // only receive delaySignal
        return;
    }

    if(this->getId() != moduleId){ // only process signal targeting its owner
        return;
    }

    cValueMap* detailMap = check_and_cast<cValueMap*>(details);
    assert(detailMap->containsKey("delayLength") &&
           detailMap->containsKey("effectStartTime") &&
           detailMap->containsKey("effectDuration"));
    delayLength = detailMap->get("delayLength").doubleValue();
    effectStartTime = detailMap->get("effectStartTime").doubleValue();
    effectDuration = detailMap->get("effectDuration").doubleValue();
    std::cout << "received signal! delayLength:" << delayLength
              << " effectStartTime:" << effectStartTime
              << " effectDuration:" << effectDuration << std::endl;
}

bool RcvDelaySignalServer::isDebugTargetModule(){
    return getNedTypeAndFullPath() ==
           std::string("(inet.queueing.server.RcvDelaySignalServer)TsnDumbbellNetwork.switch1.eth[2].macLayer.server");
}

bool RcvDelaySignalServer::isNowInEffect(){
    simtime_t now = simTime();
    return (now >= effectStartTime) && (now <= effectStartTime + effectDuration);
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
    } else {
        delayLength = 0; // out of effect time, no delay
    }
    auto processingBitrate = bps(par("processingBitrate"));
    delayLength += s(packet->getTotalLength() / processingBitrate).get();
    scheduleClockEventAfter(delayLength, processingTimer);
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
