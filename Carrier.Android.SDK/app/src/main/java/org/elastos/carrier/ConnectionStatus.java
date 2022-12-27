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
 * Carrier node connection status to the carrier network.
 */
public enum ConnectionStatus {
	/**
	 * Carrier node connected to the carrier network.
	 * Indicate the node is online.
	 */
	Connected,

	/**
	 * There is no connection to the carrier network.
	 * Indicate the node is offline.
	 */
	Disconnected;

	/**
	 * Get ConnectionStatus object from staus integter value.
	 *
	 * @param
	 * 		status			The status value
	 *
	 * @return
	 * 		The ConnectionStatus object
	 *
	 * @throws IllegalArgumentException illegal exception.
	 */
	public static ConnectionStatus valueOf(int status) {
		switch (status) {
			case 0:
				return Connected;
			case 1:
				return Disconnected;
			default:
				throw new IllegalArgumentException("Invalid Connection Status (expected: 0 ~ 1, " +
						"Gieven:" + status);
		}
	}

	/**
	 * Get the status value of current ConnectionStatus object.
	 *
	 * @return
	 * 		The connection status value.
	 */
	public int value() {
		switch (this) {
			case Connected:
				return 0;
			case Disconnected:
			default:
				return 1;
		}
	}

	/**
	 * Get the debug description of the ConnectionStatus object.
	 *
	 * @param
	 * 		status		The connect status
	 *
	 * @return
	 * 		The debug description of ConnectionStatus object
	 */
	static String format(ConnectionStatus status) {
		return String.format("%s[%d]", status.name(), status.value());
	}

	/**
	 * Get the debug description of current ConnectionStatus object.
	 *
	 * @return
	 * 		The debug description of current ConnectionStatus object.
	 */
	@Override
	public String toString() {
		return format(this);
	}
}