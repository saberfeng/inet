#include "inet/linklayer/configurator/gatescheduling/common/RobustnessScheduleConfigurator.h"
#include "inet/common/MyHelper.h"
#include <fstream>
#include <vector>
#include <chrono>


namespace inet {

using std::string;
using std::ifstream;
using std::ofstream;

Define_Module(RobustnessScheduleConfigurator);


RobustnessScheduleConfigurator::Output *RobustnessScheduleConfigurator::computeGateScheduling(const Input& input) const
{
    // read input file from ned parameter
    string schedule_input = par("schedule_path");
    ScheduleMap schedule_map = parseScheduleInput(schedule_input);

    // generate output

    auto output = new Output();
    for (auto port : input.ports) {
        EV_DEBUG << "********************" << EV_ENDL;
        EV_INFO << port->module->getFullName() << EV_ENDL;
        string full_name = port->module->getFullName(); // "eth[0]"
        string startNode = port->startNode->module->getFullName(); // "d0"
        string endNode = port->endNode->module->getFullName(); // "s0"
        string port_id = startNode + "-" + endNode;
        // for ports with no schedule, open all BE gates, close all TT gates
        if(schedule_map.find(port_id) == schedule_map.end()){
            // this port has no schedule
            addEmptySchedule(port, output);
        } else {
            addSchedule(port, output, schedule_map.at(port_id));
        }
    }
    for (auto application : input.applications)
        output->applicationStartTimes[application] = 0;
    outputGateSchedules(output); // log
    return output; // TODO: at this point, s0-d0 has no schedule
}


// log imported gate schedules
void RobustnessScheduleConfigurator::outputGateSchedules(Output* output) const{
    string output_path = par("output_schedule_path");
    ofstream output_file(output_path);

    // getting current time
    auto now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);

    string content;
    stringstream ss; ss << string(std::ctime(&end_time)) << std::endl;
    content += ss.str();
    for (auto const& port_schedules: output->gateSchedules){
        auto port = port_schedules.first;
        string startNode = port->startNode->module->getFullName(); // "d0"
        string endNode = port->endNode->module->getFullName(); // "s0"
        string port_id = startNode + "-" + endNode;

        auto schedules = port_schedules.second;
        for(auto const schedule: schedules){
            stringstream line;
            line << port_id;
            line << " Idx:" << schedule->gateIndex;
            line << " Dur:" << schedule->cycleDuration * 1e6; // s to us
            line << " Start:" << schedule->cycleStart * 1e6; // s to us
            for(auto const& slot: schedule->slots){
                line << " " << slot.start * 1e6 << "-" << slot.duration * 1e6; // to us
            }
            line << std::endl;
            content += line.str();
        }
    }
    output_file << content;
    output_file.close();
}

void RobustnessScheduleConfigurator::addSchedule(
        Input::Port* port, Output* output,
        shared_ptr<PortSchedule> port_schedule) const{
    for (int gateIndex = 0; gateIndex < port->numGates; gateIndex++) {
        Output::Schedule *schedule = new Output::Schedule();
        schedule->port = port;
        schedule->gateIndex = gateIndex;
        schedule->cycleDuration = port_schedule->gating_cycle;
        schedule->cycleStart = 0; // now always start from 0
        OneGateSlots& gate_slots = port_schedule->slots[gateIndex];

        // merge adjacent slots
        vector<Output::Slot> merged_slots;
        for(int i=0; i < gate_slots.size(); i++){
            if(merged_slots.empty()){
                merged_slots.push_back(gate_slots[i]);
                continue;
            }

            Output::Slot& cur_slot = gate_slots[i];
            Output::Slot& last_merged = merged_slots[merged_slots.size()-1];
            if(cur_slot.open == last_merged.open){
                // same operation, should merge
                last_merged.duration += cur_slot.duration;
            } else {
                merged_slots.push_back(cur_slot);
            }
        }
        schedule->slots = merged_slots;
        output->gateSchedules[port].push_back(schedule);
    }
}

void RobustnessScheduleConfigurator::addEmptySchedule(
        Input::Port* port, Output* output) const{
    for (int gateIndex = 0; gateIndex < port->numGates; gateIndex++) {
        Output::Schedule *schedule = new Output::Schedule();
        schedule->port = port;
        schedule->gateIndex = gateIndex;
        schedule->cycleStart = 0;
        schedule->cycleDuration = gateCycleDuration;
        Output::Slot slot;
        slot.start = 0;
        slot.duration = gateCycleDuration;
        slot.open = true;
        schedule->slots.push_back(slot);
        output->gateSchedules[port].push_back(schedule);
    }
}

RobustnessScheduleConfigurator::ScheduleMap RobustnessScheduleConfigurator::parseScheduleInput(string input_path) const{

    // ----- debug ---------
//    char cwd[PATH_MAX];
//    if (getcwd(cwd, sizeof(cwd)) != NULL) {
//       printf("Current working dir: %s\n", cwd);
//    }

    string line;
    std::ifstream schedule_file(input_path);
    ScheduleMap schedule_map = ScheduleMap();
    while(getline(schedule_file, line)){
        vector<string> line_components = splitString(line, string(" ")); // split by space
        string node_from = line_components[1];
        string node_to = line_components[2];
        string port_id = node_from + "-" + node_to;

        int GATES_NUM = 8;
        auto port_schedule = make_shared<PortSchedule>(GATES_NUM);
        schedule_map[port_id] = port_schedule;

        port_schedule->gating_cycle = str_us_to_s(line_components[3]);
        simtime_t start = 0;
        for(int i=4; i<line_components.size();i++){
            string& raw_slot = line_components[i]; // 11111111-0-False-76.480
            if(raw_slot.empty()){
                continue;
            }
            vector<string> slot_components = splitString(raw_slot, string("-"));
            string raw_operations = slot_components[0]; // 11111111
            simtime_t duration = str_us_to_s(slot_components[3]); // 76.480
            for(int gate_idx=0; gate_idx<raw_operations.size(); gate_idx++){
                bool gate_open = stoi(raw_operations.substr(gate_idx,1));
                Output::Slot slot(start, duration, gate_open);
                port_schedule->slots[gate_idx].push_back(slot);
            }
            start += duration;
        }
    }
    schedule_file.close();
    return schedule_map;
}

} // namespace inet
