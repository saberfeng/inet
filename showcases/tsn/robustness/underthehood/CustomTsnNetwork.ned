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

network CustomTsnNetwork extends TsnNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(100Mbps);
    submodules:
        d0: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=300,150");
        }
        d1: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=300,350");
        }
        s0: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=500,250");
        }
        s1: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=700,250");
        }
        d2: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=900,150");
        }
        d3: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=900,350");
        }
    connections:
        d0.ethg++ <--> EthernetLink <--> s0.ethg++;
        d1.ethg++ <--> EthernetLink <--> s0.ethg++;
        s1.ethg++ <--> EthernetLink <--> d2.ethg++;
        s1.ethg++ <--> EthernetLink <--> d3.ethg++;
        s0.ethg++ <--> EthernetLink <--> s1.ethg++;
}
