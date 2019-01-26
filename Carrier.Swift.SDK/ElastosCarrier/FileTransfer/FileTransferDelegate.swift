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
    The protocol to Carrier file transfer instance.
 */
@objc(ELACarrierFileTransferDelegate)
public protocol CarrierFileTransferDelegate {
    /* Common callbacks */

    /// Tell the delegate that the state of carrier filetransfer has been changed.
    ///
    /// - Parameters:
    ///   - newState: Stream state defined in `CarrierStreamState`
    @objc(carrierFileTransfer:stateDidChange:) optional
    func fileTransferStateDidChange(_ fileTransfer: CarrierFileTransfer,
                                    _ newState: CarrierFileTransferConnectionState)

    /// Tell the delegate that the current filetransfer received an incoming
    /// file request packet.
    ///
    /// - Parameters:
    ///   - fileId: The file identififer.
    ///   - fileName: The file name to transfer.
    ///   - fileSize: The total size of file.
    @objc(carrierFileTransfer:didReceiveFileRequestWithFileId:fileName:size:) optional
    func didReceiveFileRequest(_ fileTransfer: CarrierFileTransfer,
                               _ fileId: String,
                               _ fileName: String,
                               _ fileSize: UInt64)

    /// Tell the delegate that the current filetransfer received an incoming
    /// pull request packet.
    ///
    /// - Parameters:
    ///   - fileId: The file identififer.
    ///   - stream: The offset of file where transfer begins.
    @objc(carrierFileTransfer:didReceivePullRequestWithFileId:offset:) optional
    func didReceivePullRequest(_ fileTransfer: CarrierFileTransfer,
                               _ fileId: String,
                               _ offset: UInt64)

    /// Tell the delegate that the current filetransfer received an incoming
    /// data packet.
    ///
    /// - Parameters:
    ///   - fileId: The file identififer.
    ///   - data:   The received data.
    ///
    /// - Returns:
    ///   TODO:
    @objc(carrierFileTransfer:didReceiveDataWithFileId:data:) optional
    func didReceiveFileTransferData(_ fileTransfer: CarrierFileTransfer,
                                    _ fileId: String,
                                    _ data: Data) -> Bool

    /// Tell the delegate that the filetransfer should pend data sending to
    /// remote peer.
    ///
    /// - Parameters:
    ///   - fileId: The file identifier
    @objc(carrierFileTransfer:pendingTransferFile:) optional
    func fileTransferPending(_ fileTransfer: CarrierFileTransfer,
                             _ fileId: String)

    /// Tell the delegate that the filetransfer should resume data pending
    /// to remote peer.
    ///
    /// - Parameters:
    ///   - fileId: The file identifier
    @objc(carrierFileTransfer:resumedTransferFile:) optional
    func fileTransferResumed(_ fileTransfer: CarrierFileTransfer,
                             _ fileId: String)

    /// Tell the delegate that the current filetransfer received an incoming
    /// data packet.
    ///
    /// - Parameters:
    ///   - fileId: The file identififer.
    ///   - data:   The received data.
    @objc(carrierFileTransfer:willCancelTransferFile:withReason::) optional
    func fileTransferWillCancel(_ fileTransfer: CarrierFileTransfer,
                                _ fileId: String,
                                _ status: Int,
                                _ reason: String)
}
