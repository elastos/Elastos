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

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.Random;

import org.elastos.did.backend.IDChainRequest;
import org.elastos.did.backend.IDTransactionInfo;
import org.elastos.did.backend.ResolveResult;
import org.elastos.did.backend.ResolverCache;
import org.elastos.did.exception.DIDBackendException;
import org.elastos.did.exception.DIDDeactivatedException;
import org.elastos.did.exception.DIDExpiredException;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.DIDTransactionException;
import org.elastos.did.exception.InvalidKeyException;
import org.elastos.did.exception.MalformedResolveResultException;
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

	class TransactionResult {
		private String txid;
		private int status;
		private String message;
		private boolean filled;

		public TransactionResult() {
		}

		public void update(String txid, int status, String message) {
			this.txid = txid;
			this.status = status;
			this.message = message;
			this.filled = true;

			synchronized(this) {
				notifyAll();
			}
		}

		public void update(String txid) {
			update(txid, 0, null);
		}

		public String getTxid() {
			return txid;
		}

		public int getStatus() {
			return status;
		}

		public String getMessage() {
			return message;
		}

		public boolean isEmpty() {
			return !filled;
		}
	}

	private DIDBackend(DIDAdapter adapter, File cacheDir) {
		this.adapter = adapter;
		this.random = new Random();
		this.ttl = DEFAULT_TTL;
		ResolverCache.setCacheDir(cacheDir);
	}

	/*
	 * Recommendation for cache dir:
	 * - Laptop/standard Java
	 *   System.getProperty("user.home") + "/.cache.did.elastos"
	 * - Android Java
	 *   Context.getFilesDir() + "/.cache.did.elastos"
	 */
	public static void initialize(DIDAdapter adapter, File cacheDir) {
		if (instance == null)
			instance = new DIDBackend(adapter, cacheDir);
	}

	public static void initialize(DIDAdapter adapter, String cacheDir) {
		initialize(adapter, new File(cacheDir));
	}

	public static DIDBackend getInstance() throws DIDBackendException {
		if (instance == null)
			throw new DIDBackendException("DID backend not initialized.");

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
			throw new MalformedResolveResultException("Mismatched resolve result with request.");

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

	protected DIDDocument resolve(DID did) throws DIDResolveException {
		return resolve(did, false);
	}

	private String createTransaction(String payload, String memo, int confirms)
			throws DIDTransactionException {
		TransactionResult tr = new TransactionResult();

		boolean success = adapter.createIdTransaction(payload, memo, confirms,
				(txid, status, message) -> {
					tr.update(txid, status, message);
				});

		if (!success)
			throw new DIDTransactionException("Create transaction failed, unknown error.");

		synchronized(tr) {
			if (tr.isEmpty()) {
				try {
					tr.wait();
				} catch (InterruptedException e) {
					throw new DIDTransactionException(e);
				}
			}
		}

		if (tr.getStatus() != 0)
			throw new DIDTransactionException(
					"Create transaction failed(" + tr.getStatus() + "): "
					+ tr.getMessage());

		return tr.getTxid();
	}

	protected String create(DIDDocument doc, DIDURL signKey, String storepass)
			throws DIDTransactionException, DIDStoreException, InvalidKeyException {
		return create(doc, 0, signKey, storepass);
	}

	protected String create(DIDDocument doc, int confirms, DIDURL signKey,
			String storepass)
			throws DIDTransactionException, DIDStoreException, InvalidKeyException {
		IDChainRequest request = IDChainRequest.create(doc, signKey, storepass);
		String json = request.toJson(true);
		return createTransaction(json, null, confirms);
	}

	protected String update(DIDDocument doc, String previousTxid,
			DIDURL signKey, String storepass)
			throws DIDTransactionException, DIDStoreException, InvalidKeyException {
		return update(doc, previousTxid, 0, signKey, storepass);
	}

	protected String update(DIDDocument doc, String previousTxid, int confirms,
			DIDURL signKey, String storepass)
			throws DIDTransactionException, DIDStoreException, InvalidKeyException {
		IDChainRequest request = IDChainRequest.update(doc, previousTxid,
				signKey, storepass);
		String json = request.toJson(true);
		return createTransaction(json, null, confirms);
	}

	protected String deactivate(DIDDocument doc, DIDURL signKey, String storepass)
			throws DIDTransactionException, DIDStoreException, InvalidKeyException {
		return deactivate(doc, 0, signKey, storepass);
	}

	protected String deactivate(DIDDocument doc, int confirms,
			DIDURL signKey, String storepass)
			throws DIDTransactionException, DIDStoreException, InvalidKeyException {
		IDChainRequest request = IDChainRequest.deactivate(doc, signKey, storepass);
		String json = request.toJson(true);
		return createTransaction(json, null, confirms);
	}

	protected String deactivate(DID target, DIDURL targetSignKey,
			DIDDocument doc, DIDURL signKey, String storepass)
			throws DIDTransactionException, DIDStoreException, InvalidKeyException {
		return deactivate(target, targetSignKey, doc, 0, signKey, storepass);
	}

	protected String deactivate(DID target, DIDURL targetSignKey,
			DIDDocument doc, int confirms, DIDURL signKey, String storepass)
			throws DIDTransactionException, DIDStoreException, InvalidKeyException {
		IDChainRequest request = IDChainRequest.deactivate(target,
				targetSignKey, doc, signKey, storepass);
		String json = request.toJson(true);
		return createTransaction(json, null, confirms);
	}
}
