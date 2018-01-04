package org.elastos.carrier;

/**
 * The interface to process the friend invite response.
 */
public interface FriendInviteResponseHandler {

	/**
	 * The callback function to process the friend invite response.
	 *
	 * @param
	 * 		from		The target user id who send friend invite response
	 * @param
	 * 		status		The status code of invite response. 0 is success, otherwise error
	 * @param
	 * 		reason		The error message if status is error, otherwise null
	 * @param
	 * 		data		The application defined data return by target user
     */
	void onReceived(String from, int status, String reason, String data);
}
