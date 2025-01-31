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

package inet.queueing.server;
import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;


simple RcvDelaySignalServer extends PacketServerBase like IPacketServer
{
    parameters:
        displayStringTextFormat = default("served %p pk (%l)\n%s");
        int serveSchedulingPriority = default(-1); // specifies the FES scheduling priority for the extra event that is pulling the packet, -1 means no extra event
        volatile double delayLength @mutable @unit(s) = default(0ms); // determines additional delay per packet
        volatile double effectStartTime @mutable @unit(s) = default(0ms);
        volatile int numDelayPackets @mutable = default(0);
        volatile double processingBitrate @unit(bps) = default(inf bps); // determines additional processing time per bit
        volatile bool initiallyOpen @mutable = default(true); // specifies if the gate is initially open or closed
        volatile object durations @mutable @unit(s) = default([]); // list of time durations within the period that determine if the gate is open or closed; period is the sum of the durations
        volatile double cycleDuration @mutable @unit(s) = default(0s);
        @class(RcvDelaySignalServer);
}