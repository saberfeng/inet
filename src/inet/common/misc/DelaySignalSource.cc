
#include "inet/common/misc/DelaySignalSource.h"

namespace inet {

Define_Module(DelaySignalSource);

void DelaySignalSource::initialize()
{
    startTime = par("startTime");
//    endTime = par("endTime");
    numActionToDo = par("numActionToDo");
    scheduleAt(startTime, new cMessage("timer"));
    collectApplicableDevices();

    std::random_device rd;
    randGenerator = std::mt19937(rd());
    distribution = std::uniform_int_distribution<>(0, delaySvrs.size() - 1);
}

//void DelaySignalSource::collectApplicableDevices(){
//    cModule* net = this->findModuleByPath(getNetName(this).c_str());
//    for (cModule::SubmoduleIterator it(net); !it.end(); ++it) {
//        cModule *device = *it;
//        string deviceName = device->getName(); // debug
//        if (!isNetworkDeviceName(device->getName()))
//            continue; // skip non-device modules
//
//        for (cModule::SubmoduleIterator ethIter(device); !ethIter.end(); ++ethIter) {
//            cModule *ethModule = *ethIter;
//            string tmp1(ethModule->getClassAndFullPath()); // debug
//            if (strcmp(ethModule->getModuleType()->getName(), "LayeredEthernetInterface") == 0) {
//                cModule* transGate7th = ethModule->getSubmodule("macLayer")
//                                                 ->getSubmodule("queue")
//                                                 ->getSubmodule("transmissionGate", 7);
//
//                cValueArray* durations = check_and_cast<cValueArray *>(transGate7th->par("durations").objectValue());
//                auto doubleV = durations->asDoubleVector();
//
//                if(durations->size() == 2 &&
//                   durations->get(0).doubleValue() == 0.001 &&
//                   durations->get(1).doubleValue() == 0.){
//                    continue; // always open, [1ms,0ms]
//                } else {
//                    ethWithSched.push_back(ethModule); // has configured egress schedule
//                }
//
//            }
//        }
//    }
//}

void DelaySignalSource::collectApplicableDevices(){
    cModule* net = this->findModuleByPath(getNetName(this).c_str());
    for (cModule::SubmoduleIterator it(net); !it.end(); ++it) {
        cModule *device = *it;
        string deviceName = device->getName(); // debug
        if (!isNetworkDeviceName(device->getName()))
            continue; // skip non-device modules

        int ethSize =device->getSubmoduleVectorSize("eth");
        for(int i = 0; i < ethSize; i++){
            cModule* delaySvrMod = device->getSubmodule("eth", i)
                                         ->getSubmodule("macLayer")
                                         ->getSubmodule("server");
            delaySvrs.push_back(delaySvrMod);
//            std::cout << delaySvrMod->getClassAndFullPath() << std::endl;
        }
    }
}

void DelaySignalSource::handleMessage(cMessage *msg)
{
    // numActionToDo == -1 : repeat signal forever
    if (numActionToDo < 0 || numActionDone < numActionToDo) {

        cModule* randServer = delaySvrs[distribution(randGenerator)];

        std::cout << "#" << numActionDone << " random server picked:" << randServer->getFullPath()
                  << " simtime:" << simTime() << std::endl;
        randServer->par("effectStartTime") = simTime().dbl(); // apply delay from now
//        targetModule->par("effectDuration") = 0.001; // this change expire after 1000us
        randServer->par("numDelayPackets") = 1; // this change expire after 1000us
        randServer->par("delayLength") = 0.000001; // 1us

        numActionDone++;
        scheduleAfter(par("interval"), msg);
    }
    else {
        delete msg;
    }
}

void DelaySignalSource::finish()
{
}

} // namespace inet

