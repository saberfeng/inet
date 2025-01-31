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
        isOpen_ = par("initiallyOpen");
        durations = check_and_cast<cValueArray *>(par("durations").objectValue());
        cycleDuration = par("cycleDuration");
        convertDurationsToVectors();
    }
    int seed = 0;
    randGenerator = std::mt19937(seed);
}

void RcvDelaySignalServer::printDurationEntries(){
    // all entries in one line
    // each entry: startTimeNs-durationNs-isOpen
    for (int i = 0; i < durationEntries.size(); i++){
        DurationEntry entry = durationEntries[i];
        std::cout << entry.startTimeNs << "-" << entry.durationNs << "-" << entry.isOpen << " ";
    }
    std::cout<<std::endl;
}

void RcvDelaySignalServer::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "initiallyOpen"))
            isOpen_ = par("initiallyOpen");
        else if (!strcmp(name, "durations")) {
            durations = check_and_cast<cValueArray *>(par("durations").objectValue());
            convertDurationsToVectors();
        } else if (!strcmp(name, "cycleDuration")) {
            cycleDuration = par("cycleDuration");
        }
    }
}

void RcvDelaySignalServer::convertDurationsToVectors(){
    if (durations->size() % 2 != 0)
        throw cRuntimeError("The duration parameter must contain an even number of values");
    bool cur_slot_is_open = isOpen_;
    long long startTimeNs = 0;
    for(int i = 0; i < durations->size(); i ++){
        double dur_s = durations->get(i).doubleValueInUnit("s");
        long long durNs = dur_s * 1e9;
        durationEntries.push_back(DurationEntry(startTimeNs, durNs, cur_slot_is_open));
        cur_slot_is_open = !cur_slot_is_open;
        startTimeNs += durNs;
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
        rescheduleInCloseDuration(packet, processingTimer);
    }
}

void RcvDelaySignalServer::rescheduleInCloseDuration(Packet* packet, ClockEvent* processingTimer){

    // // debug--------------------
    // // get packet name
    // std::string packetName = packet->getName();
    // if (packetName == string("FG71-0")){
    //     simtime_t now1 = simTime();
    // }// ------------------------
    
    // iterate durations, find the next close slot
    simtime_t now = simTime();
    // get current simtime mod cycleDuration, in ns
    long long now_ns = now.dbl() * 1e9;
    long long cycleDuration_ns = cycleDuration.dbl() * 1e9;
    long long now_mod_ns = now_ns % cycleDuration_ns;

    // printDurationEntries();

    // find the first slot has startTimeNs >= now_mod_cycleDuration_ns
    int nxt_slot_index = 0;
    for(int i = 0; i < durationEntries.size(); i ++){
        if (durationEntries[i].startTimeNs >= now_mod_ns){
            nxt_slot_index = i;
            break;
        }
    }

    long long pktTxDelayNs = getPacketTransDelayNs(packet);
    long long front_spacing = 3000; // 3us, 3000ns
    long long tail_spacing = 2000; // 2us, 2000ns
    long long expandedPktTxDelayNs = pktTxDelayNs + front_spacing + tail_spacing; // eliminate prop delay and precision error

    // starting from the nxt_slot_index, find the next close slot
    // if no close slot, then find from the beginning of the cycle
    int next_close_slot_index = -1;
    for(int i = 0; i < durationEntries.size(); i ++){
        int cur_idx = (nxt_slot_index + i)%durationEntries.size();
        if (!durationEntries[cur_idx].isOpen && 
             durationEntries[cur_idx].durationNs >= expandedPktTxDelayNs){
            next_close_slot_index = cur_idx;
            break;
        }
    }
    if (next_close_slot_index==-1){
        throw cRuntimeError("No close slot found");
    }

    long long originalDurLen = durationEntries[next_close_slot_index].durationNs;

    // create a new DurationEntry with the delayNs, for this packet
    DurationEntry newDurationEntry(
        durationEntries[next_close_slot_index].startTimeNs,
        expandedPktTxDelayNs, true);
    // insert the newDurationEntry into durationEntries
    durationEntries.insert(durationEntries.begin() + next_close_slot_index,
                            newDurationEntry);
    // update the durationNs and startTimeNs of the splitted duration entry
    durationEntries[next_close_slot_index+1].durationNs -= newDurationEntry.durationNs;
    durationEntries[next_close_slot_index+1].startTimeNs += newDurationEntry.durationNs;
    
    packetToTmpDurationIdx[string(packet->getName())] = next_close_slot_index;

    long long waitDelay = 0;
    if (now_mod_ns < newDurationEntry.startTimeNs){
        waitDelay = newDurationEntry.startTimeNs + front_spacing - now_mod_ns;
    } else {
        waitDelay = cycleDuration_ns - now_mod_ns + newDurationEntry.startTimeNs + front_spacing;
    }

    // debug
    std::cout << "----------------found close duration, "
              << this->getParentModule()->getParentModule()->getParentModule()->getName()<< "->" << packet->getName()
              << " t:" << simTime().ustr(SimTimeUnit::SIMTIME_US)
              << " pktTxDelay:" << pktTxDelayNs/1000.0 << "us"
              << " nxtTry:" << (durationEntries[next_close_slot_index].startTimeNs+front_spacing)/1000.0 << "us"
              << " wait:" << waitDelay/1000.0 << "us"
              << " durLen:" << originalDurLen/1000.0 << "us" << std::endl;

    scheduleClockEventAfter(waitDelay/1e9, processingTimer);
}

long long RcvDelaySignalServer::getPacketTransDelayNs(Packet* packet){
    // get the interface, which is this server module's parent of parent
    cModule* interface = getParentModule()->getParentModule();
    // get the bitrate parameter
    double bitrate = interface->par("bitrate").doubleValue(); // bps
    // get the packet length
    long long packetByteLength = packet->getByteLength() + 8;
    // get the delay in ns
    long long delayNs = (packetByteLength * 8 / bitrate) * 1e9;
    return delayNs;
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

    if (delayLength>0){
        rescheduleInCloseDuration(packet, processingTimer);
    } else {
        scheduleClockEventAfter(0, processingTimer);
    }
    
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
              << ", t:" << simTime().ustr(SimTimeUnit::SIMTIME_US)
              << " pktTx:" << getPacketTransDelayNs(packet)/1000.0 << "us" << std::endl;
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
    // remove packet corresponding duration entry
    string packetName = string(packet->getName());
    if (packetToTmpDurationIdx.find(packetName) != packetToTmpDurationIdx.end()){
        int durationIdx = packetToTmpDurationIdx[packetName];
        durationEntries.erase(durationEntries.begin() + durationIdx);
        packetToTmpDurationIdx.erase(packetName);
    }
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
