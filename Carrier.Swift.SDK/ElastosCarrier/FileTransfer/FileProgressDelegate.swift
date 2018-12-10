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
    The protocol to file transfer progress of upper layer
 */
@objc(ELACarrierFileProgressDelegate)
public protocol CarrierFileProgressDelegate {

    /// Tell the delegate that the state of carrier filetransfer has been changed.
    ///
    /// - Parameters:
    ///     - newState: Stream state defined in `CarrierStreamState`
    @objc(CarrierFileTransferStateDidChange:) optional
    func fileTransferStateDidChange(_ newState: CarrierFileTransferConnection)

    /// Tell the delegate that a block data of file has been sent.
    ///
    /// - Parameters:
    ///     - length:     The length of file data that has been sent
    ///     - totalSize:  The total size of file being transfered.
    @objc(didSendDataWithLength:totalSize:) optional
    func didSendData(withLength length: UInt32, totalSize: UInt64)

    /// Tell the delegate that a block data of file has been received.
    ///
    /// - Parameters:
    ///     - length:     The length of file data that has been sent
    ///     - totalSize:  The total size of file being transfered.
    @objc(didReceiveDataWithLength:totalSize:)optional
    func didReceiveData(withLength length: UInt32, totalSize: UInt64)
}
