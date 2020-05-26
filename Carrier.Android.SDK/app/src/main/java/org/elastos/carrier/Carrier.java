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

import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.util.List;
import java.util.Date;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Collection;
import java.util.Collections;

import org.elastos.carrier.exceptions.CarrierException;

/**
 * The class representing Carrier node instance.
 */
public class Carrier {
	private Charset UTF8 = Charset.forName("UTF-8");

	/**
	 * Max Carrier ID length.
	 */
	public static final int MAX_ID_LEN = 45;

	/**
	 * Max Carrier KEY length.
	 */
	public static final int MAX_KEY_LEN = 45;

	/**
	 * Carrier App message max length.
	 */
	public static final int ELA_MAX_APP_BULKMSG_LEN = (5 * 1024 * 1024);

	private static final String TAG = "CarrierCore";
	private Thread carrierThread;
	private CarrierHandler handler;
	private long nativeCookie = 0;  // store the native (JNI-layered) carrier handler
	private boolean didKill = false;
	private Hashtable<String, Group> groups = new Hashtable<>();

	static {
		System.loadLibrary("carrierjni");
	}

	private static class Callbacks {
		private List<FriendInfo> friends;

		void onIdle(Carrier carrier) {
			carrier.handler.onIdle(carrier);
		}

		void onConnection(Carrier carrier, ConnectionStatus status) {
			carrier.handler.onConnection(carrier, status);
		}

		void onReady(Carrier carrier) {
			carrier.handler.onReady(carrier);
		}

		void onSelfInfoChanged(Carrier carrier, UserInfo userInfo) {
			carrier.handler.onSelfInfoChanged(carrier, userInfo);
		}

		boolean onFriendsIterated(Carrier carrier, FriendInfo info) {
			if (friends == null)
				friends = new ArrayList<FriendInfo>();

			if (info != null) {
				friends.add(info);
			} else {
				carrier.handler.onFriends(carrier, friends);
				friends = null;
			}
			return true;
		}

		void onFriendConnection(Carrier carrier, String friendid, ConnectionStatus status) {
			carrier.handler.onFriendConnection(carrier, friendid, status);
		}

		void onFriendInfoChanged(Carrier carrier, String friendId, FriendInfo info) {
			carrier.handler.onFriendInfoChanged(carrier, friendId, info);
		}

		void onFriendPresence(Carrier carrier, String friendId, PresenceStatus presence) {
			carrier.handler.onFriendPresence(carrier, friendId, presence);
		}

		void onFriendRequest(Carrier carrier, String userId, UserInfo info, String hello) {
			carrier.handler.onFriendRequest(carrier, userId, info, hello);
		}

		void onFriendAdded(Carrier carrier, FriendInfo friendInfo) {
			carrier.handler.onFriendAdded(carrier, friendInfo);
		}

		void onFriendRemoved(Carrier carrier, String friendId) {
			carrier.handler.onFriendRemoved(carrier, friendId);
		}

		void onFriendMessage(Carrier carrier, String from, byte[] message, Date timestamp, boolean isOffline) {
			carrier.handler.onFriendMessage(carrier, from, message, timestamp, isOffline);
		}

		void onFriendInviteRequest(Carrier carrier, String from, String data) {
			carrier.handler.onFriendInviteRequest(carrier, from, data);
		}

		void onGroupInvite(Carrier carrier, String from, byte[] cookie) {
			carrier.handler.onGroupInvite(carrier, from, cookie);
		}

		void onGroupConnected(Carrier carrier, String groupId) {
			Group group = carrier.groups.get(groupId);
			if (group != null)
				carrier.handler.onGroupConnected(group);
		}

		void onGroupMessage(Carrier carrier, String groupId, String from, byte[] message) {
			Group group = carrier.groups.get(groupId);
			if (group != null)
				carrier.handler.onGroupMessage(group, from, message);
		}

		void onGroupTitle(Carrier carrier, String groupId, String from, String title) {
			Group group = carrier.groups.get(groupId);
			if (group != null)
				carrier.handler.onGroupTitle(group, from, title);
		}

