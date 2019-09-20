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

package org.elastos.carrier;

/**
 * Carrier node presence status.
 */
public enum PresenceStatus {

	/**
	 * Carrier node is online and available.
	 */
	None,

	/**
	 * Carrier node is being away.
	 */
	Away,

	/**
	 * Carrier node is being busy.
	 */
	Busy;

	/**
	 * Get PresenceStatus object from status value.
	 *
	 * @param
	 *      status    The presence status value.
	 *
	 * @return
	 *      The Carrier node status object.
	 *
	 * @throws IllegalArgumentException illegal exception.
	 */
	public static PresenceStatus valueOf(int status)  {
		switch (status) {
			case 0:
				return None;
			case 1:
				return Away;
			case 2:
				return Busy;
			default:
				throw new IllegalArgumentException("Invalid presence status (expected: 0 ~ 2, Gieven:" + status);
		}
	}

	/**
	 * Get carrier node presence status value.
	 *
	 * @return
	 *      The carrier node presence status value.
	 */
	public int value() {
		switch (this) {
			case None:
				return 0;
			case Away:
				return 1;
			case Busy:
			default:
				return 0;
		}
	}

	/**
	 * Get debug description of carrier node presence.
	 *
	 * @param
	 *      status        The carrier node presense status.
	 *
	 * @return
	 *      The debug description of presence
	 */
	public static String format(PresenceStatus status) {
		return String.format("%s[%d]", status.name(), status.value());
	}

	/**
	 * Get debug description of current node presence.
	 *
	 * @return
	 *      The debug description of current node presence.
	 */
	@Override
	public String toString() {
		return format(this);
	}
}
