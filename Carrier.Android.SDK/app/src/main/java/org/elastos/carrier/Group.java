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

import java.util.List;
import java.util.ArrayList;

import org.elastos.carrier.exceptions.CarrierException;

public class Group {
	/**
	 * Carrier App message max length.
	 */
	public static final int MAX_APP_MESSAGE_LEN = 1024;

	/**
	 * Max Carrier group title length.
	 */
	public static final int MAX_GROUP_TITLE_LEN = 127;

	private String groupId;
	private Carrier carrier;

	private static native String new_group(Carrier carrier);
	private static native String group_join(Carrier carrier, String friendId, byte[] cookie);
	private native boolean leave_group(Carrier carrier, String groupId);
	private native boolean group_invite(Carrier carrier, String groupId, String friendId);

	private native boolean group_send_message(Carrier carrier, String groupId, byte[] message);
	private native String group_get_title(Carrier carrier, String groupId);
	private native boolean group_set_title(Carrier carrier, String groupId, String title);
	private native boolean group_get_peers(Carrier carrier, String groupId, GroupPeersIterator iterator, Object context);
	private native PeerInfo group_get_peer(Carrier carrier, String groupId, String peerId);

	private static native int get_error_code();

	Group(Carrier carrier, String groupId) {
		this.carrier = carrier;
		this.groupId = groupId;
	}

	Group(Carrier carrier) throws CarrierException {
		groupId = new_group(carrier);
		if (groupId == null)
			throw CarrierException.fromErrorCode(get_error_code());

		this.carrier = carrier;
	}

	Group(Carrier carrier, String friendId, byte[] cookie) throws CarrierException {
		groupId = group_join(carrier, friendId, cookie);
		if (groupId == null)
			throw CarrierException.fromErrorCode(get_error_code());

		this.carrier = carrier;
	}

	void leave() throws CarrierException {
		if (!leave_group(carrier, groupId))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	String getId() {
		return groupId;
	}

	/**
	 * Invite a specified friend into group.
	 *
	 * @param
	 *	  friendId	The invited friendId
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public void invite(String friendId) throws CarrierException {
		if (friendId == null || friendId.length() == 0)
			throw new IllegalArgumentException();

		if (!group_invite(carrier, groupId, friendId))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * Send a message to a group.
	 *
	 * The message length may not exceed MAX_APP_MESSAGE_LEN. Larger messages
	 * must be split by application and sent as separate fragments. Other carrier
	 * nodes can reassemble the fragments.
	 *
	 * Message may not be empty or null.
	 * @param
	 *	  message	 The message content defined by application
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public void sendMessage(byte[] message) throws CarrierException {
		if (message == null || message.length == 0 || message.length > MAX_APP_MESSAGE_LEN)
			throw new IllegalArgumentException();

		if (!group_send_message(carrier, groupId, message))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * Get group title.
	 *
	 * @return
	 * 		The title of the specified group
	 *
	 * @throws CarrierException  carrier exception.
	 */
	public String getTitle() throws CarrierException {
		String title = group_get_title(carrier, groupId);
		if (title == null)
			throw CarrierException.fromErrorCode(get_error_code());

		return title;
	}

	/**
	 * Set group title.
	 *
	 * @param
	 *	  title	   The title name to set(should be no longer than MAX_GROUP_TITLE_LEN)
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public void setTitle(String title) throws CarrierException {
		if (title == null || title.length() == 0 || title.length() > MAX_GROUP_TITLE_LEN)
			throw new IllegalArgumentException();

		if (!group_set_title(carrier, groupId, title))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * A class representing group peer information.
	 *
	 * Include the basic userid and it's nickname.
	 */
	static public class PeerInfo {
		private String userId;
		private String name;

		private PeerInfo(String name, String userId) {
			this.userId = userId;
			this.name = name;
		}

		public String getName() {
			return name;
		}

		public String getUserId() {
			return userId;
		}
	}

	/**
	 * Get group peer list.
	 *
	 * @return
	 * 		A list of all peers in the specified group
	 *
	 * @throws CarrierException  carrier exception.
	 */
	public List<PeerInfo> getPeers() throws CarrierException {
		List<PeerInfo> peers = new ArrayList<PeerInfo>();
		boolean result = group_get_peers(carrier, groupId, new GroupPeersIterator() {
			public boolean onIterated(PeerInfo peerInfo, Object context) {
				@SuppressWarnings({"unchecked"})
				List<PeerInfo> peers = (List<PeerInfo>)context;
				if (peerInfo != null)
					peers.add(peerInfo);
				return true;
			}
		}, peers);

		if (!result)
			throw CarrierException.fromErrorCode(get_error_code());

		return peers;
	}

	/**
	 * Get group peer information.
	 *
	 * @param
	 *	  peerId	  The target peerId to get it's information
	 *
	 * @return
	 * 		Information of the specified peer
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public PeerInfo getPeer(String peerId) throws CarrierException {
		if (peerId == null || peerId.length() == 0)
			throw new IllegalArgumentException();

		PeerInfo peerInfo = group_get_peer(carrier, groupId, peerId);
		if (peerInfo == null)
			throw CarrierException.fromErrorCode(get_error_code());

		return peerInfo;
	}
}