		void onPeerName(Carrier carrier, String groupId, String peerId, String peerName) {
			Group group = carrier.groups.get(groupId);
			if (group != null)
				carrier.handler.onPeerName(group, peerId, peerName);
		}

		void onPeerListChanged(Carrier carrier, String groupId) {
			Group group = carrier.groups.get(groupId);
			if (group != null)
				carrier.handler.onPeerListChanged(group);
		}
	}

	/**
	 * Options defines several settings that control the way the Carrier node
	 * connects to the carrier network.
	 *
	 * Default values are not defined for bootstraps options, so application
	 * should be set bootstrap nodes clearly.
	 */
	public static class Options {
		private String persistentLocation;
		private boolean udpEnabled;
		private List<BootstrapNode> bootstrapNodes;
		private List<ExpressNode> expressNodes;

		public static class BootstrapNode {
			private String ipv4;
			private String ipv6;
			private String port;
			private String publicKey;

			public BootstrapNode setIpv4(String ipv4) {
				this.ipv4 = ipv4;
				return this;
			}

			public String getIpv4() {
				return ipv4;
			}

			public BootstrapNode setIpv6(String ipv6) {
				this.ipv6 = ipv6;
				return this;
			}

			public String getIpv6() {
				return ipv6;
			}

			public BootstrapNode setPort(String port) {
				this.port = port;
				return this;
			}

			public String getPort() {
				return port;
			}

			public BootstrapNode setPublicKey(String publicKey) {
				this.publicKey = publicKey;
				return this;
			}

			public String getPublicKey() {
				return publicKey;
			}
		}

		public static class ExpressNode {
			private String ipv4;
			private String port;
			private String publicKey;

			public ExpressNode setIpv4(String ipv4) {
				this.ipv4 = ipv4;
				return this;
			}

			public String getIpv4() {
				return ipv4;
			}

			public ExpressNode setPort(String port) {
				this.port = port;
				return this;
			}

			public String getPort() {
				return port;
			}

			public ExpressNode setPublicKey(String publicKey) {
				this.publicKey = publicKey;
				return this;
			}

			public String getPublicKey() {
				return publicKey;
			}
		}

		/**
		 * Set the persistent data location.
		 * The location must be set.
		 *
		 * @param persistentLocation The persistent data location to set
		 *
		 * @return The current Options object reference.
		 */
		public Options setPersistentLocation(String persistentLocation) {
			this.persistentLocation = persistentLocation;
			return this;
		}

		/**
		 * Get the persistent data location.
		 *
		 * @return The persistent data location
		 */
		public String getPersistentLocation() {
			return persistentLocation;
		}

		/**
		 * Set to use udp transport or not.
		 * Setting this value to false will force carrier node to TCP only, which will
		 * potentially slow down the message to run through.
		 *
		 * @param udpEnabled flag to enable or disable udp transport.
		 *
		 * @return The current options object reference.
		 */
		public Options setUdpEnabled(boolean udpEnabled) {
			this.udpEnabled = udpEnabled;
			return this;
		}

		/**
		 * Get the udp transport used or not.
		 *
		 * @return	The value of enable/dsiable udp transport.
		 */
		public boolean getUdpEnabled() {
			return udpEnabled;
		}

		public Options setBootstrapNodes(List<BootstrapNode> bootstrapNodes) {
			this.bootstrapNodes = bootstrapNodes;
			return this;
		}

		public List<BootstrapNode> getBootstrapNodes() {
			return bootstrapNodes;
		}

		public Options setExpressNodes(List<ExpressNode> expressNodes) {
			this.expressNodes = expressNodes;
			return this;
		}

		public List<ExpressNode> getExpressNodes() {
			return expressNodes;
		}
	}

	// native jni methods.
	private native boolean native_init(Options options, Callbacks callbacks);
	private native boolean native_run(int interval);
	private native void  native_kill();

