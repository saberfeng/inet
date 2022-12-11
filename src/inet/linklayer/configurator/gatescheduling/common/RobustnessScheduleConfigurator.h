#ifndef __INET_ROBUSTNESSSCHEDULECONFIGURATOR_H
#define __INET_ROBUSTNESSSCHEDULECONFIGURATOR_H

#include "inet/linklayer/configurator/gatescheduling/base/GateScheduleConfiguratorBase.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <unistd.h>


namespace inet{

using std::string;
using std::unordered_map;
using std::vector;
using std::shared_ptr;
using std::make_shared;
using std::stoi;
using std::stringstream;


class INET_API RobustnessScheduleConfigurator: public GateScheduleConfiguratorBase{
protected:
    class OperationSlot{ // slot with operation
        public:
            OperationSlot(simtime_t start, simtime_t duration, bool open):
                start(start),duration(duration),open(open){}
            simtime_t start; // start time in seconds
            simtime_t duration; // duration in seconds
            bool open; // operation is open or close
    };
    using OneGateSlots = vector<OperationSlot>;
    class PortSchedule{
        public:
            PortSchedule(int gates_num):slots(gates_num){}
            simtime_t gating_cycle;
            vector<OneGateSlots> slots;
    };
    using PortId = string; // port_id: "node_from-node_to"
    using ScheduleMap = unordered_map<PortId, shared_ptr<PortSchedule>>;


    virtual Output *computeGateScheduling(const Input& input) const override;
    ScheduleMap parseScheduleInput(string input_path) const;
    void addEmptySchedule(Input::Port* port, Output* output) const;
    void addSchedule(Input::Port* port, Output* output,
            shared_ptr<PortSchedule> port_schedule) const;
    void outputGateSchedules(Output* output) const;
};


}

#endif
