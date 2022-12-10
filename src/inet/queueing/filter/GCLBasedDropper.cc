/*
 * GCLBasedDropper.cc
 *
 *  Created on: Oct 5, 2022
 *      Author: xingbo
 */

#include "inet/queueing/filter/GCLBasedDropper.h"

#include "inet/common/ModuleAccess.h"
#include <string>
#include <fstream>



namespace inet {
namespace queueing {

Define_Module(GCLBasedDropper);


void GCLBasedDropper::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numPackets = 0;
        numDropped = 0;

        WATCH(numPackets);
        WATCH(numDropped);

//        const char *vector = par("global_safe_input");
        string global_safe_input = par("global_safe_input");
        parseGlobalSafeInput(global_safe_input);

//        if (durationsVector.size() == 0)
//            EV_WARN << "Empty dropsVector" << EV_ENDL;
//        else {
//            EV_DEBUG << EV_FIELD(dropsVector, vector) << EV_ENDL;
//        }
    }
}

void GCLBasedDropper::parseGlobalSafeInput(string input_path){
    string line;
    std::ifstream input_file(input_path);
//    while(getline(input_file,line)){
//
//    }
    input_file.close();
}

bool GCLBasedDropper::matchesPacket(const Packet *packet) const
{
//    return !generateFurtherDrops || numPackets != dropsVector[0];
    return true;
}


void GCLBasedDropper::dropPacket(Packet *packet)
{
    EV_DEBUG << "Dropping packet" << EV_FIELD(ordinalNumber, numPackets) << EV_FIELD(packet) << EV_ENDL;
    numPackets++;
    numDropped++;
//    dropsVector.erase(dropsVector.begin());
//    if (dropsVector.size() == 0) {
//        EV_DEBUG << "End of dropsVector reached" << EV_ENDL;
//        generateFurtherDrops = false;
//    }
    PacketFilterBase::dropPacket(packet);
}

void GCLBasedDropper::processPacket(Packet *packet)
{
    // numPackets++;
}


}
}