	private native String get_address();
	private native String get_node_id();

	private native boolean set_nospam(byte[] nospam);
	private native byte[] get_nospam();

	private native boolean set_self_info(UserInfo uerInfo);
	private native UserInfo get_self_info();

	private native boolean set_presence(PresenceStatus status);
	private native PresenceStatus get_presence();

	private native boolean is_ready();

	private native boolean get_friends(FriendsIterator iterator, Object context);
	private native FriendInfo get_friend(String userId);
	private native boolean label_friend(String userId, String label);
	private native boolean is_friend(String userId);

	private native boolean add_friend(String address, String hello);
	private native boolean accept_friend(String userId);
	private native boolean remove_friend(String userId);

	private native int send_message(String to, byte[] message);
	private native long send_message_with_receipt(String to, byte[] message,
												  FriendMessageReceiptHandler handler);
	private native boolean friend_invite(String to, String data,
										 FriendInviteResponseHandler handler);
	private native boolean reply_friend_invite(String from, int status, String reason,
											   String data);

	private static native int get_error_code();
	private static native String get_version();

	private Carrier(CarrierHandler handler) {
		this.handler = handler;
	}

	/**
	 * Get current version of Carrier node.
	 *
	 * @return
	 * 		The version of carrier node.
	 */
	public static String getVersion() {
		return get_version();
	}

	/**
	 * Check if the ID is Carrier node id.
	 *
	 * @param
	 * 		id		The carrier node id to be check.
	 *
	 * @return
	 * 		True if id is valid, otherwise false.
	 */
	public static boolean isValidId(String id) {
		try {
			return Base58.decode(id).length == 32;
		} catch (Exception e) {
			return false;
		}
	}

	/**
	 * Check if the carrier node address is valid.
	 *
	 * @param
	 * 		address			The carrier node address to be check.
	 *
	 * @return
	 * 		True if key is valid, otherwise false.
	 */
	public static boolean isValidAddress(String address) {
		try {
			return Base58.decode(address).length == 38;
		} catch (Exception e) {
			return false;
		}
	}

	/**
	 * Get carrier ID from carrier node address.
	 *
	 * @param
	 * 		address			The carrier node address.
	 *
	 * @return
	 * 		User id if address is valid, otherwise null
	 */
	public static String getIdFromAddress(String address) {
		try {
			byte[] addr = Base58.decode(address);

			if (addr.length != 38)
				return null;

			return Base58.encode(Arrays.copyOf(addr, 32));
		} catch (Exception e) {
			return null;
		}
	}

	/**
	 * Create node instance. After returning, it's ready to start and
	 * therefore connect to carrier network.
	 *
	 * @param
	 * 		options		The options to set for creating carrier node.
	 * @param
	 * 		handler		The interface handler for carrier node.
	 *
	 * @return
	 * 		New Carrier instance
	 *
	 * @throws CarrierException carrier exception.
	 */
	public static Carrier createInstance(Options options, CarrierHandler handler) throws CarrierException {
		if (options == null || handler == null)
			throw new IllegalArgumentException();

		Callbacks callbacks = new Callbacks();
		Carrier tmp = new Carrier(handler);

		if (!tmp.native_init(options, callbacks))
			throw CarrierException.fromErrorCode(get_error_code());

		Log.i(TAG, "Carrier node instance created");

		return tmp;
	}

	@Override
	protected void finalize() throws Throwable {
		kill();
		super.finalize();
	}

	/**
	 * Start carrier node asynchronously to connect to carrier network. If the connection
	 * to network is successful, carrier node starts working.
	 *
	 * @param
	 * 		iterateInterval		Internal loop interval, in milliseconds.
	 */
	public void start(final int iterateInterval) {
		if (carrierThread == null) {
			carrierThread = new Thread() {
				@Override
				public void run() {
					Log.i(TAG, "Native carrier node started");
					if (!native_run(iterateInterval)) {
						Log.e(TAG, "Native carrier node started error(" + get_error_code() + ")");
						return;
					}
					Log.i(TAG, "Native carrier node stoped");
				}
			};
			carrierThread.start();
		}
	}

