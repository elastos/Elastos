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

public interface FileProgressHandler {
	/**
	 * An application-defined function that handles file transfer connection
	 * state changed event.
	 *
	 * @param
	 *      state           [in] The file transfer connection state.
	 */
    void onStateChanged(FileTransferState state);

	/**
	 * An application-defined function that handles file sent event.
	 *
	 * @param
	 *      length          [in] The amount of data sent.
	 * @param
	 *      totalsz         [in] The total amount of transferring file.
	 */
    void onDataSent(long length, long totalsz);

	/**
	 * An application-defined function that handles file received event.
	 *
	 * @param
	 *      length          [in] The amount of data sent.
	 * @param
	 *      totalsz         [in] The total amount of transferring file.
	 */
    void onDataReceived(long length, long totalsz);
}
