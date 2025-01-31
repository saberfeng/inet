//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/gatescheduling/base/GateScheduleConfiguratorBaseNew.h"

#include "inet/common/PatternMatcher.h"
#include "inet/queueing/gate/PeriodicGate.h"

namespace inet {

void GateScheduleConfiguratorBaseNew::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        gateCycleDuration = par("gateCycleDuration");
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
    }
    if (stage == INITSTAGE_QUEUEING) {
        computeConfiguration();
        configureGateScheduling();
        configureApplicationOffsets();
    }
}

void GateScheduleConfiguratorBaseNew::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "configuration")) {
            configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
            clearConfiguration();
            computeConfiguration();
            configureGateScheduling();
            configureApplicationOffsets();
        }
    }
}

void GateScheduleConfiguratorBaseNew::clearConfiguration()
{
    if (topology != nullptr)
        topology->clear();
    delete gateSchedulingInput;
    gateSchedulingInput = nullptr;
    delete gateSchedulingOutput;
    gateSchedulingOutput = nullptr;
}

void GateScheduleConfiguratorBaseNew::computeConfiguration()
{
    long startTime = clock();
    delete topology;
    topology = new Topology();
    TIME(extractTopology(*topology));
    TIME(gateSchedulingInput = createGateSchedulingInput());
    TIME(gateSchedulingOutput = computeGateScheduling(*gateSchedulingInput));
    printElapsedTime("computeConfiguration", startTime);
}

GateScheduleConfiguratorBaseNew::Input *GateScheduleConfiguratorBaseNew::createGateSchedulingInput() const
{
    auto input = new Input();
    addDevices(*input);
    addSwitches(*input);
    addPorts(*input);
    addFlows(*input);
    return input;
}

void GateScheduleConfiguratorBaseNew::addDevices(Input& input) const
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        if (!isBridgeNode(node)) {
            auto device = new Input::Device();
            device->module = node->module;
            input.devices.push_back(device);
            input.networkNodes.push_back(device);
        }
    }
}

void GateScheduleConfiguratorBaseNew::addSwitches(Input& input) const
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        if (isBridgeNode(node)) {
            auto switch_ = new Input::Switch();
            switch_->module = node->module;
            input.switches.push_back(switch_);
            input.networkNodes.push_back(switch_);
        }
    }
}

void GateScheduleConfiguratorBaseNew::addPorts(Input& input) const
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto networkNode = input.getNetworkNode(node->module);
        for (auto interface : node->interfaces) {
            auto networkInterface = interface->networkInterface;
            if (!networkInterface->isLoopback()) {
                auto subqueue = networkInterface->findModuleByPath(".macLayer.queue.queue[0]");
                auto port = new Input::Port();
//                int vec_size = subqueue->getVectorSize();
                port->numGates = subqueue != nullptr ? subqueue->getVectorSize() : -1;
                port->module = interface->networkInterface;
                port->datarate = bps(interface->networkInterface->getDatarate());
                port->propagationTime = check_and_cast<cDatarateChannel *>(interface->networkInterface->getTxTransmissionChannel())->getDelay();
                port->maxPacketLength = B(interface->networkInterface->getMtu());
                port->guardBand = s(port->maxPacketLength / port->datarate).get();
                port->maxCycleTime = gateCycleDuration;
                port->maxSlotDuration = gateCycleDuration;
                port->startNode = networkNode;
                networkNode->ports.push_back(port);
                input.ports.push_back(port);
            }
        }
    }
    for (auto networkNode : input.networkNodes) {
        auto node = check_and_cast<Node *>(topology->getNodeFor(networkNode->module));
        for (auto port : networkNode->ports) {
            auto networkInterface = check_and_cast<NetworkInterface *>(port->module);
            auto link = findLinkOut(findInterface(node, networkInterface));
            auto linkOut = findLinkOut(node, networkInterface->getNodeOutputGateId());
            auto remoteNode = check_and_cast<Node *>(linkOut->getLinkOutRemoteNode());
            port->endNode = *std::find_if(input.networkNodes.begin(), input.networkNodes.end(), [&] (const auto& networkNode) {
                return networkNode->module == remoteNode->module;
            });
            port->otherPort = *std::find_if(input.ports.begin(), input.ports.end(), [&] (const auto& otherPort) {
                return otherPort->module == link->destinationInterface->networkInterface;
            });
            ASSERT(port->endNode);
            ASSERT(port->otherPort);

            string startNode = port->startNode->module->getFullName(); // "d0"
            string endNode = port->endNode->module->getFullName(); // "s0"
            port->portId = startNode + "-" + endNode; // "d0-s0"
        }
    }
}

