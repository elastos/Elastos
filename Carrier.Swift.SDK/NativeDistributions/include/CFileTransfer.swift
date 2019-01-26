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
 * \~English
 * A structure representing the file transfer meta information.
 *
 * Two peer carrier nodes use this structure to declare which file to transfer.
 */
internal struct CFileTransferInfo {
    /**
     * \~English
     * File name of file to transfer, without file path.
     */
    var filename: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    /**
     * \~English
     * Unique fileid of file to transfer, which is being unique in a file
     * transfer instance.
     */
    var fileid: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    /**
     * \~English
     * Total size of file to transfer.
     */
    var size: UInt64 = 0

    init() {}
}

internal struct CFileTransferConnection : RawRepresentable, Equatable {

    init(_ rawValue: UInt32) {
        self.rawValue = rawValue
    }

    init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    var rawValue: UInt32
}

/** The file transfer connection is initialized. */
internal var CFileTransferConnection_initialized: CFileTransferConnection { get { return CFileTransferConnection(1) } }
/** The file transfer connection is being established.*/
internal var CFileTransferConnection_connecting: CFileTransferConnection { get { return CFileTransferConnection(2) } }
/** The file transfer connection has been connected. */
internal var CFileTransferConnection_connected: CFileTransferConnection { get { return CFileTransferConnection(3) } }
/** The file transfer connection failed with some reason. */
internal var CFileTransferConnection_failed: CFileTransferConnection { get { return CFileTransferConnection(4) } }
/** The file transfer connection is closed and disconnected. */
internal var CFileTransferConnection_closed: CFileTransferConnection { get { return CFileTransferConnection(5) } }

/**
 * \~English
 * Carrier file transfer callbacks.
 */
internal struct CFileTransferCallbacks {
    /**
     * \~English
     * An application-defined function that handle the state changed event.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      state           [in] The file transfer connection state.
     * @param
     *      context         [in] The application defined context data.
     *
     */
    var state_changed: (@convention(c) (OpaquePointer?, Int32, UnsafeMutableRawPointer?) -> Swift.Void)!

    /**
     * \~English
     * An application-defined function that handle transfer file request event.
     *
     * @param
     *      filetransfer    [in] A handle to the Carrier file transfer
     *                           instance.
     * @param
     *      fileid          [in] The file identifier.
     * @param
     *      filename        [in] The file name.
     * @param
     *      size            [in] The total file size.
     * @param
     *      context         [in] The application defined context data.
     */
    var file: (@convention(c) (OpaquePointer?, UnsafePointer<Int8>?, UnsafePointer<Int8>?,
                               UInt64, UnsafeMutableRawPointer?) -> Swift.Void)!

    /**
     * \~English
     * An application-defined function that handle file transfer pull request
     * event.
     *
     * @param
     *      filetransfer    [in] A handle to the Carrier file transfer
     *                           instance.
     * @param
     *      fileid          [in] The file identifier.
     * @param
     *      offset          [in] The offset of file where transfer begins.
     * @param
     *      context         [in] The application defined context data.
     */
     var pull: (@convention(c) (OpaquePointer?, UnsafePointer<Int8>?, UInt64,
                                UnsafeMutableRawPointer?) -> Swift.Void)!

    /**
     * \~English
     * An application-defined function that perform receiving data.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique identifier of transferring file.
     * @param
     *      data            [in] The pointer to received data.
     * @param
     *      length          [in] The length of received data.
     * @param
     *      context         [in] The application defined context data.
     *
     * @return
     *      Return True if file transfer has completed, otherwise return False.
     */
    var data: (@convention(c) (OpaquePointer?, UnsafePointer<Int8>?, UnsafePointer<Int8>,
                               UInt32, UnsafeMutableRawPointer?) -> Swift.Bool)!

    /**
     * \~English
     * An application-defined function that handles pause file transfer
     * notification from the peer.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique identifier of transferring file.
     * @param
     *      context         [in] The application defined context data.
     */
    var pending: (@convention(c) (OpaquePointer?, UnsafePointer<Int8>?,
                                  UnsafeMutableRawPointer?) -> Swift.Void)!

    /**
     * \~English
     * An application-defined function that handles resume file transfer
     * notification from the peer.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique identifier of transferring file.
     * @param
     *      context         [in] The application defined context data.
     */
    var resume: (@convention(c) (OpaquePointer?, UnsafePointer<Int8>?,
                                 UnsafeMutableRawPointer?) -> Swift.Void)!

