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

@objc(ELACarrierTransportInfo)
public class CarrierTransportInfo: NSObject {

    public let networkTopology: CarrierNetworkTopology
    public let localAddressInfo: CarrierAddressInfo
    public let remoteAddressInfo: CarrierAddressInfo

    internal init(networkTopology : CarrierNetworkTopology,
                  localAddressInfo : CarrierAddressInfo,
                  remoteAddressInfo : CarrierAddressInfo) {
        self.networkTopology = networkTopology
        self.localAddressInfo = localAddressInfo
        self.remoteAddressInfo = remoteAddressInfo
        super.init()
    }

    internal static func format(_ info: CarrierTransportInfo) -> String {
        return String(format: "TransportInfo: networkTopology[%@]" +
                      ", localAddressInfo[%@]" +
                      ", remoteAddressInfo[%@]",
                      String(describing : info.networkTopology),
                      info.localAddressInfo,
                      info.remoteAddressInfo)
    }

    public override var description: String {
        return CarrierTransportInfo.format(self)
    }
}

internal func convertCTransportInfoToCarrierTransportInfo(_ cInfo : CTransportInfo) -> CarrierTransportInfo {
    let networkTopology = convertCNetworkTopologyToCarrierNetworkTopology(cInfo.topology)
    let localAddressInfo = convertCAddressInfoToCarrierAddressInfo(cInfo.local)
    let remoteAddressInfo = convertCAddressInfoToCarrierAddressInfo(cInfo.remote)

    return CarrierTransportInfo(networkTopology : networkTopology,
                                localAddressInfo : localAddressInfo,
                                remoteAddressInfo : remoteAddressInfo)
}
