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

@inline(__always)
private func getCurrentFileTransfer(_ cctxt: UnsafeMutableRawPointer) -> CarrierFileTransfer {
    return Unmanaged<CarrierFileTransfer>.fromOpaque(cctxt).takeUnretainedValue()
}

func onStateChanged(_ : OpaquePointer?,
                    cStatus: Int32, cctxt: UnsafeMutableRawPointer?)
{
    let filetransfer = getCurrentFileTransfer(cctxt!)

    guard let handler = filetransfer.delegate else {
        return
    }

    let state = CarrierFileTransferConnection(rawValue: Int(cStatus))!

    handler.fileTransferStateDidChange?(state)
}

func onFileReceived(_ : OpaquePointer?,
                    cFileId: UnsafePointer<Int8>?, cFileName: UnsafePointer<Int8>?,
                    fileSize: UInt64,
                    cctxt: UnsafeMutableRawPointer?)
{
    let filetransfer = getCurrentFileTransfer(cctxt!)

    guard let handler = filetransfer.delegate else {
        return
    }

    let fileId   = String(cString: cFileId!)
    let fileName = String(cString: cFileName!)

    handler.didReceiveFileRequest?(fileId, fileName, fileSize)
}

func onPullReceived(_ :OpaquePointer?,
                    cFileId: UnsafePointer<Int8>?,  offset: UInt64,
                    cctxt: UnsafeMutableRawPointer?)
{
    let filetransfer = getCurrentFileTransfer(cctxt!)

    guard let handler = filetransfer.delegate else {
        return
    }

    let fileId = String(cString: cFileId!)

    handler.didReceivePullRequest?(fileId, offset)
}

func onDataReceived(_ : OpaquePointer?,
                    cFileId: UnsafePointer<Int8>?,
                    cData: UnsafePointer<Int8>, cLength: UInt32,
                    cctxt: UnsafeMutableRawPointer?) -> Bool
{
    let filetransfer = getCurrentFileTransfer(cctxt!)

    guard let handler = filetransfer.delegate else {
        return false
    }

    var result: Bool = true

    autoreleasepool {
        let fileId = String(cString: cFileId!)
        let data = Data(bytes: cData, count: Int(cLength))

        if (handler.didReceiveData != nil) {
            result = handler.didReceiveData!(fileId, data)
        }
    }

    return result
}

func onPendReceived(_ : OpaquePointer?,
                    cFileId: UnsafePointer<Int8>?,
                    cctxt: UnsafeMutableRawPointer?)
{
    let filetransfer = getCurrentFileTransfer(cctxt!)

    guard let handler = filetransfer.delegate else {
        return
    }

    let fileId = String(cString: cFileId!)

    handler.fileTransferPending?(fileId)
}

func onResumeReceived(_ : OpaquePointer?,
                    cFileId: UnsafePointer<Int8>?,
                    cctxt: UnsafeMutableRawPointer?)
{
    let filetransfer = getCurrentFileTransfer(cctxt!)

    guard let handler = filetransfer.delegate else {
        return
    }

    let fileId = String(cString: cFileId!)

    handler.fileTransferResumed?(fileId)
}

func onCancelReceived(_ : OpaquePointer?,
                    cFileId: UnsafePointer<Int8>?,
                    cStatus: Int32, cReason: UnsafePointer<Int8>?,
                    cctxt: UnsafeMutableRawPointer?)
{
    let filetransfer = getCurrentFileTransfer(cctxt!)

    guard let handler = filetransfer.delegate else {
        return
    }

    let fileId = String(cString: cFileId!)
    let reason = String(cString: cReason!)

    handler.fileTransferWillCancel?(fileId, Int(cStatus), reason)
}

@inline(__always) private func TAG() -> String { return "FileTransferManager" }

public typealias CarrierFileTransferConnectHandler = (_ carrier: Carrier,
        _ from: String, _ fileinfo: CarrierFileTransferInfo) -> Void

/// The class representing carrier session manager.
@objc(ELAFileTransferManager)
public class CarrierFileTransferManager: NSObject {

    private static var filetransferManager: CarrierFileTransferManager?

    private var carrier: Carrier?
    private var handler: CarrierFileTransferConnectHandler?
    private var didCleanup: Bool

    /// Get a carrier filetransfer manager instance.
    ///
    /// This function is convinience way to get instance without interest to
    /// connect request from friends.
    ///
    /// - Parameters:
    ///   - carrier: Carrier node instance
    ///
    /// - Throws:
    ///     CarrierError
    ///
    @objc(getInstance:error:)
    public static func InitializeInstance(carrier: Carrier) throws {
        if (filetransferManager != nil && filetransferManager!.carrier != carrier) {
            filetransferManager?.cleanup()
        }

        if (filetransferManager == nil) {
            Log.d(TAG(), "Begin to initialize filetransfer manager ...")

            let result = ela_filetransfer_init(carrier.ccarrier, nil, nil)
            guard result >= 0 else {
                let errno = getErrorCode()
                Log.e(TAG(), "Initialize filetransfer manager error:0x%X", errno)
                throw CarrierError.FromErrorCode(errno: errno)
            }

            Log.d(TAG(), "The native filetransfer manager initialized.")

            filetransferManager = CarrierFileTransferManager(carrier)
            filetransferManager!.didCleanup = false

            Log.i(TAG(), "Native session manager instance created.");
        }
    }

