#include "inet/linklayer/configurator/GlobalSafeConfigurator.h"

namespace inet{

Define_Module(GlobalSafeConfigurator);

void GlobalSafeConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        string inputFile = par("globalSafeInput");
        globalSafeInput = inputFile;
    }
    if (stage == INITSTAGE_QUEUEING) {
        parseInputFile();
    }
}

void GlobalSafeConfigurator::parseInputFile(){
    cout<<"**************" << globalSafeInput << endl;
}


}