void GateScheduleConfiguratorBaseNew::addFlows(Input& input) const
{
    int flowIndex = 0;
    EV_DEBUG << "Computing flows from configuration" << EV_FIELD(configuration) << EV_ENDL;
    for (int k = 0; k < configuration->size(); k++) {
        auto entry = check_and_cast<cValueMap *>(configuration->get(k).objectValue());
        for (int i = 0; i < topology->getNumNodes(); i++) {
            auto sourceNode = (Node *)topology->getNode(i);
            cModule *source = sourceNode->module;
            for (int j = 0; j < topology->getNumNodes(); j++) {
                auto destinationNode = (Node *)topology->getNode(j);
                cModule *destination = destinationNode->module;
                PatternMatcher sourceMatcher(entry->get("source").stringValue(), true, false, false);
                PatternMatcher destinationMatcher(entry->get("destination").stringValue(), true, false, false);
                if (sourceMatcher.matches(sourceNode->module->getFullPath().c_str()) &&
                    destinationMatcher.matches(destinationNode->module->getFullPath().c_str()))
                {
                    int pcp = entry->get("pcp").intValue();
                    int gateIndex = entry->get("gateIndex").intValue();
                    b packetLength = b(entry->get("packetLength").doubleValueInUnit("b"));
                    simtime_t packetInterval = entry->get("packetInterval").doubleValueInUnit("s");
                    simtime_t maxLatency = entry->containsKey("maxLatency") ? entry->get("maxLatency").doubleValueInUnit("s") : -1;
                    simtime_t maxJitter = entry->containsKey("maxJitter") ? entry->get("maxJitter").doubleValueInUnit("s") : 0;
                    bps datarate = packetLength / s(packetInterval.dbl());
                    auto startDevice = input.getDevice(source);
                    auto endDevice = input.getDevice(destination);
                    auto startApplication = new Input::Application();
                    auto startApplicationModule = startDevice->module->getModuleByPath((std::string(".") + std::string(entry->get("application").stringValue())).c_str());
                    if (startApplicationModule == nullptr)
                        throw cRuntimeError("Cannot find flow start application, path = %s", entry->get("application").stringValue());
                    startApplication->module = startApplicationModule;
                    startApplication->device = startDevice;
                    startApplication->pcp = pcp;
                    startApplication->packetLength = packetLength;
                    startApplication->packetInterval = packetInterval;
                    startApplication->maxLatency = maxLatency;
                    startApplication->maxJitter = maxJitter;
                    input.applications.push_back(startApplication);
                    EV_DEBUG << "Adding flow from configuration" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(pcp) << EV_FIELD(packetLength) << EV_FIELD(packetInterval, packetInterval.ustr()) << EV_FIELD(datarate) << EV_FIELD(maxLatency, maxLatency.ustr()) << EV_FIELD(maxJitter, maxJitter.ustr()) << EV_ENDL;
                    auto flow = new Input::Flow();
                    flow->name = entry->containsKey("name") ? entry->get("name").stringValue() : (std::string("flow") + std::to_string(flowIndex++)).c_str();
                    flow->gateIndex = gateIndex;
                    flow->startApplication = startApplication;
                    flow->endDevice = endDevice;
                    cValueArray *pathFragments;
                    if (entry->containsKey("pathFragments"))
                        pathFragments = check_and_cast<cValueArray *>(entry->get("pathFragments").objectValue());
                    else {
                        auto pathFragment = new cValueArray();
                        for (auto node : computeShortestNodePath(sourceNode, destinationNode))
                            pathFragment->add(node->module->getFullName());
                        pathFragments = new cValueArray();
                        pathFragments->add(pathFragment);
                    }
                    for (int l = 0; l < pathFragments->size(); l++) {
                        auto path = new Input::PathFragment();
                        auto pathFragment = check_and_cast<cValueArray *>(pathFragments->get(l).objectValue());
                        for (int m = 0; m < pathFragment->size(); m++) {
                            for (auto networkNode : input.networkNodes) {
                                auto name = pathFragment->get(m).stdstringValue();
                                int index = name.find('.');
                                auto nodeName = index != std::string::npos ? name.substr(0, index) : name;
                                auto interfaceName = index != std::string::npos ? name.substr(index + 1) : "";
                                if (networkNode->module->getFullName() == nodeName) {
                                    if (m != pathFragment->size() - 1) {
                                        auto startNode = networkNode;
                                        auto endNodeName = pathFragment->get(m + 1).stdstringValue();
                                        int index = endNodeName.find('.');
                                        endNodeName = index != std::string::npos ? endNodeName.substr(0, index) : endNodeName;
                                        auto outputPort = *std::find_if(startNode->ports.begin(), startNode->ports.end(), [&] (const auto& port) {
                                            return port->endNode->module->getFullName() == endNodeName && (interfaceName == "" || interfaceName == check_and_cast<NetworkInterface *>(port->module)->getInterfaceName());
                                        });
                                        path->outputPorts.push_back(outputPort);
                                        path->inputPorts.push_back(outputPort->otherPort);
                                    }
                                    path->networkNodes.push_back(networkNode);
                                    break;
                                }
                            }
                        }
                        flow->pathFragments.push_back(path);
                    }
                    if (!entry->containsKey("pathFragments"))
                        delete pathFragments;
                    input.flows.push_back(flow);
                }
            }
        }
    }
    std::sort(input.flows.begin(), input.flows.end(), [] (const Input::Flow *r1, const Input::Flow *r2) {
        return r1->startApplication->pcp > r2->startApplication->pcp;
    });
}

