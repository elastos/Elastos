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

/**
    The protocol to Carrier group instance.
 */
@objc(ELACarrierGroupDelegate)
public protocol CarrierGroupDelegate {

    /// Tell the delegate that Carrier node will be connected to the group.
    ///
    /// - Parameters:
    ///   - group: Group group instance.
    ///
    /// - Returns: Void
    @objc(carrierGroupDidConnect:) optional
    func groupDidConnect(_ group: CarrierGroup)

    /// Tell the delegate that an group message has been received.
    ///
    /// - Parameters:
    ///   - group: Carrier group instance
    ///   - from: The peer from who send the message
    ///   - message: The message content
    ///
    /// - Returns: Void
    @objc(carrierGroup:didReceiveMessageFromPeer:data:) optional
    func didReceiveGroupMessage(_ group: CarrierGroup,
                                _ from: String,
                                _ data: Data)

    /// Tell the delegate that group title has been changed.
    ///
    /// - Parameters:
    ///     - group: Carrier group instance
    ///     - from: The peer who change the group title
    ///     - newTitle: The new group title
    ///
    /// - Returns: Void
    @objc(carrierGroup:titleChangedByPeer:newTitle:) optional
    func groupTitleDidChange(_ group: CarrierGroup,
                             _ from: String,
                             _ newTitle: String)

    /// Tell the delegate that group peer name has been changed.
    ///
    /// - Parameters:
    ///     - group: Carrier group instance
    ///     - from: The peer who change it's name
    ///     - newName: The new peer name
    ///
    /// - Returns: Void
    @objc(carrierGroup:peer:nameDidChange:) optional
    func groupPeerNameDidChange(_ group: CarrierGroup,
                                _ from: String,
                                _ newName: String)

    /// Tell the delegate that group peer list has been changed.
    ///
    /// - Parameters:
    ///     - group: Carrier group instance
    ///
    /// - Returns: Void
    @objc(carrierGroupPeerListDidChange:) optional
    func groupPeerListDidChange(_ group: CarrierGroup)
}
