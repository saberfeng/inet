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


network TsnNetIndustrial extends TsnNetworkBase
{
    parameters:
        // parameter assignments can not be overwritten, can't write below in a simple way
        d*.eth[0].bitrate = default(1Gbps);
        
        s0.eth[1..2].bitrate = default(10Gbps);
        s1.eth[1..2].bitrate = default(10Gbps);
        s2.eth[1..2].bitrate = default(10Gbps);
        s3.eth[1..2].bitrate = default(10Gbps);
        
        s0.eth[0].bitrate = default(1Gbps);
        s1.eth[0].bitrate = default(1Gbps);
        s2.eth[0].bitrate = default(1Gbps);
        s3.eth[0].bitrate = default(1Gbps);
        
        s2.eth[3..4].bitrate = default(1Gbps);
        s3.eth[3..4].bitrate = default(1Gbps);
        
        s4.eth[*].bitrate = default(1Gbps);
        s5.eth[*].bitrate = default(1Gbps);
        s6.eth[*].bitrate = default(1Gbps);
        s7.eth[*].bitrate = default(1Gbps);
        s8.eth[*].bitrate = default(1Gbps);
        s9.eth[*].bitrate = default(1Gbps);
        
        s10.eth[*].bitrate = default(1Gbps);
        s11.eth[*].bitrate = default(1Gbps);
        s12.eth[*].bitrate = default(1Gbps);
        s13.eth[*].bitrate = default(1Gbps);
        s14.eth[*].bitrate = default(1Gbps);
        s15.eth[*].bitrate = default(1Gbps);
        @display("bgb=1709.246,1346.006");
    submodules:
        d0: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=676.02997,262.34");
        }
        d1: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=1117.972,262.34");
        }
        s0: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=817.29,262.34");
        }
        s1: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=968.64,262.34");
        }
        d2: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=676.02997,415.708");
        }
        d3: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=1117.972,415.708");
        }
        s2: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=817.29,415.708");
        }
        s3: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=968.64,415.708");
        }
        d4: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=409.654,635.67");
        }
        d5: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=801.146,635.67");
        }
        d6: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=409.654,789.038");
        }
        d7: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=801.146,789.038");
        }
        d8: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=409.654,950.47797");
        }
        d9: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=801.146,950.47797");
        }
        s4: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=522.662,635.67");
        }
        s5: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=674.012,635.67");
        }
        s6: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=522.662,789.038");
        }
        s7: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=674.012,789.038");
        }
        s8: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=522.662,950.47797");
        }
        s9: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=674.012,950.47797");
        }
        d10: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=1004.964,635.67");
        }
        d11: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=1396.4559,635.67");
        }
        d12: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=1004.964,789.038");
        }
        d13: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=1396.4559,789.038");
        }
        d14: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=1004.964,950.47797");
        }
        d15: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=1396.4559,950.47797");
        }
        s10: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=1117.972,635.67");
        }
        s11: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=1269.322,635.67");
        }
        s12: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=1117.972,789.038");
        }
        s13: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=1269.322,789.038");
        }
        s14: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=1117.972,950.47797");
        }
        s15: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=1269.322,950.47797");
        }
    connections:
        d0.ethg++ <--> EthernetLink <--> s0.ethg++;
        d1.ethg++ <--> EthernetLink <--> s1.ethg++;
        d2.ethg++ <--> EthernetLink <--> s2.ethg++;
        d3.ethg++ <--> EthernetLink <--> s3.ethg++;

        d4.ethg++ <--> EthernetLink <--> s4.ethg++;
        d5.ethg++ <--> EthernetLink <--> s5.ethg++;
        d6.ethg++ <--> EthernetLink <--> s6.ethg++;
        d7.ethg++ <--> EthernetLink <--> s7.ethg++;
        d8.ethg++ <--> EthernetLink <--> s8.ethg++;
        d9.ethg++ <--> EthernetLink <--> s9.ethg++;
        
        d10.ethg++ <--> EthernetLink <--> s10.ethg++;
        d11.ethg++ <--> EthernetLink <--> s11.ethg++;
        d12.ethg++ <--> EthernetLink <--> s12.ethg++;
        d13.ethg++ <--> EthernetLink <--> s13.ethg++;
        d14.ethg++ <--> EthernetLink <--> s14.ethg++;
        d15.ethg++ <--> EthernetLink <--> s15.ethg++;
        
        s0.ethg++ <--> EthernetLink <--> s1.ethg++;
        s1.ethg++ <--> EthernetLink <--> s3.ethg++;
        s3.ethg++ <--> EthernetLink <--> s2.ethg++;
        s2.ethg++ <--> EthernetLink <--> s0.ethg++;
        
        s2.ethg++ <--> EthernetLink <--> s4.ethg++;
        s2.ethg++ <--> EthernetLink <--> s10.ethg++;
        s3.ethg++ <--> EthernetLink <--> s5.ethg++;
        s3.ethg++ <--> EthernetLink <--> s11.ethg++;
        
        s4.ethg++ <--> EthernetLink <--> s5.ethg++;
        s4.ethg++ <--> EthernetLink <--> s6.ethg++;
        s5.ethg++ <--> EthernetLink <--> s7.ethg++;
        s6.ethg++ <--> EthernetLink <--> s8.ethg++;
        s7.ethg++ <--> EthernetLink <--> s9.ethg++;
        s8.ethg++ <--> EthernetLink <--> s9.ethg++;
        
        s10.ethg++ <--> EthernetLink <--> s11.ethg++;
        s10.ethg++ <--> EthernetLink <--> s12.ethg++;
        s11.ethg++ <--> EthernetLink <--> s13.ethg++;
        s12.ethg++ <--> EthernetLink <--> s14.ethg++;
        s13.ethg++ <--> EthernetLink <--> s15.ethg++;
        s14.ethg++ <--> EthernetLink <--> s15.ethg++;
}
