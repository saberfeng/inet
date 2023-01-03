//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks.base;

import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;
import inet.networklayer.configurator.contract.INetworkConfigurator;
import inet.queueing.contract.ISignalSource;

//
// This module serves as a network base module for Time-Sensitive Networking (TSN).
//
network TsnNetworkBase extends WiredNetworkBase
{
    parameters:
        **.clock.defaultOverdueClockEventHandlingMode = default("execute");
    submodules:
        gateScheduleConfigurator: <default("")> like IGateScheduleConfigurator if typename != "" {
            @display("p=100,500;is=s");
        }
        streamRedundancyConfigurator: <default("")> like INetworkConfigurator if typename != "" {
            @display("p=100,600;is=s");
        }
        failureProtectionConfigurator: <default("")> like INetworkConfigurator if typename != "" {
            @display("p=100,700;is=s");
        }
        globalSafeConfigurator: <default("")> like IGateScheduleConfigurator if typename != "" {
            @display("p=100,800;is=s");
        }
        delaySignalSource: <default("DelaySignalSource")> like ISignalSource {
            @display("p=100,900;is=s");
        }
}
