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

package org.elastos.carrier.session;

/**
 * Multiplexing channel close reason mode.
 */
public enum CloseReason {
	/**
	 * Channel closed normaly.
	 */
	Normal,

	/**
	 * Channel closed because of timeout.
	 */
	Timeout,

	/**
	 * Channel closed because error occured.
	 */
	Error;

	/**
	 * Get CloseReason instance from reason value.
	 *
	 * @param
	 *      reason      The value of reason mode.
	 *
	 * @return
	 *      The close reason instance.
	 *
	 * @throws
	 *      IllegalArgumentException
	 */
	public static CloseReason valueOf(int reason) {
		switch (reason) {
			case 0:
				return Normal;
			case 1:
				return Timeout;
			case 2:
				return Error;
			default:
				throw new IllegalArgumentException("Invalid close reason (expected: 0 ~ 2, Gieven:" + reason);
		}
	}

	/**
	 * Get reason value.
	 *
	 * @return
	 *      The reason value.
	 */
	public int value() {
		switch (this) {
			case Normal:
				return 0;
			case Timeout:
				return 1;
			case Error:
			default:
				return 2;
		}
	}

	/**
	 * Get the fully formatized string of close reason instance.
	 *
	 * @param
	 *      reason      The close reason instance.
	 *
	 * @return
	 *      The formatized string of close reason.
	 */
	public static String format(CloseReason reason) {
		return String.format("%s[%d]", reason.name(), reason.value());
	}

	/**
	 * Get fully formatized string.
	 *
	 * @return
	 *      The fully formatized string.
	 */
	@Override
	public String toString() {
		return format(this);
	}
}