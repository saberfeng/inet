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
simple GCLBasedDropper extends PacketFilterBase like IPacketFilter
{
    parameters:
        string durationsVector = default(""); // the dropsVector format is "<packetNumber>;<packetNumber>;...;"
                                          // packet numbers in ascending order e.g. "5;6;12;13;" 
                                          // corresponding to actions               "open;close;open;close;"
        @class(GCLBasedDropper);
}