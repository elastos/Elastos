import Foundation

///
/// Elastos carrier stream type.
///
/// Reference:
/// https://tools.ietf.org/html/rfc4566#section-5.14
/// https://tools.ietf.org/html/rfc4566#section-8
///
@objc(ELACarrierStreamType)
public enum CarrierStreamType: Int, CustomStringConvertible {
    /// Audio stream
    case Audio       = 0

    /// Video stream
    case Video       = 1

    /// Text stream
    case Text        = 2

    /// Application stream
    case Application = 3

    /// Message stream
    case Message     = 4

    internal static func format(_ type: CarrierStreamType) -> String {
        var value: String

        switch type {
        case Audio:
            value = "Audio"
        case Video:
            value = "Video"
        case Text:
            value = "Text"
        case Application:
            value = "Application"
        case Message:
            value = "Message"
        }

        return value
    }

    public var description: String {
        return CarrierStreamType.format(self)
    }
}

internal func convertCarrierStreamTypeToCStreamType(_ type : CarrierStreamType) -> CStreamType {
    return CStreamType(rawValue: UInt32(type.rawValue))
}

internal func convertCStreamTypeToCarrierStreamType(_ ctype : CStreamType) -> CarrierStreamType {
    return CarrierStreamType(rawValue: Int(ctype.rawValue))!
}
