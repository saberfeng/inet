package inet.linklayer.configurator;

import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;

simple GlobalSafeConfigurator like IGateScheduleConfigurator
{
    parameters:
        string globalSafeInput = default(""); // the input file path
        string ingressScheduleInput = default(""); // the input file path
        @class(GlobalSafeConfigurator);
}