    /**
     * \~English
     * An application-defined function that handles cancel file transfer
     * notification from the peer.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique identifier of transferring file.
     * @param
     *      status          [in] Cancel transfer status code.
     * @param
     *      reason          [in] The string presents cancel reason.
     * @param
     *      context         [in] The application defined context data.
     */
    var cancel: (@convention(c) (OpaquePointer?, UnsafePointer<Int8>?, Int32,
                UnsafePointer<Int8>?, UnsafeMutableRawPointer?) -> Swift.Void)!

    init() {}
}

/**
 * \~English
 * Generate unique file identifier with random algorithm.
 *
 * Application can call this function to explicitly generate transfer fileid,
 * or let file transfer module generate fileid implicitly.
 *
 * @param
 *      fileid          [in] The buffer to receive generated file identifier.
 * @param
 *      length          [in] The buffer length.
 *
 * @return
 *      The generated unique file identifier.
 */
@_silgen_name("ela_filetransfer_fileid")
internal func ela_filetransfer_fileid(_ fileid: UnsafeMutablePointer<Int8>?,
                                      _ length: Int) -> UnsafePointer<Int8>?

/**
 * \~English
 * An application-defined function that handles file transfer connect
 * request.
 *
 * @param
 *      carrier         [in] A handle to the Carrier node instance.
 * @param
 *      address         [in] The requesting address.
 * @param
 *      fileinfo        [in] Information of the file to transfer.
 * @param
 *      context         [in] The application defined context data.
 */
internal typealias CFileTransferConnectCallback = @convention(c)
    (OpaquePointer?, UnsafePointer<Int8>?, UnsafeRawPointer?, UnsafeMutableRawPointer?) -> Swift.Void

/**
 * \~English
 * Initialize file transfer extension.
 *
 * The application must initialize the file transfer extension before
 * calling any file transfer API.
 *
 * @param
 *      carrier         [in] A handle to the Carrier node instance.
 * @param
 *      callback        [in] A pointer to file transfer connect callback.
 * @param
 *      context         [in] The application defined context data.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_init")
internal func ela_filetransfer_init(_ carrier: OpaquePointer!,
                                    _ callback: CFileTransferConnectCallback?,
                                    _ context: UnsafeMutableRawPointer?) -> Int32

/**
 * \~English
 * Clean up Carrier file transfer extension.
 *
 * The application should call ela_file_transfer_cleanup before quit,
 * to clean up the resources associated with the extension.
 *
 * If the extension is not initialized, this function has no effect.
 */
@_silgen_name("ela_session_cleanup")
internal func ela_filetransfer_cleanup(_ carrier: OpaquePointer!)

/**
 * \~English
 * Open a file transfer instance.
 *
 * The application must open file transfer instance before sending
 * request/reply to transfer file.
 *
 * As to send request to transfer file, application may or may not feed
 * information of the file that we want to transfer. And for receiving side,
 * application MUST feed file information received from connect request
 * callback.
 *
 * @param
 *      carrier         [in] A handle to the Carrier node instance.
 * @param
 *      address         [in] The target address.
 * @param
 *      fileinfo        [in] The information of file to transfer, could be NULL.
 * @param
 *      callbacks       [in] A pointer to ElaFileTransferCallbacks to handle
 *                           all events related to new file transfer instance.
 * @param
 *      context         [in] The application defined context data.
 *
 * @return
 *      Return an ElaFileTransfer instance on success, NULL otherwise(The
 *      specific error code can be retrieved by calling ela_get_error()).
 */
@_silgen_name("ela_filetransfer_new")
internal func ela_filetransfer_new(_ carrier: OpaquePointer!,
                                   _ address: UnsafePointer<Int8>!,
                                   _ fileinfo: UnsafePointer<CFileTransferInfo>?,
                                   _ callbacks: CFileTransferCallbacks!,
                                   _ context: UnsafeMutableRawPointer?) -> OpaquePointer?
/**
 * \~English
 * Close file transfer instance.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 */
@_silgen_name("ela_filetransfer_close")
internal func ela_filetransfer_close(_ filetransfer: OpaquePointer!)

