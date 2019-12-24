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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

import org.elastos.did.DIDAdapter;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDResolveException;

import com.fasterxml.jackson.core.JsonEncoding;
import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;

public class SPVAdapter implements DIDAdapter {
	private URL url;

	private String walletDir;
	private String walletId;
	private String network;

	private long handle;
	private PasswordCallback passwordCallback;

	static {
		System.loadLibrary("spvadapterjni");
	}

	public interface PasswordCallback {
		public String getPassword(String walletDir, String walletId);
	}

	public SPVAdapter(String walletDir, String walletId, String network,
			String resolver, PasswordCallback passwordCallback)
			throws DIDResolveException {
		if (resolver == null || resolver.isEmpty())
			throw new IllegalArgumentException();

		try {
			this.url = new URL(resolver);
		} catch (MalformedURLException e) {
			throw new DIDResolveException(e);
		}

		handle = create(walletDir, walletId, network, "");
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
	public InputStream resolve(String requestId, String did, boolean all)
			throws DIDResolveException {
		try {
			HttpURLConnection connection = (HttpURLConnection)url.openConnection();
			connection.setRequestMethod("POST");
			connection.setRequestProperty("User-Agent",
					"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.11 (KHTML, like Gecko) Chrome/23.0.1271.95 Safari/537.11");
			connection.setRequestProperty("Content-Type", "application/json");
			connection.setRequestProperty("Accept", "application/json");
			connection.setDoOutput(true);
			connection.connect();

			OutputStream os = connection.getOutputStream();
			JsonFactory factory = new JsonFactory();
			JsonGenerator generator = factory.createGenerator(os, JsonEncoding.UTF8);
			generator.writeStartObject();
			generator.writeStringField("id", requestId);
			generator.writeStringField("method", "resolvedid");
			generator.writeFieldName("params");
			generator.writeStartObject();
			generator.writeStringField("did", did);
			generator.writeBooleanField("all", all);
			generator.writeEndObject();
			generator.writeEndObject();
			generator.close();
			os.close();

			int code = connection.getResponseCode();
			if (code != 200)
				return null;

			return connection.getInputStream();
		} catch (IOException e) {
			throw new DIDResolveException("Network error.", e);
		}
	}
}
