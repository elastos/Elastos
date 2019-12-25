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

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.Random;

import org.elastos.did.backend.IDChainRequest;
import org.elastos.did.backend.IDTransactionInfo;
import org.elastos.did.backend.ResolveResult;
import org.elastos.did.backend.ResolverCache;
import org.elastos.did.exception.DIDDeactivatedException;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDExpiredException;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.meta.DIDMeta;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class DIDBackend {
	private final static String ID = "id";
	private final static String RESULT = "result";
	private final static String ERROR = "error";
	private final static String ERROR_CODE = "code";
	private final static String ERROR_MESSAGE = "message";

	private static final long DEFAULT_TTL = 24 * 60 * 60 * 1000;
	private static final Charset utf8 = Charset.forName("UTF-8");
	private static DIDBackend instance;

	private Random random;
	private long ttl; // milliseconds
	private DIDAdapter adapter;

	private DIDBackend(DIDAdapter adapter) {
		this.adapter = adapter;
		this.random = new Random();
		this.ttl = DEFAULT_TTL;
	}

	public static void initialize(DIDAdapter adapter) {
		instance = new DIDBackend(adapter);
	}

	public static DIDBackend getInstance() throws DIDException {
		if (instance == null)
			throw new DIDException("DID backend not initialized.");

		return instance;
	}

	public DIDAdapter getAdapter() {
		return adapter;
	}

	// Time to live in minutes
	public void setTTL(long ttl) {
		this.ttl = ttl > 0 ? (ttl * 60 * 1000) : 0;
	}

	public long getTTL() {
		return ttl != 0 ? (ttl / 60 / 1000) : 0;
	}

	private String generateRequestId() {
		StringBuffer sb = new StringBuffer();

		while(sb.length() < 16)
			sb.append(Integer.toHexString(random.nextInt()));

		return sb.toString();
	}

	private ResolveResult resolveFromBackend(DID did)
			throws DIDResolveException {
		String requestId = generateRequestId();

		InputStream is = adapter.resolve(requestId, did.toString(), false);
		if (is == null)
			throw new DIDResolveException("Unknown error.");

		ObjectMapper mapper = new ObjectMapper();
		JsonNode node = null;

		try {
			node = mapper.readTree(new InputStreamReader(is, utf8));
		} catch (IOException e) {
			throw new DIDResolveException("Parse resolved json error.", e);
		}

		// Check response id, should equals requestId
		JsonNode id = node.get(ID);
		if (id == null || id.textValue() == null ||
				!id.textValue().equals(requestId))
			throw new DIDResolveException("Missmatched resolve result with request.");

		JsonNode result = node.get(RESULT);
		if (result == null || result.isNull()) {
			JsonNode error = node.get(ERROR);
			throw new DIDResolveException("Resolve DID error("
					+ error.get(ERROR_CODE).longValue() + "): "
					+ error.get(ERROR_MESSAGE).textValue());
		}

		ResolveResult rr = ResolveResult.fromJson(result);

		if (rr.getStatus() != ResolveResult.STATUS_NOT_FOUND) {
			try {
				ResolverCache.store(rr);
			} catch (IOException e) {
				System.out.println("!!! Cache resolved resolved result error: "
						+ e.getMessage());
			}
		}

		return rr;
	}

	protected DIDDocument resolve(DID did, boolean force)
			throws DIDResolveException {
		if (did == null)
			throw new IllegalArgumentException();

		ResolveResult rr = null;
		if (!force)
			rr = ResolverCache.load(did, ttl);

		if (rr == null)
			rr = resolveFromBackend(did);

		switch (rr.getStatus()) {
		case ResolveResult.STATUS_EXPIRED:
			throw new DIDExpiredException();

		case ResolveResult.STATUS_DEACTIVATED:
			throw new DIDDeactivatedException();

		case ResolveResult.STATUS_NOT_FOUND:
			return null;

		default:
			IDTransactionInfo ti = rr.getTransactionInfo(0);
			DIDDocument doc = ti.getRequest().getDocument();
			DIDMeta meta = new DIDMeta();
			meta.setTransactionId(ti.getTransactionId());
			meta.setUpdated(ti.getTimestamp());
			doc.setMeta(meta);
			return doc;
		}
	}

	public DIDDocument resolve(DID did) throws DIDResolveException {
		return resolve(did, false);
	}

	protected boolean create(DIDDocument doc, DIDURL signKey, String storepass)
			throws DIDStoreException {
		IDChainRequest request = IDChainRequest.create(doc, signKey, storepass);
		String json = request.toJson(true);

		try {
			return adapter.createIdTransaction(json, null);
		} catch (DIDException e) {
			throw new DIDStoreException("Create ID transaction error.", e);
		}
	}

	protected boolean update(DIDDocument doc, String previousTxid,
			DIDURL signKey, String storepass) throws DIDStoreException {
		IDChainRequest request = IDChainRequest.update(doc,
				previousTxid, signKey, storepass);
		String json = request.toJson(true);

		try {
			return adapter.createIdTransaction(json, null);
		} catch (DIDException e) {
			throw new DIDStoreException("Create ID transaction error.", e);
		}
	}

	protected boolean deactivate(DID did, DIDURL signKey, String storepass)
			throws DIDStoreException {
		IDChainRequest request = IDChainRequest.deactivate(did, signKey, storepass);
		String json = request.toJson(true);

		try {
			return adapter.createIdTransaction(json, null);
		} catch (DIDException e) {
			throw new DIDStoreException("Create ID transaction error.", e);
		}
	}
}
