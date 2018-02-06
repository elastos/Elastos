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