	/**
	 * Disconnect carrier node from carrier network, and destroy all associated resources to
	 * carreier node instance.
	 *
	 * After calling the method, the carrier node instance becomes invalid.
	 */
	public synchronized void kill() {
		if (!didKill) {

			Log.i(TAG, "Killing Carrier node instance ...");
			native_kill();
			didKill = true;

			if (carrierThread != null) {
				try {
					carrierThread.join();
				} catch (Exception e) {
					Log.i(TAG, "Join carrier thread is interrupted");
				}
				carrierThread = null;
			}

			Log.i(TAG, "Carrier instance killed");
		}
	}

	/**
	 * Get node address associated with the carrier node instance.
	 *
	 * @return
	 *  	the node address.
	 * @throws CarrierException carrier exception.
	 */
	public String getAddress() throws CarrierException {
		String address = get_address();
		if (address == null)
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Current carrier address: " + address);
		return address;
	}

	/**
	 * Get nodeid associated with the carrier node instance.
	 *
	 * @return
	 * 		the nodeid.
	 * @throws CarrierException carrier exception.
	 */
	public String getNodeId() throws CarrierException {
		String nodeId = get_node_id();
		if (nodeId == null)
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Current carrier NodeId: " + nodeId);
		return nodeId;
	}

	/**
	 * Get userid associated with the carrier node instance.
	 *
	 * @return
	 * 		the userid.
	 * @throws CarrierException carrier exception.
	 */
	public String getUserId() throws CarrierException {
		String userId = getNodeId();
		Log.d(TAG, "Current carrier userId: " + userId);
		return userId;
	}

	/**
	 * Update self nospam of address for this carrier node.
	 *
	 * Update the nospam of carrier node address with host byte order
	 * expected. Nospam for Carrier address is used to eliminate spam friend
	 * request.
	 *
	 * @param
	 * 		nospam 			An integer value.
	 *
	 * @throws CarrierException carrier exception.
	 */
	public void setNospam(int nospam) throws CarrierException {
		byte[] value = ByteBuffer.allocate(4).putInt(nospam).array();
		if (!set_nospam(value))
			throw CarrierException.fromErrorCode(get_error_code());
	}

	/**
	 * \~Egnlish
	 * Get the nospam for Carrier address.
	 *
	 * Get the 4-byte nospam part of the Carrier address with host byte order
	 * expected. Nospam for Carrier address is used to eliminate spam friend
	 * request.
	 *
	 * @return the nospam for Carrier address.
	 * @throws CarrierException carrier exception.
	 */
	public int getNospam() throws CarrierException {
		byte[] nospam = get_nospam();
		if (nospam == null)
			throw CarrierException.fromErrorCode(get_error_code());

		return ByteBuffer.wrap(nospam).getInt();
	}

	/**
	 * Update self user information.
	 *
	 * After self user information changed, carrier node will update this information
	 * to carrier network, and thereupon network broadcasts the change to all friends.
	 *
	 * @param
	 * 		userinfo	The user information to update for this carrier node.
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException carrier exception.
	 */
	public void setSelfInfo(UserInfo userinfo) throws CarrierException {
		if (userinfo == null)
			throw new IllegalArgumentException();

		if (!set_self_info(userinfo))
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Current user information updated");
	}

	/**
	 * Get self user information.
	 *
	 * @return
	 * 		the user information to the carrier node.
	 *
	 * @throws CarrierException carrier exception.
	 */

	public UserInfo getSelfInfo() throws CarrierException {
		UserInfo userInfo = get_self_info();
		if (userInfo == null)
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Current user information: " + userInfo);
		return userInfo;
	}

	/**
	 * Update self presence status.
	 *
	 * @param
	 * 		presence 			the new presence status.
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException carrier exception.
	 */
	public void setPresence(PresenceStatus presence) throws CarrierException {
		if (presence == null)
			throw new IllegalArgumentException();

		if (!set_presence(presence))
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Current presence updated to be " + presence);
	}

