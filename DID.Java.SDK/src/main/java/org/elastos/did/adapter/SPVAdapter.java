/*
 * Copyright (c) 2019 Elastos Foundation
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

package org.elastos.did.adapter;

import org.elastos.did.DIDAdapter;
import org.elastos.did.exception.DIDException;

public class SPVAdapter implements DIDAdapter {
	static {
		System.loadLibrary("spvadapterjni");
	}

	private String walletDir;
	private String walletId;
	private String network;

	private long handle;
	private PasswordCallback passwordCallback;

	public interface PasswordCallback {
		public String getPassword(String walletDir, String walletId);
	}

	public SPVAdapter(String walletDir, String walletId, String network,
			String resolver, PasswordCallback passwordCallback) {
		handle = create(walletDir, walletId, network, resolver);
		this.walletDir = walletDir;
		this.walletId = walletId;
		this.network = network;
		this.passwordCallback = passwordCallback;
	}

	public void destroy() {
		destroy(handle);
		handle = 0;
	}

	private final static native long create(String walletDir, String walletId,
			String network, String resolver);

	private final static native void destroy(long handle);

	private final static native boolean isAvailable(long handle);

	private final static native int createIdTransaction(long handle,
			String payload, String memo, String password);

	private final static native String resolve(long handle,
			String did, boolean all);

	public boolean isAvailable() {
		return isAvailable(handle);
	}

	@Override
	public boolean createIdTransaction(String payload, String memo)
			throws DIDException {
		String password = passwordCallback.getPassword(walletDir, walletId);
		if (password == null)
			return false;

		int rc = createIdTransaction(handle, payload, memo, password);
		return rc == 0;
	}

	@Override
	public String resolve(String did, boolean all) {
		return resolve(handle, did, all);
	}
}
