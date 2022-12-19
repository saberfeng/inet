//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMCLASSIFIER_H
#define __INET_STREAMCLASSIFIER_H

#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/common/MyHelper.h"
#include <iostream>
#include <string>

namespace inet {

using std::cout;
using std::endl;
using std::string;

class INET_API StreamClassifier : public queueing::PacketClassifierBase
{
  protected:
    const char *mode = nullptr;
    int gateIndexOffset = -1;
    int defaultGateIndex = -1;
    cValueMap *mapping = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) override;
    virtual void handleParameterChange(const char *name) override;
};

} // namespace inet

#endif

