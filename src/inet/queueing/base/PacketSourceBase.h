//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETSOURCEBASE_H
#define __INET_PACKETSOURCEBASE_H

#include "inet/queueing/base/PacketProcessorBase.h"

namespace inet {
namespace queueing {

class INET_API PacketSourceBase : public PacketProcessorBase
{
  protected:
    const char *packetNameFormat = nullptr;
    const char *packetRepresentation = nullptr;
    cPar *packetLengthParameter = nullptr;
    cPar *packetDataParameter = nullptr;
    bool attachCreationTimeTag = false;
    bool attachIdentityTag = false;
    bool attachDirectionTag = false;

  protected:
    virtual void initialize(int stage) override;

    virtual std::string createPacketName(const Ptr<const Chunk>& data) const;
    virtual Ptr<Chunk> createPacketContent() const;
    virtual Packet *createPacket();
    virtual const cModule *findContainingApplication() const;
    virtual const cModule *getContainingApplication() const;
    virtual void handleParameterChange(const char *name);
};

} // namespace queueing
} // namespace inet

#endif