	/**
	 * Get self presence status.
	 * @return self presence status
	 * @throws CarrierException carrier exception.
	 */
	public PresenceStatus getPresence() throws CarrierException {
		PresenceStatus presence = get_presence();
		if (presence == null)
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Current presence " + presence);
		return presence;
	}

	/**
	 * \~English
	 * Check if carrier node instance is being ready.
	 *
	 * All carrier interactive APIs should be called only if carrier node instance
	 * is being ready.
	 *
	 * @return
	 *      true if the carrier node instance is ready, or false if not.
	 *
	 */
	public boolean isReady() {
		return is_ready();
	}

	/**
	 * Get friends list.
	 *
	 * @return
	 * 		The list of friend information to current user
	 *
	 * @throws CarrierException carrier exception.
	 */
	public List<FriendInfo> getFriends() throws CarrierException {
		List<FriendInfo> friends = new ArrayList<FriendInfo>();

		boolean result = get_friends(new FriendsIterator() {
			public boolean onIterated(FriendInfo info, Object context) {
				@SuppressWarnings({"unchecked"})
				List<FriendInfo> friends = (List<FriendInfo>)context;
				if (info != null)
					friends.add(info);
				return true;
			}
		}, friends);

		if (!result)
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Current user's friends listed below: +++++>>");
		for (FriendInfo friend: friends) {
			Log.d(TAG, friend.toString());
		}
		Log.d(TAG, "<<++++++++");

		return friends;
	}

