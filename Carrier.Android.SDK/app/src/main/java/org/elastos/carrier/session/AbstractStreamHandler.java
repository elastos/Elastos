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
 * The abstract carrier stream handler class.
 *
 */
public class AbstractStreamHandler implements StreamHandler {

	/* Common callbacks */
	/**
	 * The callback function to report state of stream when it's state changes.
	 *
	 * @param
	 *      stream      The carrier stream instance
	 * @param
	 *      state       Stream state defined in StreamState
	 */
	public void onStateChanged(Stream stream, StreamState state) {}

	/* Stream layered data callbacks */
	/**
	 * The callback will be called when the stream receives incoming packet.
	 *
	 * If the stream enabled multiplexing mode, application will not
	 * receive stream-layered data callback any more. All data will reported
	 * as multiplexing channel data.
	 *
	 * @param
	 *      stream      The carrier stream instance
	 * @param
	 *      data        The received packet data
	 */
	public void onStreamData(Stream stream, byte[] data) {}

	/* Channel callbacks */
	/**
	 * The callback function to be called when new multiplexing channel request to open.
	 *
	 * @param
	 *      stream      The carrier stream instance
	 * @param
	 *      channel     The current channel ID.
	 * @param
	 *      cookie      Application defined string data send from remote peer.
	 *
	 * @return
	 *      True on success, or false if an error occurred.
	 *      The channel will continue to open only this callback return true,
	 *      otherwise the channel will be closed.
	 */
	public boolean onChannelOpen(Stream stream, int channel, String cookie) {
		return true;
	}

	/**
	 * The callback function to be called when new multiplexing channel opened.
	 *
	 * @param
	 *      stream      The carrier stream instance
	 * @param
	 *      channel     The current channel ID
	 */
	public void onChannelOpened(Stream stream, int channel) {}

	/**
	 * The callback function to be called when channel close.
	 *
	 * @param
	 *      stream      The carrier stream instance
	 * @param
	 *      channel     The current channel ID.
	 * @param
	 *      reason      Channel close reason code, defined in CloseReason.
	 */
	public void onChannelClose(Stream stream, int channel, CloseReason reason) {}

	/**
	 * The callback functiont to be called when channel received incoming data.
	 *
	 * @param
	 *      stream      The carrier stream instance
	 * @param
	 *      channel     The current channel ID
	 * @param
	 *      data        The received data
	 *
	 * @return
	 *      True on success, or false if an error occurred.
	 *      If this callback return false, the channel will be closed
	 *      with CloseReason_Error.
	 */
	public boolean onChannelData(Stream stream, int channel, byte[] data) {
		return true;
	}

	/**
	 * The callback function to be called when remote peer ask to pend data sending.
	 *
	 * @param
	 *      stream      The carrier stream instance
	 * @param
	 *      channel     The current channel ID
	 */
	public void onChannelPending(Stream stream, int channel) {}

	/**
	 * The callback function to be called when remote peer ask to resume data sending.
	 *
	 * @param
	 *      stream      The carrier stream instance
	 * @param
	 *      channel     The current channel ID
	 */
	public void onChannelResume(Stream stream, int channel) {}
}
