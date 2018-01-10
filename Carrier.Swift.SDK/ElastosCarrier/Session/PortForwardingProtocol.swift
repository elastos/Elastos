import Foundation

/// Port forwarding supported protocols.
@objc(ELAPortForwardingProtocol)
public enum PortForwardingProtocol: Int, CustomStringConvertible {
    /// TCP protocol.
    case TCP = 1

    internal static func format(_ proto: PortForwardingProtocol) -> String {
        var value: String

        switch proto {
        case TCP:
            value = "tcp"
        }

        return value
    }

    public var description: String {
        return PortForwardingProtocol.format(self)
    }
}

internal func convertPortForwardingProtocolToCPortForwardingProtocol(
    _ proto : PortForwardingProtocol) -> CPortForwardingProtocol {
    return CPortForwardingProtocol(rawValue: UInt32(proto.rawValue))
}
