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
    clocktime_t processingTime = par("processingTime");
    auto processingBitrate = bps(par("processingBitrate"));
    processingTime += s(packet->getTotalLength() / processingBitrate).get();
    scheduleClockEventAfter(processingTime, processingTimer);
}

bool RcvDelaySignalServer::canStartProcessingPacket()
{
    return provider->canPullSomePacket(inputGate->getPathStartGate()) &&
           consumer->canPushSomePacket(outputGate->getPathEndGate());
}

void RcvDelaySignalServer::startProcessingPacket()
{
    cout << "**********" << getNedTypeAndFullPath() << " pulling" << endl;
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
    cout << "**********" << getNedTypeAndFullPath() << " pushing" << endl;
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;
    packet = nullptr;
}

void RcvDelaySignalServer::handleCanPushPacketChanged(cGate *gate)
{
    cout << "**********" << getNedTypeAndFullPath() << " handleCanPushPacketChanged" << endl;
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
    cout << "**********" << getNedTypeAndFullPath() << " handleCanPullPacketChanged" << endl;
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
