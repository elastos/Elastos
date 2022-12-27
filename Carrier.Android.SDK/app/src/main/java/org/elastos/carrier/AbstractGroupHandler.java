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
 * The abstract carrier group handler class.
 */

public abstract class AbstractGroupHandler implements GroupHandler {
	/**
	 * The callback function that process event of being connected to group.
	 *
	 * @param
	 *      group       The target group connected
	 */
	@Override
	public void onGroupConnected(Group group) {}

	/**
	 * The callback function that process the group messages.
	 *
	 * @param
	 *      group       The group that received message
	 * @param
	 *      from        The user id who send the message
	 * @param
	 *      message     The message content
	 */
	@Override
	public void onGroupMessage(Group group, String from, byte[] message) {}

	/**
	 * The callback function that process the group title change event.
	 *
	 * @param
	 *      group       The group id of its title changed
	 * @param
	 *      from        The peer Id who changed title name
	 * @param
	 *      title       The updated title name
	 */
	@Override
	public void onGroupTitle(Group group, String from, String title) {}

	/**
	 * The callback function that process the group peer's name change event.
	 *
	 * @param
	 *      group       The target group
	 * @param
	 *      peerId      The peer Id who changed its name
	 * @param
	 *      peerName    The updated peer name
	 */
	@Override
	public void onPeerName(Group group, String peerId, String peerName) {}

	/**
	 * The callback function that process the group list change event.
	 *
	 * @param
	 *      group       The target group that changed it's peer list
	 */
	@Override
	public void onPeerListChanged(Group group) {}
}
