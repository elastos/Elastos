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
