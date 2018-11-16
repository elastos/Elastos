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
 * A class representing the Carrier friend information.
 *
 * Include the basic user information and the extra friend information.
 */
public class FriendInfo extends UserInfo {
	/**
	 * Friend label name max length.
	 */
	public static final int MAX_LABEL_LEN = 63;

	private PresenceStatus presence;
	private ConnectionStatus connection;
	private String label;

	protected FriendInfo() {
		presence = PresenceStatus.None;
		connection = ConnectionStatus.Disconnected;
	}

	/**
	 * Set friend's label name.
	 *
	 * This function will be called in Java JNI only.
	 *
	 * @param
	 * 		label			The new label to set.
	 */
	public void setLabel(String label) {
		this.label = label;
	}

	/**
	 * Get friend's label name.
	 *
	 * @return
	 * 		The friend's label name.
	 */
	public String getLabel() {
		return label;
	}

	/**
	 * Set friend's connection status.
	 *
	 * This function will be called in Java JNI only.
	 *
	 * @param
	 * 		status		The ConnectionStatus object.
	 */
	public void setConnectionStatus(ConnectionStatus status) {
		this.connection = status;
	}

	/**
	 * Get friend's connection status.
	 *
	 * @return
	 * 		The ConnectionStatus object.
	 */
	public ConnectionStatus getConnectionStatus() {
		return this.connection;
	}


	/**
	 * Set friend's presence.
	 *
	 * This function will be called in Java JNI only.
	 *
	 * @param
	 * 		status		The presence status.
	 */
	public void setPresence(PresenceStatus status) {
		this.presence = status;
	}

	/**
	 * Get friend's presence.
	 *
	 * @return
	 * 		The presence of friend.
	 */
	public PresenceStatus getPresence() {
		return presence;
	}

	/**
	 * Get formatted debug description of current FriendInfo object.
 	 *
 	 * @return
 	 * 		The debug description of current FriendInfo object.
	 */
	@Override
	public String toString() {
		return String.format("FriendInfo[%s, label:%s, presence:%s, connection:%s]",
				super.toString(), label, presence, connection);
	}
}
