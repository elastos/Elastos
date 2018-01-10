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
