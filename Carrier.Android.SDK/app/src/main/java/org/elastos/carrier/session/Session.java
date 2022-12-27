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

import org.elastos.carrier.Log;
import org.elastos.carrier.exceptions.CarrierException;

/**
 * The class representing Carrier session conversion with friends.
 */
public class Session {
	private static final String TAG = "CarrierSession";

	private long nativeCookie = 0; // store native (jni-layered) session handler.

	private String to;  // with whom being conversation.
	private boolean didClose;

	/* Jni native methods. */
	private native void session_close();
	private native boolean native_request(SessionRequestCompleteHandler handler);
	private native boolean native_reply_request(int status, String reason);
	private native boolean native_start(String sdp);
	private native Stream add_stream(StreamType type, int options, StreamHandler handler);
	private native boolean remove_stream(int streamId, Stream stream);
	private native boolean add_service(String service, PortForwardingProtocol protocol,
									   String host, String port);
	private native void remove_service(String service);
	private static native int get_error_code();

	private Session(String to) {
		this.to = to;
		this.didClose = false;
		Log.d(TAG, "Session identityHashCode:" + System.identityHashCode(this));
	}

	@Override
	protected void finalize() throws Throwable {
		close();
		super.finalize();
	}

	/**
	 * Close a session to friend. All resources include streams, channels, portforwardings
	 * associated with current session will be destroyed.
	 */
	public synchronized void close() {
		if (!didClose) {

			Log.d(TAG, "Closing session with " + to + " ...");
			Log.d(TAG, "close identityHashCode:" + System.identityHashCode(this));

			session_close();
			didClose = true;

			Log.d(TAG, "Session with " + to + " closed");
		}
	}

	/**
	 * Get remote peer id.
	 *
	 * @return
	 *      The remote peer userid.
	 */
	public String getPeer() {
		return to;
	}

	/**
	 * Send session request to the friend.
	 *
	 * @param
	 *      handler     A handler to the SessionRequestCompleteHandler to receive the
	 *                  session response
	 *
	 * @throws
	 *      IllegalArgumentException
	 *      CarrierException
	 */
	public void request(SessionRequestCompleteHandler handler)
			throws CarrierException {

		if (handler == null)
			throw new IllegalArgumentException();

		if (!native_request(handler))
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Initiate session request to " + to);
	}

	/**
	 * Reply the session request from friend.
	 *
	 * This function will send a session response to friend.
	 *
	 * @param
	 *      status      The status code of the response. 0 is success, otherwise is error
	 * @param
	 *      reason      The error message if status is error, or null if success
	 *
	 * @throws
	 *      IllegalArgumentException
	 *      CarrierException
	 */
	public void replyRequest(int status, String reason) throws CarrierException {

		if (status != 0 && (reason == null || reason.length() == 0))
			throw new IllegalArgumentException();

		if (!native_reply_request(status, reason))
			throw CarrierException.fromErrorCode(get_error_code());

		if (status == 0) {
			Log.d(TAG, "Confirmed session request from " + to);
		} else {
			Log.d(TAG, "Refused session request from " + to + " with reason " + reason);
		}
	}

	/**
	 * Begin to start a session.
	 *
	 * All streams in current session will try to connect with remote friend,
	 * The stream status will update to application by stream's StreamHandler.
	 *
	 * @param
	 *      sdp         The remote user's SDP.  Reference: https://tools.ietf.org/html/rfc4566
	 *
	 * @throws
	 *      IllegalArgumentException
	 *      CarrierException
	 */
	public void start(String sdp) throws CarrierException {

		if (sdp == null || sdp.length() == 0)
			throw new IllegalArgumentException();

		if (!native_start(sdp))
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Session to " + to + " started");
	}

	/**
	 * Add a new stream to session.
	 *
	 * Carrier stream supports several underlying transport mechanisms:
	 *
	 *   - Plain/encrypted UDP data gram protocol
	 *   - Plain/encrypted TCP like reliable stream protocol
	 *   - Multiplexing over UDP
	 *   - Multiplexing over TCP like reliable protocol
	 *
	 *  Application can use options to specify the new stream mode.
	 *  Multiplexing over UDP can not provide reliable transport.
	 *
	 * @param
	 *      type        The stream type defined in StreamType
	 * @param
	 *      options     The stream mode options. options are constructed by a
	 *                  bitwise-inclusive OR of flags
	 * @param
	 *      handler     The Application defined inerface to StreamHandler
	 *
	 * @return
	 *      The new added carrier stream
	 *
	 * @throws
	 *      IllegalArgumentException
	 *      CarrierException
	 */
	public Stream addStream(StreamType type, int options, StreamHandler handler)
			throws CarrierException {

		if (handler == null)
			throw new IllegalArgumentException();

		Log.d(TAG, String.format("Attempt to add stream (type:%s, options:%d)", type, options));

		Stream stream = add_stream(type, options, handler);
		if (stream == null)
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, String.format("Stream %d with %s type created", stream.getStreamId(), type.name()));

		return stream;
	}

	/**
	 * Remove a stream from session.
	 *
	 * @param
	 *      stream      The Stream to be removed
	 *
	 * @throws
	 *      IllegalArgumentException
	 *      CarrierException
	 */
	public void removeStream(Stream stream) throws CarrierException {
		if (stream == null)
			throw new IllegalArgumentException();

		if (!remove_stream(stream.getStreamId(), stream))
			throw CarrierException.fromErrorCode(get_error_code());


		Log.d(TAG, "Stream " + stream.getStreamId() + " was removed from session");
	}

	/**
	 * Add a new portforwarding service to session.
	 *
	 * The registered services can be used by remote peer in portforwarding
	 * request.
	 *
	 * @param
	 *      service     The new service name, should be unique in session scope
	 * @param
	 *      protocol    The protocol of the service
	 * @param
	 *      host        The host name or ip of the service
	 * @param
	 *      port        The port of the service
	 *
	 * @throws
	 *      IllegalArgumentException
	 *      CarrierException
	 */
	public void addService(String service, PortForwardingProtocol protocol, String host, String port)
		throws CarrierException {

		if (service == null || service.length() == 0 ||  host == null || host.length() == 0 ||
				port == null || port.length() == 0)
			throw new IllegalArgumentException();

		if (!add_service(service, protocol, host, port))
			throw CarrierException.fromErrorCode(get_error_code());


		Log.d(TAG, "Service " + service + " added to session");
	}

	/**
	 * Remove a portforwarding server to session.
	 *
	 * This function has not effect on existing portforwarings.
	 *
	 * @param
	 *      service     The service name.
	 */
	public void removeService(String service) {
		if (service == null || service.length() == 0)
			return;

		remove_service(service);

		Log.d(TAG, "Service " + service + "was removed from session");
	}
}
