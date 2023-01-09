//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.showcases.tsn.robustness.underthehood;

import inet.networks.base.TsnNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;


network TsnNetMediumMesh extends TsnNetworkBase
{
    parameters:
        d*.eth[0].bitrate = default(100Mbps);
        s*.eth[0..3].bitrate = default(100Mbps); // [0..3] 0 and 3 are inclusive
        s*.eth[4..6].bitrate = default(1Gbps);
        @display("bgb=919,1005");
    submodules:
        d0: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=300,211");
        }
        d1: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=300,300");
        }
        s0: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=500,250");
        }
        s1: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=500,359");
        }
        d2: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=410,100");
        }
        d3: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=533,100");
        }
        s2: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=650,359");
        }
        s3: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=650,250");
        }
        d4: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=300,398");
        }
        d5: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=300,487");
        }
        d6: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=402,544");
        }
        d7: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=510,544");
        }
        d8: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=617,544");
        }
        d9: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=701,544");
        }
        d10: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=815,456");
        }
        d11: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=815,367");
        }
        d12: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=825,269");
        }
        d13: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=825,190");
        }
        d14: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=694,100");
        }
        d15: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=623,100");
        }
    connections:
        d0.ethg++ <--> EthernetLink <--> s0.ethg++;
        d1.ethg++ <--> EthernetLink <--> s0.ethg++;
        d2.ethg++ <--> EthernetLink <--> s0.ethg++;
        d3.ethg++ <--> EthernetLink <--> s0.ethg++;

        d4.ethg++ <--> EthernetLink <--> s1.ethg++;
        d5.ethg++ <--> EthernetLink <--> s1.ethg++;
        d6.ethg++ <--> EthernetLink <--> s1.ethg++;
        d7.ethg++ <--> EthernetLink <--> s1.ethg++;

        d8.ethg++ <--> EthernetLink <--> s2.ethg++;
        d9.ethg++ <--> EthernetLink <--> s2.ethg++;
        d10.ethg++ <--> EthernetLink <--> s2.ethg++;
        d11.ethg++ <--> EthernetLink <--> s2.ethg++;

        d12.ethg++ <--> EthernetLink <--> s3.ethg++;
        d13.ethg++ <--> EthernetLink <--> s3.ethg++;
        d14.ethg++ <--> EthernetLink <--> s3.ethg++;
        d15.ethg++ <--> EthernetLink <--> s3.ethg++;

        s0.ethg++ <--> EthernetLink <--> s1.ethg++;
        s1.ethg++ <--> EthernetLink <--> s3.ethg++;
        s3.ethg++ <--> EthernetLink <--> s2.ethg++;
        s2.ethg++ <--> EthernetLink <--> s0.ethg++;
        s0.ethg++ <--> EthernetLink <--> s3.ethg++;
        s1.ethg++ <--> EthernetLink <--> s2.ethg++;
}
