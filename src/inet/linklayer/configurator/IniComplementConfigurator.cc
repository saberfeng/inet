#include "inet/linklayer/configurator/IniComplementConfigurator.h"


namespace inet{

Define_Module(IniComplementConfigurator);

void IniComplementConfigurator::initialize(int stage){
    if (stage == INITSTAGE_QUEUEING) {
        configure();
    }
}

//void IniComplementConfigurator::skipEmptyLine(ifstream& inputFile){
//    string line;
//    while(getline(inputFile, line)){
//
//    }
//}

string IniComplementConfigurator::getNetName(){
    string path = this->getFullPath(); // TsnNetSmallRing.iniComplementConfigurator
    return splitString(path, ".")[0];
}

void IniComplementConfigurator::configureClientApps(vector<string>& lines){
    for(const auto& line : lines){
        vector<string> elements = splitString(line, ",");
        assert(elements.size() >= 8);

        string srcDevice = elements[0];
        int srcAppIdx = stoi(elements[1]);
        string displayName = elements[2];
        string dstAddress = elements[3];
        int dstPort = stoi(elements[4]);
        int packetLength = stoi(elements[5]);
        double prodInterval = std::stod(elements[6]) / 1e6; // us -> s
        double initOffset = std::stod(elements[7]) / 1e6; // us -> s

        stringstream ssApp;
        ssApp << getNetName() << "." << srcDevice << ".app[" << srcAppIdx << "]";
        cModule* application = this->findModuleByPath(ssApp.str().c_str());

        string ioPath = ssApp.str() + ".io";
        cModule* appIo = this->findModuleByPath(ioPath.c_str());

        string sourcePath = ssApp.str() + ".source";
        cModule* appSource = this->findModuleByPath(sourcePath.c_str());

        application->setDisplayName(displayName.c_str());
        appIo->par("destAddress") = dstAddress;
        appIo->par("destPort") = dstPort;
        appSource->par("packetLength") = packetLength;
        appSource->par("productionInterval") = prodInterval;
        appSource->par("initialProductionOffset") = initOffset;
    }
}

void IniComplementConfigurator::configureServerApps(vector<string>& lines){
    for(const auto& line : lines){
        vector<string> elements = splitString(line, ",");
        assert(elements.size() >= 4);

        string dstDevice = elements[0];
        int dstAppIdx = stoi(elements[1]);
        string displayName = elements[2];
        int localPort = stoi(elements[3]);

        stringstream ssApp;
        ssApp << getNetName() << "." << dstDevice << ".app[" << dstAppIdx << "]";
        cModule* application = this->findModuleByPath(ssApp.str().c_str());

        string ioPath = ssApp.str() + ".io";
        cModule* appIo = this->findModuleByPath(ioPath.c_str());

        application->setDisplayName(displayName.c_str());
        appIo->par("localPort") = localPort;
    }
}

//stream, pcp
//FG0,7
//FG1,7
void IniComplementConfigurator::configureEncDecApps(vector<string>& lines){
    cValueArray* encDecMaps = new cValueArray();
    for(const auto& line : lines){
        vector<string> elements = splitString(line, ",");
        assert(elements.size() >= 2);
        string stream = elements[0];
        int pcp = stoi(elements[1]);

        cValueMap* encDecMap = new cValueMap();
        encDecMap->set("stream", stream);
        encDecMap->set("pcp", pcp);
        encDecMaps->add(encDecMap);
    }

    // iterate through all devices with a streamCoder
    cModule* net = this->findModuleByPath(getNetName().c_str());
    for (cModule::SubmoduleIterator it(net); !it.end(); ++it) {
        cModule *sub = *it;
        string subFullClassName = sub->getNedTypeAndFullName();
        vector<string> splitted = splitString(subFullClassName, ".");
        string subClassName = splitted[splitted.size()-1];
        if(subClassName == "TsnDevice" || subClassName == "TsnSwitch"){
            string subName = sub->getFullName();
            stringstream ss;
            ss << getNetName() << "." << subName << ".bridging.streamCoder.encoder";
            cModule* encoder = this->findModuleByPath(ss.str().c_str());
            encoder->par("mapping") = encDecMaps;

            ss << getNetName() << "." << subName << ".bridging.streamCoder.decoder";
            cModule* decoder = this->findModuleByPath(ss.str().c_str());
            decoder->par("mapping") = encDecMaps;
        }
    }
}

//stream, destPort
//FG0,1000
//FG1,1001
void IniComplementConfigurator::configureStreamIdenApps(vector<string>& lines){
    cValueArray* identMaps = new cValueArray();
    for(const auto& line : lines){
        vector<string> elements = splitString(line, ",");
        assert(elements.size() >= 2);
        string stream = elements[0];
        string destPort = elements[1];

        cValueMap* identMap = new cValueMap();
        identMap->set("stream", stream);

        cDynamicExpression expr = cDynamicExpression();
        string pattern = string("udp.destPort == ") + destPort;
        expr.parse(pattern.c_str());
        identMap->set("packetFilter", expr);
        identMaps->add(identMap);
    }

    // iterate through all end-hosts
    cModule* net = this->findModuleByPath(getNetName().c_str());
    for (cModule::SubmoduleIterator it(net); !it.end(); ++it) {
        cModule *sub = *it;
        string subFullClassName = sub->getNedTypeAndFullName();
        vector<string> splitted = splitString(subFullClassName, ".");
        string subClassName = splitted[splitted.size()-1];
        if(subClassName == "TsnDevice"){
            string subName = sub->getFullName();
            stringstream ss;
            ss << getNetName() << "." << subName << ".bridging.streamIdentifier.identifier";
            cModule* identifier = this->findModuleByPath(ss.str().c_str());
            identifier->par("mapping") = identMaps;
        }
    }
}


void IniComplementConfigurator::configureGateSchedApps(vector<string>& lines){
    for(const auto& line : lines){
        vector<string> elements = splitString(line, ",");
        assert(elements.size() >= 8);

    }
}

void IniComplementConfigurator::configureWrapper(
        vector<string>& lines, string& confName){
    if(confName == "client_app_conf"){
        configureClientApps(lines);
    } else if (confName == "server_app_conf"){
        configureServerApps(lines);
    } else if (confName == "enc_dec"){
        configureEncDecApps(lines);
    } else if (confName == "stream_identifier"){
        configureStreamIdenApps(lines);
    } else if (confName == "gate_schedule"){
        configureGateSchedApps(lines);
    } else {
        throw "invalid confName";
    }
}

void IniComplementConfigurator::parseHeader(
        string& header, string& confName, int& lineNum){
    vector<string> splitted = splitString(header, ",");
    assert(splitted.size()>=3);
    confName = splitted[1];
    vector<string> splittedLineNum = splitString(splitted[2], ":");
    lineNum =stoi(splittedLineNum[1]);
}

void IniComplementConfigurator::configure(){
    string inputPath = par("iniComplementTxt");
    std::ifstream inputFile(inputPath);
    string line;
    getline(inputFile, line);

    while(inputFile){
        assert(line[0]==',');
        string confName;
        int lineNum;
        parseHeader(line, confName, lineNum);
        getline(inputFile, line);
        vector<string> lines;
        for(int i = 0; i < lineNum; i++){
            getline(inputFile, line);
            lines.push_back(line);
        }

        configureWrapper(lines, confName);

        while(getline(inputFile, line)){
            if(!line.empty()){
                break;
            }
        }
    }


}

}
