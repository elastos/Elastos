/*
 * Copyright (c) 2018 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

import Foundation

@objc(ELACarrierNetworkTopology)
public enum CarrierNetworkTopology: Int, CustomStringConvertible {

    case LAN

    case P2P

    case Relayed

    internal static func format(_ type: CarrierNetworkTopology) -> String {
        var value: String

        switch type {
        case .LAN:
            value = "LAN"
        case .P2P:
            value = "P2P"
        case .Relayed:
            value = "Relayed"
        }

        return value
    }

    public var description: String {
        return CarrierNetworkTopology.format(self)
    }
}

internal func convertCNetworkTopologyToCarrierNetworkTopology(_ cTopology : CNetworkTopology) -> CarrierNetworkTopology {
    return CarrierNetworkTopology(rawValue: Int(cTopology.rawValue))!
}
