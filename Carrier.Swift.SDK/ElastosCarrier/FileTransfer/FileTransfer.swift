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

@inline(__always) private func TAG() -> String { return "CarrierFileTransfer" }

/// The class representing the carrier filetransfer connection.
@objc(ELACarrierFileTransfer)
public class CarrierFileTransfer: NSObject {

    public static let MAX_FILE_ID_LEN   = 45
    public static let MAX_FILE_NAME_LEN = 255

    private  var address: String
    private  var didClose: Bool

    internal var cfiletransfer: OpaquePointer?
    internal weak var delegate: CarrierFileTransferDelegate?

    init(_ address: String) {
        self.address = address
        self.didClose = false
        super.init()
    }

    deinit {
        close()
    }

    /// Close a filetransfer to friend. All resources associated with current
    /// filetransfer will be destroyed.
    public func close() {
        objc_sync_enter(self)
        if !didClose {
            Log.d(TAG(), "Begin to close filetransfer instance ...")

            ela_filetransfer_close(cfiletransfer)
            didClose = true

            Log.d(TAG(), "Native filetransfer instance closed nicely")
        }
        objc_sync_exit(self)
    }

    /// Get remote friend id.
    ///
    /// - Returns: The remote peer userid or userid@nodeid
    public func getPeer() -> String {
        return address;
    }

    /// Acquire an unique file identifier by filename within the filetransfer
    /// instance.
    ///
    /// - Parameters:
    ///     - fileName:    The file name
    @objc(acquireFileIdByFileName:error:)
    public func acquireFileId(by fileName: String) throws -> String {
        let len  = CarrierFileTransfer.MAX_FILE_ID_LEN + 1
        var data = Data(count: len)

        let cfileId = fileName.withCString() {
            (cfileName) -> UnsafePointer<Int8>? in
            return data.withUnsafeMutableBytes() {
                (ptr: UnsafeMutablePointer<Int8>) -> UnsafePointer<Int8>? in
                return ela_filetransfer_get_fileid(cfiletransfer, cfileName, ptr, len)
            }
        }

        guard cfileId != nil else {
            let errno = getErrorCode()
            Log.e(TAG(), "Acquire fileId of \(fileName) error:0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        return String(cString: cfileId!)
    }

    /// Acquire an unique file filename by file identifier within the filetransfer
    /// instance.
    ///
    /// - Parameters:
    ///     - fileId:    The file identifier
    @objc(acquireFileNameByFileId:error:)
    public func acquireFileName(by fileId: String) throws -> String {
        let len  = CarrierFileTransfer.MAX_FILE_NAME_LEN + 1
        var data = Data(count: len)

        let cfileName = fileId.withCString() {
            (cfileId) -> UnsafePointer<Int8>? in
            return data.withUnsafeMutableBytes() {
                (ptr: UnsafeMutablePointer<Int8>) -> UnsafePointer<Int8>? in
                return ela_filetransfer_get_fileid(cfiletransfer, cfileId, ptr, len)
            }
        }

        guard cfileName != nil else {
            let errno = getErrorCode()
            Log.e(TAG(), "Acquire filename with fileId \(fileId) error:0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        return String(cString: cfileName!)
    }

    /// Send filetransfer connection request to remote friend.
    ///
    /// - Throws: CarrierError
    ///
    @objc(sendConnectionRequest:)
    public func sendConnectionRequest() throws {
        let result = ela_filetransfer_connect(cfiletransfer)

        guard result >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Filetransfer send connection request error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Filetransfer connection request to \(address) sended.")
    }

    /// Accept filetransfer connection  request.
    ///
    /// - Throws: CarrierError
    ///
    @objc(acceptConnectionRequest:)
    public func acceptConnectionRequest() throws {
        let result = ela_filetransfer_accept_connect(cfiletransfer)

        guard result >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Filetransfer accept connection request error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Filetransfer connection request from \(address) accepted.")
    }

    /// Add a file to the queue of file transfer.
    ///
    /// - Parameters:
    ///     - fileinfo:     The fileinfo to transfer
    ///
    /// - Throws: CarrierError
    ///
    @objc(addFile:error:)
    public func addFile(_ fileInfo: CarrierFileTransferInfo) throws {
        var cfileInfo = convertCarrierFileTransferInfoToCFileTransferInfo(fileInfo)

        let result = ela_filetransfer_add(cfiletransfer, &cfileInfo)
        guard result >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Add a file to filetranasfer error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Added \(fileInfo) to file transfer.")
    }

    /// Send pull request to start transfering.
    ///
    /// - Parameters:
    ///     - fileId:   The identifier of file to start transfering.
    ///     - offset:   The file offset where transfering begins
    ///
    /// - Throws: CarrierError
    @objc(sendPullRequest:withOffset:error:)
    public func sendPullRequest(fileId: String, withOffset offset: UInt64) throws {
        let result = fileId.withCString() { (cfileId) -> Int32 in
            return ela_filetransfer_pull(cfiletransfer, cfileId, offset)
        }

        guard result >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Send a pull request to filetranasfer error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Send a pull request to transfer \(fileId).")
    }

    /// Send file data to remote peer.
    ///
    /// - Parameters:
    ///     - fileId:   The identifer of file from which data has been read.
    ///     - data:     The file data.
    ///
    /// - Throws: CarrierError
    @objc(sendData:withValue:error:)
    public func sendData(fileId: String, withData data: Data) throws {
        let result = fileId.withCString() { (cfileId) -> Int32 in
            return data.withUnsafeBytes() { (cdata) -> Int32 in
                return ela_filetransfer_send(cfiletransfer, cfileId, cdata, UInt32(data.count))
            }
        }

        guard result >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Send data to filetranasfer error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Send file \(fileId) data in success.")
    }

    /// Cancel transfering file
    ///
    /// - Parameters:
    ///     - fileId: The identifier of file to cancel transfering.
    ///     - status: The status represnets cancel reason.
    ///     - reason: The cancel reason.
    ///
    /// - Throws: CarrierError
    @objc(cancelTransfering:withReason::error:)
    public func cancelTransfering(fileId: String, withReason status: Int,
                                  reason: String) throws {
        let result = fileId.withCString() { (cfileId) -> Int32 in
            return reason.withCString() { (creason) -> Int32 in
                return ela_filetransfer_cancel(cfiletransfer, cfileId,
                                               Int32(status), creason)
            }
        }

        guard result >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Cancel filetransfer for \(fileId) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Send a request to cancel transfering file \(fileId) with",
                     " status \(status) and reason \(reason).")
    }

    /// Pend to transfer file.
    ///
    /// - Parameters:
    ///     - fileId: The identifier of file to pend transfering.
    ///
    /// - Throws: CarrierError
    @objc(pendTransfering:error:)
    public func pendTransfering(fileId: String) throws {
        let result = fileId.withCString() { (cfileId) -> Int32 in
            return ela_filetransfer_pend(cfiletransfer, cfileId)
        }

        guard result >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Pend transfering of file \(fileId) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Send request to pend transfer file \(fileId)")
    }

    /// Resume to transfer file.
    ///
    /// - Parameters:
    ///     - fileId: The identifier of file to resume transfer.
    ///
    /// - Throws: CarrierError
    @objc(resumeTransfering:error:)
    public func resumeTransfering(fileId: String) throws {
        let result = fileId.withCString() { (cfileId) -> Int32 in
            return ela_filetransfer_resume(cfiletransfer, cfileId)
        }

        guard result >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Resume transfering of file \(fileId) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Send request to Resume transfer file \(fileId)")
    }
}