	/**
	 * Get specified friend information.
	 *
	 * @param
	 * 		userId		The user identifier of friend
	 *
	 * @return
	 * 		The friend information.
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public FriendInfo getFriend(String userId) throws CarrierException {
		if (userId == null || userId.length() == 0)
			throw new IllegalArgumentException();

		FriendInfo friendInfo = get_friend(userId);
		if (friendInfo == null)
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "The information of friend " + userId + ": " + friendInfo);
		return friendInfo;
	}

	/**
	 * Set the label of the specified friend.
	 *
	 * The label of a friend is a private alias name for current user. It can be
	 * seen by current user only, and has no impact to the target friend itself.
	 *
	 * @param
	 * 		userId			The friend's user identifier
	 * @param
	 * 		label			The new label of specified friend
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public void labelFriend(String userId, String label) throws CarrierException {
		if (userId == null || userId.length() == 0 ||
			label  == null || label.length() == 0)
			throw new IllegalArgumentException();

		if (!label_friend(userId, label))
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Label friend " + userId + " as  " + label);
	}

	/**
	 * Check if the user ID is friend.
	 *
	 * @param
	 * 		userId 		The userId to check
	 *
	 * @return
	 * 		True if the user is a friend, or false if not
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public boolean isFriend(String userId) throws CarrierException {
		if (userId == null || userId.length() == 0)
			throw new IllegalArgumentException();

		return is_friend(userId);
	}

	/**
	 * Add friend by sending a new friend request.
	 *
	 * This function will add a new friend with specific address, and then
	 * send a friend request to the target node.
	 *
	 * @param
	 * 		address 	the target user address of remote carrier node.
	 * @param
	 * 		hello 	 	PIN for target user, or any application defined content
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public void addFriend(String address, String hello) throws CarrierException {
		if (address == null || address.length() == 0)
			throw new IllegalArgumentException();

		Log.d(TAG, "Attempt to add " + address + " to be friend by greeting with (" +
				hello + ")");

		if (!add_friend(address, hello))
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Added friend " + address + " success");
	}

	/**
	 * Accept the friend request.
	 *
	 * This function is used to add a friend in response to a friend request.
	 *
	 * @param
	 * 		userId 		The user id who want be friend with us.
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public void acceptFriend(String userId) throws CarrierException {
		if (userId == null || userId.length() == 0)
			throw new IllegalArgumentException();

		if (!accept_friend(userId))
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Accepted friend request from " + userId);
	}

	/**
	 * Remove a friend.
	 *
	 * This function will remove a friend on this carrier node.
	 *
	 * @param
	 * 		userId	The target user id to remove friendship
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public void removeFriend(String userId) throws CarrierException {
		if (userId == null || userId.length() == 0)
			throw new IllegalArgumentException();

		if (!remove_friend(userId))
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Friend " + userId + " was removed");
	}

	/**
	 * Send a message to a friend.
	 *
	 * The message length may not exceed ELA_MAX_APP_BULKMSG_LEN, and message itself
	 * should be text-formatted. Larger messages must be split by application
	 * and sent as separate messages. Other nodes can reassemble the fragments.
	 *
	 * @param
	 * 		to 			The target id
	 * @param
	 * 		message		The message content defined by application
	 *
	 * @return
	 *		Return boolean value whether this message sent as online message or
	 *		offline message. The value of true means the message was sent as
	 *		online message, otherwise as offline message.
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
    public boolean sendFriendMessage(String to, String message) throws CarrierException {
		if (to == null || to.length() == 0 ||
				message == null || message.length() == 0 || message.length() >= ELA_MAX_APP_BULKMSG_LEN)
			throw new IllegalArgumentException();

		return sendFriendMessage(to, message.getBytes(UTF8));
	}

	/**
	 * Send a message to a friend.
	 *
	 * The message length may not exceed ELA_MAX_APP_BULKMSG_LEN, and message itself
	 * should be text-formatted. Larger messages must be split by application
	 * and sent as separate messages. Other nodes can reassemble the fragments.
	 *
	 * @param
	 * 		to 			The target id
	 * @param
	 * 		message		The message content defined by application
	 *
	 * @return
	 *		Return boolean value whether this message sent as online message or
	 *		offline message. The value of true means the message was sent as
	 *		online message, otherwise as offline message.
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public boolean sendFriendMessage(String to, byte[] message) throws CarrierException {
		if (to == null || to.length() == 0 || message == null || message.length == 0)
			throw new IllegalArgumentException();

		int ret = send_message(to, message);
		if (ret < 0)
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Send " + message.length + " bytes message to friend " + to);
		return (ret == 0);
	}

	/**
	 * Send a message to a friend.
	 *
	 * The message length may not exceed ELA_MAX_APP_BULKMSG_LEN, and message itself
	 * should be text-formatted. Larger messages must be split by application
	 * and sent as separate messages. Other nodes can reassemble the fragments.
	 *
	 * @param
	 * 		to 			The target id
	 * @param
	 * 		message		The message content defined by application
	 * @param
	 *      handler     The handler to receive the receipt notification.
	 * @return
	 *      Return the message identifier which would be used for handler to
	 *      invoke receipt notification.
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public long sendFriendMessage(String to, String message, FriendMessageReceiptHandler handler)
			throws CarrierException {
		if (to == null || to.length() == 0 ||
				message == null || message.length() == 0 || message.length() >= ELA_MAX_APP_BULKMSG_LEN)
			throw new IllegalArgumentException();

		return sendFriendMessage(to, message.getBytes(UTF8), handler);
	}

	/**
	 * Send a message to a friend.
	 *
	 * The message length may not exceed ELA_MAX_APP_BULKMSG_LEN, and message itself
	 * should be text-formatted. Larger messages must be split by application
	 * and sent as separate messages. Other nodes can reassemble the fragments.
	 *
	 * @param
	 * 		to 			The target id
	 * @param
	 * 		message		The message content defined by application
	 * @param
	 *      handler     The handler to receive the receipt notification.
	 * @return
	 *      Return the message identifier which would be used for handler to
	 * 	    invoke receipt notification.
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public long sendFriendMessage(String to, byte[] message, FriendMessageReceiptHandler handler)
			throws CarrierException {
		if (to == null || to.length() == 0 || message == null || message.length == 0 ||
				handler == null)
			throw new IllegalArgumentException();

		long ret = send_message_with_receipt(to, message, handler);
		if (ret < 0)
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Send " + message.length + " bytes message with receipt ack to friend " + to);
		return ret;
	}

	/**
	 * Send invite request to a friend.
	 *
	 * Application can attach the application defined data with in the invite
	 * request, and the data will send to target friend.
	 *
	 * @param
	 * 		to			The target id
	 * @param
	 * 		data 		The application defined data send to target user
	 * @param
	 * 		handler	   	The handler to receive invite reponse
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public void inviteFriend(String to, String data, FriendInviteResponseHandler handler)
			throws CarrierException {

		if (to == null || to.length() == 0 ||
				data == null || data.length() == 0 || handler == null)
			throw new IllegalArgumentException();

		Log.d(TAG, "Inviting friend " + to + "with greet data " + data);

		if (!friend_invite(to, data, handler))
		    throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Send friend invite request to " + to);
	}

	/**
	 * Reply the friend invite request.
	 *
	 * This function will send a invite response to friend.
	 *
	 * @param
	 * 		to			The id who send invite request
	 * @param
	 * 		status		The status code of the response. 0 is success, otherwise is error
	 * @param
	 * 		reason		The error message if status is error, or null if success
	 * @param
	 * 		data		The application defined data send to target user. If the status
	 * 	                is error, this will be ignored
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public void replyFriendInvite(String to, int status, String reason, String data)
			throws CarrierException {
		if (to == null || to.length() == 0 || (status != 0 && reason == null))
			throw new IllegalArgumentException();

		if (status == 0)
			Log.d(TAG, String.format("Attempt to confirm friend invite to %s with data [%s]",
					to, data));
		else
			Log.d(TAG, String.format("Attempt to refuse friend invite to %s with status %d," +
					"and reason %s", to, status, reason));

		if (!reply_friend_invite(to, status, reason, data))
			throw CarrierException.fromErrorCode(get_error_code());

		if (status == 0)
			Log.d(TAG, String.format("Confirmed friend invite to %s with data [%s]", to, data));
		else
			Log.d(TAG, String.format("Refused friend invite to %s with status %d and " +
					"reason %s", to, status, reason));
	}

	/**
	 * Create a new group.
	 *
	 * @return
	 *	  The instance of the newly created group
	 *
	 * @throws CarrierException  carrier exception.
	 */
	public Group newGroup() throws CarrierException {
		Group group = new Group(this);
		groups.put(group.getId(), group);
		return group;
	}

