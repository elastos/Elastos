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

package org.elastos.did;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;

import org.elastos.did.exception.DIDBackendException;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.MalformedDIDException;
import org.elastos.did.metadata.DIDMetadataImpl;
import org.elastos.did.parser.DIDURLBaseListener;
import org.elastos.did.parser.DIDURLParser;
import org.elastos.did.parser.ParserHelper;

public class DID implements Comparable<DID> {
	public final static String METHOD = "elastos";

	private String method;
	private String methodSpecificId;

	private DIDMetadataImpl metadata;

	protected DID() {
	}

	protected DID(String method, String methodSpecificId) {
		this.method = method;
		this.methodSpecificId = methodSpecificId;
	}

	public DID(String did) throws MalformedDIDException {
		if (did == null || did.isEmpty())
			throw new IllegalArgumentException();

		try {
			ParserHelper.parse(did, true, new Listener());
		} catch(IllegalArgumentException e) {
			throw new MalformedDIDException(e.getMessage());
		}
	}

	public String getMethod() {
		return method;
	}

	protected void setMethod(String method) {
		this.method = method;
	}

	public String getMethodSpecificId() {
		return methodSpecificId;
	}

	protected void setMethodSpecificId(String methodSpecificId) {
		this.methodSpecificId = methodSpecificId;
	}

	protected void setMetadata(DIDMetadataImpl metadata) {
		this.metadata = metadata;
	}

	public DIDMetadata getMetadata() {
		if (metadata == null)
			metadata = new DIDMetadataImpl();

		return metadata;
	}

	public void saveMetadata() throws DIDStoreException {
		if (metadata != null && metadata.attachedStore())
			metadata.getStore().storeDidMetadata(this, metadata);
	}

	public boolean isDeactivated() {
		return getMetadata().isDeactivated();
	}

	public DIDDocument resolve(boolean force)
			throws DIDBackendException, DIDResolveException {
		DIDDocument doc = DIDBackend.resolve(this, force);
		if (doc != null)
			setMetadata(doc.getMetadataImpl());

		return doc;
	}

	public DIDDocument resolve()
			throws DIDBackendException, DIDResolveException {
		return resolve(false);
	}

	public CompletableFuture<DIDDocument> resolveAsync(boolean force) {
		CompletableFuture<DIDDocument> future = CompletableFuture.supplyAsync(() -> {
			try {
				return resolve(force);
			} catch (DIDBackendException e) {
				throw new CompletionException(e);
			}
		});

		return future;
	}

	public CompletableFuture<DIDDocument> resolveAsync() {
		return resolveAsync(false);
	}

	public DIDHistory resolveHistory() throws DIDResolveException {
		return DIDBackend.resolveHistory(this);
	}

	public CompletableFuture<DIDHistory> resolveHistoryAsync() {
		CompletableFuture<DIDHistory> future = CompletableFuture.supplyAsync(() -> {
			try {
				return resolveHistory();
			} catch (DIDResolveException e) {
				throw new CompletionException(e);
			}
		});

		return future;
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder(64);
		builder.append("did:")
			.append(method)
			.append(":")
			.append(methodSpecificId);

		return builder.toString();
	}

	@Override
	public int hashCode() {
		return METHOD.hashCode() + methodSpecificId.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == this)
			return true;

		if (obj instanceof DID) {
			DID did = (DID)obj;
			boolean eq = method.equals(did.method);
			return eq ? methodSpecificId.equals(did.methodSpecificId) : eq;
		}

		if (obj instanceof String) {
			String did = (String)obj;
			return toString().equals(did);
		}

		return false;
	}

	@Override
	public int compareTo(DID did) {
		if (did == null)
			throw new IllegalArgumentException();

		int rc = method.compareTo(did.method);
		return rc == 0 ? methodSpecificId.compareTo(did.methodSpecificId) : rc;
	}

	class Listener extends DIDURLBaseListener {
		@Override
		public void exitMethod(DIDURLParser.MethodContext ctx) {
			String method = ctx.getText();
			if (!method.equals(DID.METHOD))
				throw new IllegalArgumentException("Unknown method: " + method);

			setMethod(DID.METHOD);
		}

		@Override
		public void exitMethodSpecificString(
				DIDURLParser.MethodSpecificStringContext ctx) {
			setMethodSpecificId(ctx.getText());
		}
	}
}
