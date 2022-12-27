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

package org.elastos.carrier.filetransfer;

public interface FileTransferHandler {
	/**
	 * An application-defined function that handle the state changed event.
	 *
	 * @param
	 *      filetransfer    [in] Carrier file transfer instance.
	 * @param
	 *      state           [in] The file transfer connection state.
	 *
	 */
	void onStateChanged(FileTransfer filetransfer, FileTransferState state);

	/**
	 * An application-defined function that handle transfer file request event.
	 *
	 * @param
	 *      filetransfer    [in] Carrier file transfer instance.
	 * @param
	 *      fileId          [in] The file identifier.
	 * @param
	 *      filename        [in] The file name.
	 * @param
	 *      size            [in] The total file size.
	 */
	void onFileRequest(FileTransfer filetransfer, String fileId, String filename, long size);

	/**
	 * An application-defined function that handle file transfer pull request
	 * event.
	 *
	 * @param
	 *      filetransfer    [in] Carrier file transfer instance.
	 * @param
	 *      fileId          [in] The file identifier.
	 * @param
	 *      offset          [in] The offset of file where transfer begins.
	 */
	 void onPullRequest(FileTransfer filetransfer, String fileId, long offset);

	/**
	 * An application-defined function that perform receiving data.
	 *
	 * @param
	 *      filetransfer    [in] Carrier file transfer instance.
	 * @param
	 *      fileId          [in] The unique identifier of transferring file.
	 * @param
	 *      data            [in] The received data.
	 *
	 * @return
	 * 		Return false if you require no more data, otherwise return true.
	 */
	boolean onData(FileTransfer filetransfer, String fileId, byte[] data);

	/**
 	 * An application-defined function that handles the event of end of receiving data.
 	 *
 	 * @param
 	 *      filetransfer    [in] Carrier file transfer instance.
	 * @param
	 *      fileId          [in] The unique identifier of transferring file.
 	 */
	void onDataFinished(FileTransfer filetransfer, String fileId);

	/**
	 * An application-defined function that handles pause file transfer
	 * notification from the peer.
	 *
	 * @param
	 *      filetransfer    [in] Carrier file transfer instance.
	 * @param
	 *      fileId          [in] The unique identifier of transferring file.
	 */
	void onPending(FileTransfer filetransfer, String fileId);

	/**
	 * An application-defined function that handles resume file transfer
	 * notification from the peer.
	 *
	 * @param
	 *      filetransfer    [in] Carrier file transfer instance.
	 * @param
	 *      fileId          [in] The unique identifier of transferring file.
	 */
	void onResume(FileTransfer filetransfer, String fileId);

	/**
	 * An application-defined function that handles cancel file transfer
	 * notification from the peer.
	 *
	 * @param
	 *      filetransfer    [in] Carrier file transfer instance.
	 * @param
	 *      fileId          [in] The unique identifier of transferring file.
	 * @param
	 *      status          [in] Cancel transfer status code.
	 * @param
	 *      reason          [in] Cancel transfer reason.
	 */
	void onCancel(FileTransfer filetransfer, String fileId, int status, String reason);
}
