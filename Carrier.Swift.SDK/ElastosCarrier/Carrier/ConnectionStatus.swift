import Foundation

/**
    Carrier node connection status to carrier network.
 */
@objc(ELACarrierConnectionStatus)

public enum CarrierConnectionStatus : Int, CustomStringConvertible {

    /// Carrier node connected to the network.
    /// Indicate the node is online.
    case Connected      = 0

    /// There is no connection to the network.
    /// Indicate the node is offline.
    case Disconnected   = 1

    internal static func format(_ connection: CarrierConnectionStatus) -> String {
        var value : String

        switch connection {
        case Connected:
            value = "Connected"
        case Disconnected:
            value = "Disconnected"
        }
        return value
    }

    public var description: String {
        return CarrierConnectionStatus.format(self)
    }
}

internal func convertCConnectionStatusToCarrierConnectionStatus(_ cstatus: Int32) -> CarrierConnectionStatus {
    return CarrierConnectionStatus(rawValue: Int(cstatus))!
}
