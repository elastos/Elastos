import Foundation

/// Elastos carrier stream state.
///
/// The stream state will be changed according to the phase of the stream.
@objc(ELACarrierStreamState)
public enum CarrierStreamState: Int, CustomStringConvertible {
    /// Initialized stream.
    case Initialized = 1

    /// The underlying transport is ready for the stream.
    case TransportReady = 2

    /// The stream is trying to connect the remote.
    case Connecting = 3

    /// The stream connected with remove peer.
    case Connected = 4

    /// The stream is deactivated.
    case Deactivated = 5

    /// The stream closed normally.
    case Closed = 6

    /// The stream is on error, cannot to continue.
    case Error = 7


    internal static func format(_ state: CarrierStreamState) -> String {
        var value: String

        switch state {
        case Initialized:
            value = "Initialized"
        case TransportReady:
            value = "Transport ready"
        case Connecting:
            value = "Connecting"
        case Connected:
            value = "Connected"
        case Deactivated:
            value = "Deactivated"
        case Closed:
            value = "Closed"
        case Error:
            value = "Error"
        }

        return value
    }

    public var description: String {
        return CarrierStreamState.format(self)
    }
}

internal func convertCStreamStateToCarrierStreamState(_ cstate : CStreamState) -> CarrierStreamState {
    return CarrierStreamState(rawValue: Int(cstate.rawValue))!
}
