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

import java.io.IOException;
import java.io.OutputStream;
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
	private native int native_send(String fileId, byte[] data, int offset, int len);
	private native boolean native_cancel(String fileId, int status, String reason);
	private native boolean native_pend(String fileId);
	private native boolean native_resume(String fileId);

	private static native int get_error_code();

	private FileTransfer() {}

	private class FileTransferOutputStream extends OutputStream {
		private String fileId;

		FileTransferOutputStream(String fileId) {
			this.fileId = fileId;
		}

		@Override
		public void write(int i) throws IOException {
			try {
				FileTransfer.this.writeData(fileId, (byte)i);
			} catch (CarrierException e) {
				throw new IOException(e);
			}
		}

		@Override
		public void write(byte[] b) throws IOException {
			try {
				FileTransfer.this.writeData(fileId, b);
			} catch (CarrierException e) {
				throw new IOException(e);
			}
		}

		@Override
		public void write(byte[] b, int offset, int len) throws IOException {
			try {
				FileTransfer.this.writeData(fileId, b, offset, len);
			} catch (CarrierException e) {
				throw new IOException(e);
			}
		}

		@Override
		public void flush() {
		}

		@Override
		public void close() {
		}
	}

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
	 * @param
	 *      offset          [in] The start offset.
	 * @param
	 *      len             [in] The bytes to write.
	 *
	 * @return
	 * 		Bytes of data sent on success.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public int writeData(String fileId, byte[] data, int offset, int len) throws CarrierException {
		if (fileId == null || fileId.isEmpty() || data == null || data.length == 0 ||
			offset < 0 || len <= 0 || offset + len < len || offset + len > data.length)
			throw new IllegalArgumentException();

		int bytes = native_send(fileId, data, offset, len);
		if (bytes < 0)
			throw CarrierException.fromErrorCode(get_error_code());
		return bytes;
	}

	/**
	 * To transfer file data with specified fileId.
	 *
	 * @param
	 *      fileId          [in] The file identifier.
	 * @param
	 *      data            [in] The data to transfer for file.
	 *
	 * @return
	 * 		Bytes of data sent on success.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public int writeData(String fileId, byte[] data) throws CarrierException {
		return writeData(fileId, data, 0, data.length);
	}

	/**
	 * To transfer file data with specified fileId.
	 *
	 * @param
	 *      fileId          [in] The file identifier.
	 * @param
	 *      data            [in] The data to transfer for file.
	 *
	 * @return
	 * 		Bytes of data sent on success.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public int writeData(String fileId, byte data) throws CarrierException {
		byte[] _data = new byte[1];
		_data[0] = data;

		return writeData(fileId, _data);
	}

	public OutputStream getOutputStream(String fileId) {
		return new FileTransferOutputStream(fileId);
	}

	/**
	 * Finish transferring file with specified fileId(only available to sender).
	 *
	 * @param
	 *      fileId          [in] The file identifier.
	 *
	 * @throws
	 * 		CarrierException
	 */
	public void sendFinish(String fileId) throws CarrierException {
		if (fileId == null || fileId.isEmpty())
			throw new IllegalArgumentException();

		int bytes = native_send(fileId, new byte[0], 0, 0);
		if (bytes < 0)
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
	public void cancelTransfer(String fileId, int status, String reason) throws CarrierException {
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
}
