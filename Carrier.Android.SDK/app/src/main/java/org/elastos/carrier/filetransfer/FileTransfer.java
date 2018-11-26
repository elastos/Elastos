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

import org.elastos.carrier.exceptions.CarrierException;
import org.elastos.carrier.Carrier;

import java.util.HashMap;
import java.util.Map;

public class FileTransfer {
	private long nativeCookie; // store native (jni-layered) file transfer handler.
	private long nativeContext;

	private static native String generate_fileId();
	private native void native_close();
	private native String get_fileId(String filename);
	private native String get_filename(String fileId);
	private native boolean native_connect();
	private native boolean accept_connect();
	private native boolean native_add(FileTransferInfo fileinfo);
	private native boolean native_pull(String fileId, long offset);
	private native boolean native_send(String fileId, byte[] data);
	private native boolean native_cancel(String fileId, int status, String reason);
	private native boolean native_pend(String fileId);
	private native boolean native_resume(String fileId);

	private	static native boolean native_easy_send(Carrier carrier, String to, String filename,
												   FileProgressHandler handler);
	private static native boolean native_easy_recv(Carrier carrier, String from, String filename,
												   FileProgressHandler handler);

	private static native int get_error_code();

	private FileTransfer() {}

	/**
	 * Generate unique file identifier with random algorithm.
	 *
	 * Application can call this function to explicitly generate transfer file id,
	 * or let file transfer module generate file id implicitly.
	 *
	 * @return
	 *      The generated unique file identifier.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public static String generateFileId() throws CarrierException {
		String fileId = generate_fileId();
		if (fileId == null)
			throw CarrierException.fromErrorCode(get_error_code());
		return fileId;
	}

	/**
	 * Close file transfer instance.
	 */
	public void close() {
		native_close();
	}

	/**
	 * Get an unique file identifier of specified file.
	 *
	 * Each file has its unique file id used between two peers.
	 *
	 * @param
	 *      filename        [in] The target file name.
	 *
	 * @return
	 *      File id is returned if file transfer instance has file info of filename.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public String getFileId(String filename) throws CarrierException {
		String fileId;

		if (filename == null || filename.isEmpty())
			throw new IllegalArgumentException();

		fileId = get_fileId(filename);
		if (fileId == null)
			throw CarrierException.fromErrorCode(get_error_code());

		return fileId;
	}

	/**
	 * Get file name by file id.
	 *
	 * Each file has its unique file id used between two peers.
	 *
	 * @param
	 *      fileId          [in] The target file identifier.
	 *
	 * @return
	 *      File name is returned if file transfer instance has file id specified.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public String getFileName(String fileId) throws CarrierException {
		String filename;

		if (fileId == null || fileId.isEmpty())
			throw new IllegalArgumentException();

		filename = get_filename(fileId);
		if (filename == null)
			throw CarrierException.fromErrorCode(get_error_code());

		return filename;
	}

	/**
	 * Send a file transfer connect request to target peer.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public void connect() throws CarrierException {
		if (!native_connect())
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * Accept file transfer connection request.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public void acceptConnect() throws CarrierException {
		if (!accept_connect())
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * Add a file to queue of file transfer.
	 *
	 * @param
	 *      fileinfo        [in] Information of the file to be added.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public void addFile(FileTransferInfo fileinfo) throws CarrierException {
		if (fileinfo == null || fileinfo.getFileName().isEmpty() ||
			fileinfo.getSize() == 0)
			throw new IllegalArgumentException();

		if (!native_add(fileinfo))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * To send pull request to transfer file with specified fileId.
	 *
	 * @param
	 *      fileId          [in] The file identifier.
	 * @param
	 *      offset          [in] The offset of file where transfer begins.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public void pullData(String fileId, long offset) throws CarrierException {
		if (fileId == null || fileId.isEmpty())
			throw new IllegalArgumentException();

		if (!native_pull(fileId, offset))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * To transfer file data with specified fileId.
	 *
	 * @param
	 *      fileId          [in] The file identifier.
	 * @param
	 *      data            [in] The data to transfer for file.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public void sendData(String fileId, byte[] data) throws CarrierException {
		if (fileId == null || fileId.isEmpty() || data == null || data.length == 0)
			throw new IllegalArgumentException();

		if (!native_send(fileId, data))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * finish transferring file with specified fileId(only available to sender).
	 *
	 * @param
	 *      fileId          [in] The file identifier.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public void finish(String fileId) throws CarrierException {
		if (fileId == null || fileId.isEmpty())
			throw new IllegalArgumentException();

		if (!native_send(fileId, new byte[0]))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * Cancel transferring file with specified fileId(only available to receiver).
	 *
	 * @param
	 *      fileId          [in] The file identifier.
	 * @param
	 *      status          [in] Cancel transfer status code.
	 * @param
	 *      reason          [in] Cancel transfer reason.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public void cancel(String fileId, int status, String reason) throws CarrierException {
		if (fileId == null || fileId.isEmpty() || reason == null)
			throw new IllegalArgumentException();

		if (!native_cancel(fileId, status, reason))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * Pend transferring file with specified fileId.
	 *
	 * @param
	 *      fileId          [in] The file identifier.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public void pendTransfer(String fileId) throws CarrierException {
		if (fileId == null || fileId.isEmpty())
			throw new IllegalArgumentException();

		if (!native_pend(fileId))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * Resume transferring file with specified fileId.
	 *
	 * @param
	 *      fileId          [in] The file identifier.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public void resumeTransfer(String fileId) throws CarrierException {
		if (fileId == null || fileId.isEmpty())
			throw new IllegalArgumentException();

		if (!native_resume(fileId))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * Send a file to target friend.
	 *
	 * @param
	 *      carrier         [in] Carrier node instance.
	 * @param
	 *      to	  	        [in] The target address.
	 * @param
	 *      filename        [in] The full name of file to transfer.
	 * @param
	 *      handler         [in] A handler that handles all events related to transferring the file.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public static void sendFile(Carrier carrier, String to, String filename,
								FileProgressHandler handler) throws CarrierException {
		if (carrier == null || to == null || to.isEmpty() || filename == null ||
				filename.isEmpty() || handler == null)
			throw new IllegalArgumentException();

		if (!native_easy_send(carrier, to, filename, handler))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * Receive a file from target friend.
	 *
	 * @param
	 *      carrier         [in] Carrier node instance.
	 * @param
	 *      from 	        [in] The target address.
	 * @param
	 *      filename        [in] The full name of file to transfer.
	 * @param
	 *      handler	        [in] A handler that handles all events related to transferring the file.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public static void recvFile(Carrier carrier, String from, String filename,
								FileProgressHandler handler) throws CarrierException {
		if (carrier == null || from == null || from.isEmpty() || filename == null ||
				filename.isEmpty() || handler == null)
			throw new IllegalArgumentException();

		if (!native_easy_recv(carrier, from, filename, handler))
			throw CarrierException.fromErrorCode(get_error_code());
	}
}
