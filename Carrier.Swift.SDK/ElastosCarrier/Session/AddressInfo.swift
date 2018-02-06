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

@objc(ELACarrierAddressInfo)
public class CarrierAddressInfo: NSObject {

    @objc(ELACarrierCandidateType)
    public enum CandidateType: Int, CustomStringConvertible {

        case Host

        case ServerReflexive

        case PeerReflexive

        case Relayed

        public var description: String {
            var value: String

            switch self {
            case .Host:
                value = "Host"
            case .ServerReflexive:
                value = "Server Reflexive"
            case .PeerReflexive:
                value = "Peer Reflexive"
            case .Relayed:
                value = "Relayed"
            }
            
            return value
        }
    }

    @objc(ELACarrierSocketAddress)
    public class SocketAddress: NSObject {
        public let hostname: String
        public let port: Int

        internal init(hostname: String,
                      port: Int) {
            self.hostname = hostname
            self.port = port
            super.init()
        }

        public override var description: String {
            return String(format: "hostname[%@], port[%d]",
                          String.toHardString(hostname),
                          port)
        }
    }

    public let candidateType: CandidateType
    public let address: SocketAddress
    public let relatedAddress: SocketAddress?

    internal init(candidateType: CandidateType,
                  address: String,
                  port: Int,
                  relatedAddress: String?,
                  relatedPort: Int?) {
        self.candidateType = candidateType
        self.address = SocketAddress(hostname: address, port: port)
        if relatedAddress != nil && relatedPort != nil {
            self.relatedAddress = SocketAddress(hostname: relatedAddress!, port: relatedPort!)
        }
        else {
            self.relatedAddress = nil
        }
        super.init()
    }

    internal static func format(_ info: CarrierAddressInfo) -> String {
        return String(format: "candidateType[%@], address[%@], relatedAddress[%@]",
                      info.candidateType.description,
                      String.toHardString(info.address.description),
                      String.toHardString(info.relatedAddress?.description))
    }

    public override var description: String {
        return CarrierAddressInfo.format(self)
    }
}

internal func convertCAddressInfoToCarrierAddressInfo(_ cInfo : CAddressInfo) -> CarrierAddressInfo {
    var temp = cInfo

    let candidateType = CarrierAddressInfo.CandidateType(rawValue: Int(temp.type.rawValue))!
    let address = String(cCharPointer: &temp.addr)
    let port = Int(temp.port)
    var relatedAddress : String? = nil
    var relatedPort : Int? = nil
    if temp.related_addr.0 != 0 {
        relatedAddress = String(cCharPointer: &temp.related_addr)
        relatedPort = Int(temp.related_port)
    }

    return CarrierAddressInfo(candidateType: candidateType,
                              address: address,
                              port: port,
                              relatedAddress: relatedAddress,
                              relatedPort: relatedPort)
}
