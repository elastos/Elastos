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
 * Port forwarding supported protocols.
 */
public enum PortForwardingProtocol {
	/**
	 * TCP protocol.
	 */
	TCP;

	/**
	 * Get port forwarding protocol from protocol value.
	 *
	 * @param
	 *      protocol        The protocol value
	 *
	 * @return
	 *      The port forwarding protocol
	 *
	 * @throws
	 *      IllegalArgumentException
	 */
	public static PortForwardingProtocol valueOf(int protocol) {
		switch (protocol) {
			case 1:
				return TCP;
			default:
				throw new IllegalArgumentException("Invalid Protocol (expected: 0 ~ 1, Gieven:" + protocol);
		}
	}

	/**
	 * Get value of port forwarding protocol.
	 *
	 * @return
	 *      The value of port forwarding protocol
	 */
	public int value() {
		switch (this) {
			case TCP:
				return 1;
			default:
				return 0;
		}
	}

	/**
	 * Get the debug description of port forwarding protocol
	 *
	 * @param
	 *      protocol        The port forwarding protocol
	 *
	 * @return
	 *      The deubg description of port forwarding protocol
	 */
	public static String format(PortForwardingProtocol protocol) {
		return String.format("%s", protocol.name());
	}

	/**
	 * Get the debug description
	 *
	 * @return
	 *      The debug description of current port forwarding protocol
	 */
	@Override
	public String toString() {
		return format(this);
	}
}
