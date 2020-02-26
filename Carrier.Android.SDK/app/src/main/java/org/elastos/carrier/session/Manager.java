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

import org.elastos.carrier.Carrier;
import org.elastos.carrier.Log;
import org.elastos.carrier.exceptions.CarrierException;

/**
 * The class representing Carrier session manager.
 */
public class Manager {
	private static final String TAG = "CarrierManager";

	private Carrier carrier;
	private boolean didCleanup;
	private long nativeCookie = 0;  // store the native (JNI-layered) context

	// jni native methods.
	private native boolean native_init(Carrier carrier, ManagerHandler handler);
	private native void native_cleanup(Carrier carrier);
	private static native Session create_session(Carrier carrier, String to);
	private static native int get_error_code();

	/**
	 * Create a carrier session manager instance.
	 *
	 * This function is convenient way to get instance without interest to session request
	 * from friends.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 *
	 * @return
	 * 		A carrier session manager
	 *
	 * @throws
	 * 		CarrierException
	 */
	public static Manager createInstance(Carrier carrier) throws CarrierException {
		return createInstance(carrier, null);
	}

	/**
	 * Create a session manager instance.
	 *
	 * @param
	 * 		carrier		Carrier node instance
	 * @param
	 *      handler     The interface handler for carrier session manager to comply with
	 *
	 * @return
	 * 		A carrier session manager
	 *
	 * @throws
	 * 		CarrierException
	 */
	public static Manager createInstance(Carrier carrier, ManagerHandler handler)
			throws CarrierException {
		if (carrier == null)
			throw new IllegalArgumentException();

		Log.d(TAG, "Attempt to create carrier session manager instance ...");

		Manager tmp = new Manager(carrier);

		if (!tmp.native_init(carrier, handler))
			throw CarrierException.fromErrorCode(get_error_code());

		Log.d(TAG, "Carrier session manager instance created");

		return tmp;
	}

	private Manager(Carrier carrier) {
		this.carrier = carrier;
		this.didCleanup = false;
	}

	@Override
	protected void finalize() throws Throwable {
		cleanup();
		super.finalize();
	}

	/**
	 * Clean up carrier session manager.
	 */
	public synchronized void cleanup() {
		if (!didCleanup) {
			native_cleanup(carrier);
			carrier = null;
			didCleanup = true;
		}
	}

	/**
	 * Create a new session to a friend.
	 *
	 * The session object represent a conversation handle to a friend.
	 *
	 * @param
	 *      to          The target id(userid or userid@nodeid).
	 *
	 * @return
	 *      The new Session object
	 *
	 * @throws
	 * 		IllegalArgumentException
	 * 		CarrierException
	 */
	public Session newSession(String to) throws CarrierException {

		if (to == null)
			throw new IllegalArgumentException();

		Log.d(TAG, "Attempt to create a new session to:" + to);

		Session session = create_session(carrier, to);
		if (session == null) {
			throw CarrierException.fromErrorCode(get_error_code());
		}

		Log.d(TAG, "Session to " + to +  " created");

		return session;
	}
}
