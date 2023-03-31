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

package inet.showcases.tsn.robustness;

import inet.networks.base.TsnNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

network TsnNetSmallTree extends TsnNetworkBase
{
    parameters:
        d*.eth[0].bitrate = default(100Mbps);
        s*.eth[0..3].bitrate = default(100Mbps);
    submodules:
        d0: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=300,150");
        }
        d1: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=300,350");
        }
        d2: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=900,150");
        }
        d3: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=900,350");
        }
        s0: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=607,247");
        }
    connections:
        d0.ethg++ <--> EthernetLink <--> s0.ethg++;
        d1.ethg++ <--> EthernetLink <--> s0.ethg++;
        d2.ethg++ <--> EthernetLink <--> s0.ethg++;
        d3.ethg++ <--> EthernetLink <--> s0.ethg++;
}
