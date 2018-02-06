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
 * Carrier Stream's candidate type.
 */
public enum CandidateType {
	/**
	 * Host candidate.
	 */
	Host,

	/**
	 * Server reflexive, only valid to ICE transport.
	 */
	ServerReflexive,

	/**
	 * Peer reflexive, only valid to ICE transport.
	 */
	PeerReflexive,

	/**
	 * Relayed Candidate, only valid to ICE tranport.
	 */
	Relayed;

	/**
	 * Get candidate type
	 *
	 * @param
	 *      type      The value of candidate type.
	 *
	 * @return
	 *      The close reason instance.
	 *
	 * @throws
	 *      IllegalArgumentException
	 */
	public static CandidateType valueOf(int type) {
		switch (type) {
			case 0:
				return Host;
			case 1:
				return ServerReflexive;
			case 2:
				return PeerReflexive;
			case 3:
				return Relayed;
			default:
				throw new IllegalArgumentException("Invalid candidate type(expected: 0 ~ 3, Gieven:" + type);
		}
	}

	/**
	 * Get candidate type.
	 *
	 * @return
	 *      The candidate value.
	 */
	public int value() {
		switch (this) {
			case Host:
				return 0;
			case ServerReflexive:
				return 1;
			case PeerReflexive:
				return 2;
			case Relayed:
			default:
				return 2;
		}
	}

	/**
	 * Get the fully formatized string of candidate type.
	 *
	 * @param
	 *      type      The candidate type instance.
	 *
	 * @return
	 *      The formatized string of candidate type.
	 */
	public static String format(CandidateType type) {
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
