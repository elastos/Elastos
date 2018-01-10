import Foundation

/**
    Carrier node presence status to friends
 */
@objc(ELACarrierConnectionStatus)

public enum CarrierPresenceStatus : Int, CustomStringConvertible {

    /// Carrier node in online or available to friends.
    case None = 0

    /// Carrier node is being away.
    case Away = 1

    /// Carrier node is being busy.
    case Busy = 2

    internal static func format(_ presence: CarrierPresenceStatus) -> String {
        var value : String

        switch presence {
        case None:
            value = "None"
        case Away:
            value = "Away"
        case Busy:
            value = "Busy"
        }
        return value
    }

    public var description: String {
        return CarrierPresenceStatus.format(self)
    }
}

internal func convertCPresenceStatusToCarrierPresenceStatus(_ cstatus: Int32) -> CarrierPresenceStatus {
    return CarrierPresenceStatus(rawValue: Int(cstatus))!
}

internal func convertCarrierPresenceStatusToCPresenceStatus(_ status: CarrierPresenceStatus) -> CPresenceStatus {
    return CPresenceStatus(Int32(status.rawValue))
}
