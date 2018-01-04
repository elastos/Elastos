package org.elastos.carrier;

import java.util.List;

/**
 * The abstract carrier node handler class.
 */

public abstract class AbstractCarrierHandler implements CarrierHandler {
	/**
	 * The callback function that perform idle work.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 */
	public void onIdle(Carrier carrier) {}

	/**
	 * The callback function to process the self connection status.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 * @param
	 * 		status 		Current connection status. @see ConnectionStatus
	 */
	public void onConnection(Carrier carrier, ConnectionStatus status) {}

	/**
	 * The callback function to process the ready notification.
	 *
	 * Application should wait this callback invoked before calling any carrier
	 * function to interact with friends.
	 *
	 * @param
	 * 		carrier 	Carrier node instance
	 */
	public void onReady(Carrier carrier) {}

	/**
	 * The callback function to process the self info changed event.
	 *
	 * @param
	 * 		carrier 	Carrier node instance
	 * @param
	 * 		info 	The updated user information
	 */
	public void onSelfInfoChanged(Carrier carrier, UserInfo info) {}

	/**
	 * The callback function to iterate the each friend item in friend list.
	 *
	 * @param
	 * 		carrier    	Carrier node instance
	 * @param
	 * 		friends 	The friends list.
	 */
	public void onFriends(Carrier carrier, List<FriendInfo> friends) {}

	/**
	 * The callback function to process the friend connections status changed event.
	 *
	 * @param
	 * 		carrier    	carrier node instance.
	 * @param
	 * 		friendId 	The friend's user id.
	 * @param
	 * 		status	    The connection status of friend. @see ConnectionStatus
	 */
	public void onFriendConnection(Carrier carrier, String friendId, ConnectionStatus status) {}

	/**
	 * The callback function to process the friend information changed event.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 * @param
	 * 		friendId   	The friend's user id
	 * @param
	 * 		info		The update friend information
	 */
	public void onFriendInfoChanged(Carrier carrier, String friendId, FriendInfo info) {}

	/**
	 * The callback function to process the friend presence changed event.
	 *
	 * @param
	 * 		carrier    	Carrier node instance
	 * @param
	 * 		friendId   	The friend's user id
	 * @param
	 * 		presence	The presence status of the friend
	 */
	public void onFriendPresence(Carrier carrier, String friendId, PresenceStatus presence) {}

	/**
	 * The callback function to process the friend request.
	 *
	 * @param
	 * 		carrier    	Carrier node instance
	 * @param
	 * 		userId    	The user id who want be friend with current user
	 * @param
	 * 		info		The user information to `userId`
	 * @param
	 * 		hello      	The PIN for target user, or any application defined content
	 */
	public void onFriendRequest(Carrier carrier, String userId, UserInfo info, String hello) {}

	/**
	 * The callback function to process the new friend added event.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 * @param
	 * 		info	The added friend's information
	 */
	public void onFriendAdded(Carrier carrier, FriendInfo info) {}

	/**
	 * The callback function to process the friend removed event.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 * @param
	 * 		friendId   	The friend's user id
	 */
	public void onFriendRemoved(Carrier carrier, String friendId) {}

	/**
	 * The callback function to process the friend message.
	 *
	 * @param
	 * 		carrier   	Carrier node instance
	 * @param
	 * 		from     	The id(userid@nodeid) from who send the message
	 * @param
	 * 		message   	The message content
	 */
	public void onFriendMessage(Carrier carrier, String from, String message) {}

	/**
	 * The callback function to process the friend invite request.
	 *
	 * @param
	 * 		carrier   	Carrier node instance
	 * @param
	 * 		from       	The user id from who send the invite request
	 * @param
	 * 		data       	The application defined data sent from friend
	 */
	public void onFriendInviteRequest(Carrier carrier, String from, String data) {}
}