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
import java.util.Date;

/**
 * The abstract carrier node handler class.
 */

public abstract class AbstractCarrierHandler extends AbstractGroupHandler implements CarrierHandler {
	/**
	 * The callback function that perform idle work.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 */
	@Override
	public void onIdle(Carrier carrier) {}

	/**
	 * The callback function to process the self connection status.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 * @param
	 * 		status 		Current connection status. @see ConnectionStatus
	 */
	@Override
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
	@Override
	public void onReady(Carrier carrier) {}

	/**
	 * The callback function to process the self info changed event.
	 *
	 * @param
	 * 		carrier 	Carrier node instance
	 * @param
	 * 		info 	The updated user information
	 */
	@Override
	public void onSelfInfoChanged(Carrier carrier, UserInfo info) {}

	/**
	 * The callback function to iterate the each friend item in friend list.
	 *
	 * @param
	 * 		carrier    	Carrier node instance
	 * @param
	 * 		friends 	The friends list.
	 */
	@Override
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
	@Override
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
	@Override
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
	@Override
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
	@Override
	public void onFriendRequest(Carrier carrier, String userId, UserInfo info, String hello) {}

	/**
	 * The callback function to process the new friend added event.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 * @param
	 * 		info	The added friend's information
	 */
	@Override
	public void onFriendAdded(Carrier carrier, FriendInfo info) {}

	/**
	 * The callback function to process the friend removed event.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 * @param
	 * 		friendId   	The friend's user id
	 */
	@Override
	public void onFriendRemoved(Carrier carrier, String friendId) {}

	/**
	 * The callback function to process the friend message.
	 *
	 * @param
	 * 		carrier   	Carrier node instance
	 * @param
	 * 		from     	The id(userid@nodeid) from who send the message
	 * @param
	 *      timestamp   The message sent time as the number of seconds
	 *                  since the Epoch, 1970-01-01 00:00:00 +0000 (UTC)
	 * @param
	 *      isOffline	The boolean value means this message is received as offline message
	 *                  or online message. The vlaue of true means the message is received
	 *                  as offline message, otherwise as online message.
	 */
	@Override
	public void onFriendMessage(Carrier carrier, String from, byte[] message, Date timestamp, boolean isOffline) {}

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
	@Override
	public void onFriendInviteRequest(Carrier carrier, String from, String data) {}


	/**
	 * The callback function to process the group invite request.
	 *
	 * @param
	 *      carrier     Carrier node instance
	 * @param
	 *      from        The user id from who send the invite request
	 * @param
	 *      cookie      The application defined cookie send from friend
	 */
	@Override
	public void onGroupInvite(Carrier carrier, String from, byte[] cookie) {}
}