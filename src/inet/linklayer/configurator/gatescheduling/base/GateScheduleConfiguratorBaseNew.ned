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

package inet.linklayer.configurator.gatescheduling.base;

simple GateScheduleConfiguratorBaseNew
{
    parameters:
        double gateCycleDuration @unit(s); // the globally used gate scheduling period in each PeriodicGate modules
        object configuration @mutable = default([]); // a vector of objects (e.g. [{...}, {...}]), see above
                                                     // here is an example: [{name: "s1", type: "unicast", application: "app[0]", pcp: 0, gateIndex: 0, source: "wheel*", destination: "hud", packetLength: 100B, packetInterval: 5ms, maxLatency: 100us, maxJitter: 10us}]
        @display("i=block/cogwheel");
}