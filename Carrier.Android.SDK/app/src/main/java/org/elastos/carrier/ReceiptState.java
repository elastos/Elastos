/*
 * Copyright (c) 2020 Elastos Foundation
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

package org.elastos.carrier;

/**
 * Carrier message receipt status
 */
public enum ReceiptState {

	/**
	 * Message has been accepted by remote friend via carrier network.
	 */
	ReceiptByFriend,
	/**
	 * \~English
	 * Message has been delivered to offline message store.
	 */
	DeliveredAsOffline,
	/**
	 * \~English
	 * Message sent before not
	 * Message send unsuccessfully. A specific error code can be
	 * retrieved by calling ela_get_error().
	 */
	Error;

	/**
	 * Get ReceiptState object from state value.
	 *
	 * @param
	 *      state    The receipt state value.
	 *
	 * @return
	 *      The ReceiptState object.
	 *
	 * @throws IllegalArgumentException illegal exception.
	 */
	public static ReceiptState valueOf(int state)  {
		switch (state) {
			case 0:
				return ReceiptByFriend;
			case 1:
				return DeliveredAsOffline;
			case 2:
				return Error;
			default:
				throw new IllegalArgumentException("Invalid receipt state (expected: 0 ~ 2, Gieven:" + state);
		}
	}

	/**
	 * Get receipt state value.
	 *
	 * @return
	 *      The receipt state value.
	 */
	public int value() {
		switch (this) {
			case ReceiptByFriend:
				return 0;
			case DeliveredAsOffline:
				return 1;
			case Error:
			default:
				return 2;
		}
	}

	/**
	 * Get debug description of ReceiptState object
	 *
	 * @param
	 *      state        The receipt state.
	 *
	 * @return
	 *      The debug description of receipt state.
	 */
	public static String format(ReceiptState state) {
		return String.format("%s[%d]", state.name(), state.value());
	}

	/**
	 * Get debug description of ReceiptState object
	 *
	 * @return
	 *      The debug description of receipt state.
	 */
	@Override
	public String toString() {
		return format(this);
	}
}
