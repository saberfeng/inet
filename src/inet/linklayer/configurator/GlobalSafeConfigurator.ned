package inet.linklayer.configurator;

import inet.networklayer.configurator.contract.INetworkConfigurator;

simple GlobalSafeConfigurator like INetworkConfigurator
{
    parameters:
        string globalSafeInput = default(""); // the input file path
        string ingressScheduleInput = default(""); // the input file path
        @class(GlobalSafeConfigurator);
}