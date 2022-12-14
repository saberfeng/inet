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

package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// This module drops packets based on the GCL: dropping packets when the gate is closed
//
simple RobustnessDropper extends PacketFilterBase like IPacketFilter
{
    parameters:
        int hypercycle = default(nan); // hypercycle in ns 
        string ingressWindows = default(""); // windows in ns, example: "201000-322000 701000-822000"
        string globalSafeIntervals = default(""); // intervals in ns, example: "134000-321000 321000-322000 322000-821000"
        @class(RobustnessDropper);
}