    /// Get a carrier filetransfer manager instance.
    ///
    /// - Parameters:
    ///   - carrier: Carrier node instance
    ///
    /// - Throws:
    ///     CarrierError
    @objc(getInstance:usingHandler:error:)
    public static func InitializeInstance(carrier: Carrier,
                            handler: @escaping CarrierFileTransferConnectHandler) throws {
        if (filetransferManager != nil && filetransferManager!.carrier != carrier) {
            filetransferManager?.cleanup()
        }

        if (filetransferManager == nil) {
            Log.d(TAG(), "Begin to initialize native carrier filetransfer manager ...")

            let cb: CFileTransferConnectCallback? = {(_, cfrom, cfileInfo, cctxt) in
                let manager = Unmanaged<CarrierFileTransferManager>
                        .fromOpaque(cctxt!).takeUnretainedValue()

                let carrier = manager.carrier
                let handler = manager.handler

                let from = String(cString: cfrom!)
                let fileinfo = CarrierFileTransferInfo()  //TODO

                handler!(carrier!, from, fileinfo)
            }

            let filetransferManager = CarrierFileTransferManager(carrier)
            filetransferManager.handler = handler
            let cctxt = Unmanaged.passUnretained(filetransferManager).toOpaque()

            let result = ela_filetransfer_init(carrier.ccarrier, cb, cctxt)
            guard result >= 0 else {
                let errno = getErrorCode()
                Log.e(TAG(), "Initialize filetransfer manager error:0x%X", errno)
                throw CarrierError.FromErrorCode(errno: errno)
            }

            Log.d(TAG(), "The native filetransfer manager initialized.")
        }
    }

    /// Get a carrier filetransfer manager instance.
    ///
    /// - Returns: The carrier filetransfer manager or nil
    public static func getInstance() -> CarrierFileTransferManager? {
        return filetransferManager
    }

    private init(_ carrier: Carrier) {
        self.carrier = carrier
        self.didCleanup = false
    }

    deinit {
        cleanup()
    }

    ///  Clean up carrier session manager.
    public func cleanup() {

        objc_sync_enter(self)
        if !didCleanup {
            Log.d(TAG(), "Begin clean up native carrier session manager ...")

            ela_session_cleanup(carrier!.ccarrier)
            carrier = nil
            didCleanup = true
            CarrierFileTransferManager.filetransferManager = nil

            Log.i(TAG(), "Native carrier session managed cleanuped.")
        }
        objc_sync_exit(self)
    }

    /// Create a new filetransfer connection to the specified friend.
    ///
    /// The filetransfer object represents a connection handle to a friend.
    ///
    /// - Parameters:
    ///   - address:    The target address.
    ///   - fileinfo:   The fileinfo to plan to transfer.
    ///   - delegate:   The delegate attached to the new filetransfer instance.
    ///
    /// - Returns: The new CarrierFileTransfer
    ///
    /// - Throws: CarrierError
    private func newFileTransfer(to address: String,
                                withFileInfo fileInfo: CarrierFileTransferInfo?,
                                delegate: CarrierFileTransferDelegate)
        throws -> CarrierFileTransfer {

        var cFileInfo: CFileTransferInfo?
        if (fileInfo != nil) {
            cFileInfo = convertCarrierFileTransferInfoToCFileTransferInfo(fileInfo!)
        }

        var callbacks = CFileTransferCallbacks()
        callbacks.state_changed = onStateChanged
        callbacks.file          = onFileReceived
        callbacks.pull          = onPullReceived
        callbacks.data          = onDataReceived
        callbacks.pending       = onPendReceived
        callbacks.resume        = onResumeReceived
        callbacks.cancel        = onCancelReceived

        let fileTransfer = CarrierFileTransfer(address)
        fileTransfer.delegate = delegate

        Log.d(TAG(), "Begin to new native filetransfer instance.")

        let cctxt = Unmanaged.passUnretained(fileTransfer).toOpaque()

        let cFileTransfer = address.withCString() { (ptr) -> OpaquePointer? in
            if cFileInfo != nil {
                return ela_filetransfer_new(carrier?.ccarrier, ptr, &cFileInfo!, callbacks, cctxt)
            } else {
                return ela_filetransfer_new(carrier?.ccarrier, ptr, nil, callbacks, cctxt)
            }
        }

        guard cFileTransfer != nil else {
            let errno = getErrorCode()
            Log.e(TAG(), "Creating a new filetransfer error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "An new filetransfer instance created.\n")

        fileTransfer.cfiletransfer = cFileTransfer
        return fileTransfer
    }
}