void GateScheduleConfiguratorBaseNew::configureGateScheduling()
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto networkNode = node->module;
        for (auto interface : node->interfaces) {
            auto queue = interface->networkInterface->findModuleByPath(".macLayer.queue");
            auto rcvDelaySignalServer = interface->networkInterface->findModuleByPath(".macLayer.server");
            if (queue != nullptr) {
                for (cModule::SubmoduleIterator it(queue); !it.end(); ++it) {
                    cModule *gate = *it;
                    if (dynamic_cast<queueing::PeriodicGate *>(gate) != nullptr)
                        configureGateScheduling(networkNode, gate, interface, rcvDelaySignalServer);
                }
            }
        }
    }
}

void GateScheduleConfiguratorBaseNew::configureGateScheduling(
    cModule *networkNode, cModule *gate, Interface *interface, cModule *rcvDelaySignalServer)
{
    auto networkInterface = interface->networkInterface;
    bool initiallyOpen = false;
    simtime_t offset = 0;
    simtime_t slotEnd = 0;
    int gateIndex = gate->getIndex();
    auto port = gateSchedulingInput->getPort(networkInterface);

    // ----------- debug ----------------
//    string startNode = port->startNode->module->getFullName(); // "d0"
//    string endNode = port->endNode->module->getFullName(); // "s0"
//    string port_id = startNode + "-" + endNode;
//    if ((port_id == string("s0-d0")) && (gateIndex == 7)){
//        port_id += "!";
//    }

    auto it = gateSchedulingOutput->gateSchedules.find(port);
    if (it == gateSchedulingOutput->gateSchedules.end())
        throw cRuntimeError("Cannot find schedule for interface, interface = %s", networkInterface->getInterfaceFullPath().c_str());
    auto& schedules = it->second;
    if (gateIndex >= schedules.size())
        throw cRuntimeError("Cannot find schedule for traffic class, interface = %s, gate index = %d", port->module->getFullPath().c_str(), gateIndex);
    auto schedule = schedules[gateIndex];

    cValueArray *durations = new cValueArray();
    for(auto& slot: schedule->slots){
        durations->add(cValue(slot.duration.dbl(), "s"));
    }
    if (durations->size() % 2 != 0){
        // if num of durations is not even, add one empty slot
        durations->add(cValue(0, "s"));
    }

    // configure slots info to rcvDelaySignalServer, only configure for the gate 7
    if (schedule->slots.size() > 5) {
        rcvDelaySignalServer->par("initiallyOpen") = schedule->slots[0].open;
        rcvDelaySignalServer->par("cycleDuration") = schedule->cycleDuration.dbl();
        cPar& durationsPar = rcvDelaySignalServer->par("durations");
        durationsPar.copyIfShared();
        durationsPar.setObjectValue(durations);
    }

    gate->par("initiallyOpen") = schedule->slots[0].open;
    gate->par("offset") = schedule->cycleStart.dbl();

    EV_DEBUG << "Configuring gate scheduling parameters" << EV_FIELD(networkNode) << EV_FIELD(networkInterface) << EV_FIELD(gate) << EV_FIELD(schedule->slots[0].open) << EV_FIELD(schedule->cycleStart.dbl()) << EV_FIELD(durations) << EV_ENDL;

    cPar& durationsPar = gate->par("durations");
    durationsPar.copyIfShared();
    durationsPar.setObjectValue(durations);
}

void GateScheduleConfiguratorBaseNew::configureApplicationOffsets()
{
    for (auto& it : gateSchedulingOutput->applicationStartTimes) {
        auto startOffset = it.second;
        auto applicationModule = it.first->module;
        EV_DEBUG << "Setting initial packet production offset for application source" << EV_FIELD(applicationModule) << EV_FIELD(startOffset) << EV_ENDL;
        auto sourceModule = applicationModule->getSubmodule("source");
        sourceModule->par("initialProductionOffset") = startOffset.dbl();
    }
}

} // namespace inet

