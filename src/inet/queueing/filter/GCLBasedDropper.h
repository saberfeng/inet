/*
 * GCLBasedDropper.cc
 *
 *  Created on: Oct 5, 2022
 *      Author: xingbo
 */

#ifndef __INET_GCLBASEDDROPPER_H
#define __INET_GCLBASEDDROPPER_H

#include <vector>
#include <string>
#include "inet/queueing/base/PacketFilterBase.h"
using namespace std;

namespace inet{
namespace queueing{

/**
 * GCL Based Dropper module
 */
class INET_API GCLBasedDropper : public PacketFilterBase
{
protected:
    unsigned int numPackets;
    unsigned int numDropped;
    std::vector<unsigned int> durationsVector;

    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;
    virtual void parseGlobalSafeInput(std::string input);

public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

}
}

#endif