	/**
	 * Join a group associating with cookie into which remote friend invites.
	 *
	 * This function should be called only if application received a group
	 * invitation from remote friend.
	 *
	 * @param
	 *	  friendId	The friend who send a group invitation
	 * @param
	 *	  cookie	The cookie information to join group
	 *
	 * @return
	 *		The instance of the group joined in
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public Group groupJoin(String friendId, byte[] cookie)
		throws CarrierException {
		if (friendId == null || cookie == null ||
				friendId.length() == 0 || cookie.length == 0)
			throw new IllegalArgumentException();

		Group group = new Group(this, friendId, cookie);
		groups.put(group.getId(), group);
		return group;
	}

	/**
	 * Leave a group.
	 *
	 * @param
	 *	  group		The instance of the group to leave
	 *
	 * @throws IllegalArgumentException illegal exception.
	 * @throws CarrierException  carrier exception.
	 */
	public void groupLeave(Group group) throws CarrierException {
		if (group == null)
			throw new IllegalArgumentException();

		Group tmp = groups.remove(group.getId());
		if (tmp == null)
			throw new IllegalArgumentException();

		try {
			group.leave();
		} catch (CarrierException e) {
			groups.put(group.getId(), group);
			throw e;
		}
	}

	/**
	 * Get groups in the Carrier instance.
	 *
	 * @return
	 * 		A collection of groups joined in
	 */
	public Collection<Group> getGroups() {
		return Collections.unmodifiableCollection(groups.values());
	}
}
