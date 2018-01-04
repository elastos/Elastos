package org.elastos.carrier;

/*
 * The interface to iterate each friend item in friends list.
 */
interface FriendsIterator {

	/**
	 * The callback function to iterate friends.
	 *
	 * @param
	 * 		friendInfo		The friend information that representing a friend.
	 * @param
	 * 		context:		The application defined context data.
	 *
     * @return
	 * 		True to continue iterate next friend user info, false to stop iteration.
     */
	boolean onIterated(FriendInfo friendInfo, Object context);
}
