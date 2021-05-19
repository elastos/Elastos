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

/// Elastos carrier file transfer connection state.
///
/// The filetransfer connection state will be changed according to the phase of
//  the filetransfer instance.
@objc(ELACarrierFileTransferConnectionState)
public enum CarrierFileTransferConnectionState: Int, CustomStringConvertible {
    /// The file transfer connection is initialized.
    case Initialized = 1

    /// The file transfer connection is connecting to remote peer.
    case Connecting = 2

    /// The file transfer connection is being established.
    case Connected = 3

    /// The file transfer connection is closed and disconnected.
    case Closed = 4

    /// The file transfer connection failed with some reason.
    case Error = 5

    internal static func format(_ state: CarrierFileTransferConnectionState) -> String {
        var value: String

        switch state {
        case Initialized:
            value = "Initialized"
        case Connecting:
            value = "Connecting"
        case Connected:
            value = "Connected"
        case Closed:
            value = "Closed"
        case Error:
            value = "Error"
        }

        return value
    }

    public var description: String {
        return CarrierFileTransferConnectionState.format(self)
    }
}

internal func convertCFileTransferConnectionToCarrierFileTransferConnection (
        _ cstate : CFileTransferConnection) -> CarrierFileTransferConnectionState {
    return CarrierFileTransferConnectionState(rawValue: Int(cstate.rawValue))!
}
