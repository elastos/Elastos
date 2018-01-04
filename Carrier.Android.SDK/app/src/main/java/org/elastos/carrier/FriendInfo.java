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
	void setLabel(String label) {
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
	 * 		status		The PresenceStatus object.
     */
	void setConnectionStatus(ConnectionStatus status) {
		this.connection = status;
	}

	/**
	 * Get friend's presence status.
	 *
	 * @return
	 * 		The PresenceStatus object.
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
	void setPresence(PresenceStatus status) {
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