/**
 * \~English
 * Get an unique file identifier of specified file.
 *
 * Each file has its unique fileid used between two peers.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      filename        [in] The target file name.
 * @param
 *      fileid          [in] A buffer to receive file identifier.
 * @param
 *      length          [in] The length of buffer to receive file identifier.
 *
 * @return
 *      Fileid is returned if filetransfer instance has file info of filename,
 *      otherwise, NULL value is returned. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_session_cleanup")
internal func ela_filetransfer_get_fileid(_ filetransfer: OpaquePointer!,
                                          _ filename: UnsafePointer<Int8>!,
                                          _ fileid: UnsafeMutablePointer<Int8>!,
                                          _ length: Int) -> UnsafePointer<Int8>?

/**
 * \~English
 * Get file name by fileid.
 *
 * Each file has its unique fileid used between two peers.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The target file identifier.
 * @param
 *      filename        [in] The buffer to receive file name.
 * @param
 *      length          [in] The length of buffer to receive file name.
 *
 * @return
 *      File name is returned if filetransfer instance has fileid specified,
 *      otherwise, NULL value is returned. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_get_filename")
internal func ela_filetransfer_get_filename(_ filetransfer: OpaquePointer!,
                                            _ fileid: UnsafePointer<Int8>!,
                                            _ filename: UnsafeMutablePointer<Int8>!,
                                            _ length: Int) -> UnsafePointer<Int8>?
/**
 * \~English
 * Send a file transfer connect request to target peer.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_connect")
internal func ela_filetransfer_connect(_ filetransfer: OpaquePointer!) -> Int32

/**
 * \~English
 * Accept file transfer connection request.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_accept_connect")
internal func ela_filetransfer_accept_connect(_ filetransfer: OpaquePointer!) -> Int32

/**
 * \~English
 * Add a file to queue of file transfer.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileinfo        [in] Information of the file to be added.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_add")
internal func ela_filetransfer_add(_ filetransfer: OpaquePointer!,
                                   _ fileinfo: UnsafePointer<CFileTransferInfo>!) -> Int32

/**
 * \~English
 * To send pull request to transfer file with specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 * @param
 *      offset          [in] The offset of file where transfer begins.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_pull")
internal func ela_filetransfer_pull(_ filetransfer: OpaquePointer!,
                                    _ fileid: UnsafePointer<Int8>!,
                                    _ offset: UInt64) -> Int32

/**
 * \~English
 * To transfer file data with specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 * @param
 *      data            [in] The data to transfer for file.
 * @param
 *      length          [in] The length of data to transfer for file.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_send")
internal func ela_filetransfer_send(_ filetransfer: OpaquePointer!,
                                    _ fileid: UnsafePointer<Int8>!,
                                    _ cdata: UnsafePointer<Int8>!,
                                    _ length: UInt32) -> Int32

/**
 * \~English
 * Cancel transferring file with specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_cancel")
internal func ela_filetransfer_cancel(_ filetransfer: OpaquePointer!,
                                      _ fileid: UnsafePointer<Int8>!,
                                      _ status: Int32,
                                      _ reason: UnsafePointer<Int8>!) -> Int32

/**
 * \~English
 * Pause transferring file with specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_pend")
internal func ela_filetransfer_pend(_ filetransfer: OpaquePointer!,
                                    _ fileid: UnsafePointer<Int8>!) -> Int32

/**
 * \~English
 * Resume transferring file with specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_resume")
internal func ela_filetransfer_resume(_ filetransfer: OpaquePointer!,
                                      _ fileid: UnsafePointer<Int8>!) -> Int32

/**
 * \~English
 * Bind userdata to specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 * @param
 *      userdata        [in] The pointer to userdata.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_set_userdata")
internal func ela_filetransfer_set_userdata(_ filetransfer: OpaquePointer!,
                                            _ fileid: UnsafePointer<Int8>!,
                                            _ userdata: UnsafeMutableRawPointer?) -> Int32

/**
 * \~English
 * Get userdata bound to specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 *
 * @return
 *      userdata on success, or NULL if an error occurred. The specific error
 *      code can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_filetransfer_set_userdata")
internal func ela_filetransfer_set_userdata(_ filetransfer: OpaquePointer!,
                                            _ fileid: UnsafePointer<Int8>!) -> UnsafeMutableRawPointer?
