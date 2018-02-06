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
 * Carrier network topology for session peers related to each other.
 */
public enum NetworkTopology {
	/**
	 * LAN network topology.
	 */
	LAN,
	/**
	 * P2P network topology,
	 */
	P2P,
	/**
	 * Relayed netowrk topology.
	 */
	Relayed;

	/**
	 * Get network topolgoy.
	 *
	 * @param
	 *      type      The value of topolgoy.
	 *
	 * @return
	 *      The network topology instance.
	 *
	 * @throws
	 *      IllegalArgumentException
	 */
	public static NetworkTopology valueOf(int type) {
		switch (type) {
			case 0:
				return LAN;
			case 1:
				return P2P;
			case 2:
				return Relayed;
			default:
				throw new IllegalArgumentException("Invalid network topology (expected: 0 ~ 3, Gieven:" + type);
		}
	}

	/**
	 * Get netowrk topology type.
	 *
	 * @return
	 *      The topology value.
	 */
	public int value() {
		switch (this) {
			case LAN:
				return 0;
			case P2P:
				return 1;
			case Relayed:
			default:
				return 2;
		}
	}

	/**
	 * Get the fully formatized string of network topology type.
	 *
	 * @param
	 *      type      The network topology instance.
	 *
	 * @return
	 *      The formatized string of network topology.
	 */
	public static String format(NetworkTopology type) {
		return String.format("%s[%d]", type.name(), type.value());
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
