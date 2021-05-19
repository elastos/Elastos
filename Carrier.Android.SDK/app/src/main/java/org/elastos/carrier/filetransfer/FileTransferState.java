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

public enum FileTransferState {
	/** The file transfer connection is initialized. */
	Initialized,

	/** The file transfer connection is connecting.*/
	Connecting,

	/** The file transfer connection has been established. */
	Connected,

	/** The file transfer connection is closed and disconnected. */
	Closed,

	/** The file transfer connection failed with some reason. */
	Failed;

	/**
	 * Get carrier file transfer state from state value.
	 *
	 * @param
	 *      state       The state value.
	 *
	 * @return
	 *      The carrier file transfer state.
	 */
	public static FileTransferState valueOf(int state) {
		switch (state) {
			case 1:
				return Initialized;
			case 2:
				return Connecting;
			case 3:
				return Connected;
			case 4:
				return Closed;
			case 5:
				return Failed;
			default:
				throw new IllegalArgumentException("Invalid Filetransfer State (expected: 1 ~ 5, Gieven:" + state);
		}
	}
}
