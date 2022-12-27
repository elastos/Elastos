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

/// The protocol to carrier stream instance.
///
/// Include stream status callback, stream data callback, and channel 
/// callbacks.
@objc(ELACarrierStreamDelegate)
public protocol CarrierStreamDelegate {

    /* Common callbacks */

    /// Tell the delegate that the state of carrier stream has been changed.
    ///
    /// - Parameters:
    ///   - stream: The carrier stream instance
    ///   - newState: Stream state defined in `CarrierStreamState`
    @objc(carrierStream:stateDidChange:) optional
    func streamStateDidChange(_ stream: CarrierStream,
                              _ newState: CarrierStreamState)

    /* Stream layered data callbacks */

    /// Tell the delegate that the current stream received an incomping 
    /// packet.
    ///
    /// If the stream enabled multiplexing mode, application will not
    /// receive stream-layered data callback any more. All data will 
    /// reported as multiplexing channel data.
    ///
    /// - Parameters:
    ///   - stream: The carrier stream instance
    ///   - data: The received packet data

    @objc(carrierStream:didReceiveData:) optional
    func didReceiveStreamData(_ stream: CarrierStream,
                              _ data: Data)

    /* Multiplexer callbacks */

    /// Tell the delegate that an new request within sesion to open multiplexing
    /// channel has been received.
    ///
    /// - Parameters:
    ///   - stream:  The carrier stream instance
    ///   - wantChannel: The channel ID
    ///   - cookie: Application defined string data send from remote peer.
    ///
    /// - Returns: True on success, or false if an error occurred.
    ///     The channel will continue to open only this callback return true,
    ///     otherwise the channel will be closed.

    @objc(carrierStream:shouldOpenNewChannel:withCookie:) optional
    func shouldOpenNewChannel(_ stream: CarrierStream,
                              _ wantChannel: Int,
                              _ cookie: String) -> Bool

    /// Tell the delegate that new multiplexing channel has been opened.
    ///
    /// - Parameters:
    ///   - stream:  The carrier stream instance
    ///   - newChannel: The channel ID
    @objc(carrierStream:didOpenNewChannel:) optional
    func didOpenNewChannel(_ stream: CarrierStream,
                           _ newChannel: Int)

    /// Tell the delegate that an multiplexing channel has been closed.
    ///
    /// - Parameters:
    ///   - stream:  The carrier stream instance
    ///   - channel: The channel ID
    ///   - reason: Channel close reason code, defined in `CloseReason`.
    @objc(carrierStream:didCloseChannel:withReason:) optional
    func didCloseChannel(_ stream: CarrierStream,
                         _ channel: Int,
                         _ reason: CloseReason)

    /// Tell the delegate that the channel received an incoming packet.
    ///
    /// - Parameters:
    ///   - stream:  The carrier stream instance
    ///   - channel: The channel ID
    ///   - data: The received data
    ///
    /// - Returns: True on success, or false if an error occurred.
    ///     If this callback return false, the channel will be closed
    ///     with CloseReason_Error.
    @objc(carrierStream:didReceiveDataFromChannel:data:) optional
    func didReceiveChannelData(_ stream: CarrierStream,
                               _ channel: Int,
                               _ data: Data) -> Bool

    /// Tell the delegate that the channel should pend data sending to remote
    /// peer.
    ///
    /// - Parameters:
    ///   - stream: The carrier stream instance
    ///   - channel: The channel ID
    @objc(carrierStream:channelPending:) optional
    func channelPending(_ stream: CarrierStream,
                        _ channel: Int)

    /// Tell the delegate that the channel should resume data pending to remote
    /// peer.
    ///
    /// - Parameters:
    ///   - stream: The carrier stream instance
    ///   - channel: The channel ID
    @objc(carrierStream:channelResumed:) optional
    func channelResumed(_ stream: CarrierStream,
                        _ channel: Int)
}
