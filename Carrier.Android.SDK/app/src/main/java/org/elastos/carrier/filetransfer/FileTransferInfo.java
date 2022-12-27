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

public class FileTransferInfo {
	private String filename;
	private String fileId;
	private long size;

	public final int MAX_FILE_NAME_LEN = 255;

	/**
	 * Constructor of FileTransferInfo which denotes a file.
	 *
	 * @param
	 *      filename   		[in] The name of the file(no longer than ELA_MAX_FILE_NAME_LEN).
	 * @param
	 *      fileId          [in] Id(obtained by calling FileTransfer.generateFileId()) assigned to the file
	 *                         	 (This field can be left unspecified(i.e. null).In that case,
	 *                         	 the file will be assigned an Id automatically).
	 * @param
	 *      size            [in] The size of the file.
	 *
	 * @throws
	 * 		IllegalArgumentException
	 */
	public FileTransferInfo(String filename, String fileId, long size) {
		if (filename == null || filename.isEmpty() || (fileId != null && fileId.isEmpty()) || size == 0)
			throw new IllegalArgumentException();

		this.filename = filename;
		this.fileId = fileId;
		this.size = size;
	}

	/**
	 * Get the name of the file.
	 *
	 * @return
	 * 		The name of the file.
	 */
	public String getFileName() {
		return filename;
	}

	/**
	 * Get the Id of the file.
	 *
	 * @return
	 * 		The Id of the file.
	 */
	public String getFileId() {
		return fileId;
	}

	/**
	 * Get the size of the file.
	 *
	 * @return
	 * 		The size of the file.
	 */
	public long getSize() {
		return size;
	}
}
