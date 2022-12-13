#ifndef __INET_GLOBALSAFECONFIGURATOR_H
#define __INET_GLOBALSAFECONFIGURATOR_H

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"
#include <string>
#include <iostream>

namespace inet{

using std::string;
using std::cout;
using std::endl;

class INET_API GlobalSafeConfigurator : public NetworkConfiguratorBase{
    protected:
        string globalSafeInput;

        virtual void initialize(int stage) override;
        virtual void parseInputFile();
        /**
         * Computes the network configuration for all nodes in the network.
         * The result of the computation is only stored in the configurator.
         */

};

}


#endif