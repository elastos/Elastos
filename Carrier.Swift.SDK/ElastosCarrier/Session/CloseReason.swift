import Foundation

/// Multiplexing channel close reason mode.
@objc(ELACloseReason)
public enum CloseReason: Int, CustomStringConvertible {
    /// Channel closed normally.
    case Normal  = 0

    /// Channel closed because timeout.
    case Timeout = 1

    /// Channel closed because error occured.
    case Error   = 2

    internal static func format(_ reason: CloseReason) -> String {
        var value: String

        switch reason {
        case Normal:
            value = "Normal"
        case Timeout:
            value = "Timeout"
        case Error:
            value = "Error"
        }

        return value
    }

    public var description: String {
        return CloseReason.format(self)
    }
}

internal func convertCCloseReasonToCloseReason(_ creason : CCloseReason) -> CloseReason {
    return CloseReason(rawValue: Int(creason.rawValue))!
}
