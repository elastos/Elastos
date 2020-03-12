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
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.nio.charset.Charset;
import java.util.Random;

import org.elastos.did.backend.IDChainRequest;
import org.elastos.did.backend.IDChainTransaction;
import org.elastos.did.backend.ResolveResult;
import org.elastos.did.backend.ResolverCache;
import org.elastos.did.exception.DIDDeactivatedException;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.DIDTransactionException;
import org.elastos.did.exception.InvalidKeyException;
import org.elastos.did.exception.MalformedResolveResultException;
import org.elastos.did.exception.NetworkException;
import org.elastos.did.meta.DIDMeta;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.core.JsonEncoding;
import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;
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
	private static DIDResolver resolver;

	private static Random random = new Random();
	private static long ttl = DEFAULT_TTL; // milliseconds

	private DIDAdapter adapter;

	private static final Logger log = LoggerFactory.getLogger(DIDBackend.class);

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

		@Override
		public String toString() {
			StringBuilder sb = new StringBuilder(256);

			sb.append("txid: ").append(txid).append(", ")
				.append("status: ").append(status);

			if (status != 0)
				sb.append(", ").append("message: ").append(message);

			return sb.toString();
		}
	}

	static class DefaultResolver implements DIDResolver {
		private URL url;

		private static final Logger log = LoggerFactory.getLogger(DefaultResolver.class);

		public DefaultResolver(String resolver) throws DIDResolveException {
			if (resolver == null || resolver.isEmpty())
				throw new IllegalArgumentException();

			try {
				this.url = new URL(resolver);
			} catch (MalformedURLException e) {
				throw new DIDResolveException(e);
			}
		}

		@Override
		public InputStream resolve(String requestId, String did, boolean all)
				throws DIDResolveException {
			try {
				log.debug("Resolving {}...", did.toString());

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
				if (code != 200) {
					log.error("Resolve {} error, status: {}, message: {}",
							did.toString(), code, connection.getResponseMessage());
					throw new DIDResolveException("HTTP error with status: " + code);
				}

				return connection.getInputStream();
			} catch (IOException e) {
				log.error("Resovle " + did + " error", e);
				throw new NetworkException("Network error.", e);
			}
		}
	}

	private DIDBackend(DIDAdapter adapter) {
		this.adapter = adapter;
	}

	/*
	 * Recommendation for cache dir:
	 * - Laptop/standard Java
	 *   System.getProperty("user.home") + "/.cache.did.elastos"
	 * - Android Java
	 *   Context.getFilesDir() + "/.cache.did.elastos"
	 */
	public static void initialize(String resolverURL, File cacheDir)
			throws DIDResolveException {
		if (resolverURL == null || resolverURL.isEmpty() || cacheDir == null)
			throw new IllegalArgumentException();

		initialize(new DefaultResolver(resolverURL), cacheDir);
	}

	public static void initialize(String resolverURL, String cacheDir)
			throws DIDResolveException {
		if (resolverURL == null || resolverURL.isEmpty() ||
				cacheDir == null || cacheDir.isEmpty())
			throw new IllegalArgumentException();

		initialize(resolverURL, new File(cacheDir));
	}

	public static void initialize(DIDResolver resolver, File cacheDir) {
		if (resolver == null || cacheDir == null)
			throw new IllegalArgumentException();

		DIDBackend.resolver = resolver;
		ResolverCache.setCacheDir(cacheDir);
	}

	public static void initialize(DIDResolver resolver, String cacheDir) {
		if (resolver == null || cacheDir == null || cacheDir.isEmpty())
			throw new IllegalArgumentException();

		initialize(resolver, new File(cacheDir));
	}

	protected static DIDBackend getInstance(DIDAdapter adapter) {
		return new DIDBackend(adapter);
	}

	// Time to live in minutes
	public static void setTTL(long ttl) {
		ttl = ttl > 0 ? (ttl * 60 * 1000) : 0;
	}

	public static long getTTL() {
		return ttl != 0 ? (ttl / 60 / 1000) : 0;
	}

	private static String generateRequestId() {
		StringBuffer sb = new StringBuffer();

		while(sb.length() < 16)
			sb.append(Integer.toHexString(random.nextInt()));

		return sb.toString();
	}

	private static ResolveResult resolveFromBackend(DID did, boolean all)
			throws DIDResolveException {
		String requestId = generateRequestId();

		if (resolver == null)
			throw new DIDResolveException("DID resolver not initialized.");

		InputStream is = resolver.resolve(requestId, did.toString(), all);

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

	protected static DIDHistory resolveHistory(DID did) throws DIDResolveException {
		log.info("Resolving {}...", did.toString());

		ResolveResult rr = resolveFromBackend(did, true);
		if (rr.getStatus() == ResolveResult.STATUS_NOT_FOUND)
			return null;

		return rr;
	}

	protected static DIDDocument resolve(DID did, boolean force)
			throws DIDResolveException {
		log.info("Resolving {}...", did.toString());

		ResolveResult rr = null;
		if (!force) {
			rr = ResolverCache.load(did, ttl);
			log.debug("Try load {} from resolver cache: {}.",
					did.toString(), rr == null ? "non" : "matched");
		}

		if (rr == null)
			rr = resolveFromBackend(did, false);

		switch (rr.getStatus()) {
		// When DID expired, we should also return the document.
		// case ResolveResult.STATUS_EXPIRED:
		// 	throw new DIDExpiredException();

		case ResolveResult.STATUS_DEACTIVATED:
			throw new DIDDeactivatedException();

		case ResolveResult.STATUS_NOT_FOUND:
			return null;

		default:
			IDChainTransaction ti = rr.getTransactionInfo(0);
			DIDDocument doc = ti.getRequest().getDocument();
			DIDMeta meta = new DIDMeta();
			meta.setTransactionId(ti.getTransactionId());
			meta.setSignature(doc.getProof().getSignature());
			meta.setUpdated(ti.getTimestamp());
			doc.setMeta(meta);
			return doc;
		}
	}

	protected static DIDDocument resolve(DID did) throws DIDResolveException {
		return resolve(did, false);
	}

	protected DIDAdapter getAdapter() {
		return adapter;
	}

	private String createTransaction(String payload, String memo, int confirms)
			throws DIDTransactionException {
		TransactionResult tr = new TransactionResult();

		log.info("Create ID transaction...");
		log.trace("Transaction paload: '{}', memo: {}, confirms: {}",
				payload, memo, confirms);

		adapter.createIdTransaction(payload, memo, confirms,
				(txid, status, message) -> {
					tr.update(txid, status, message);
				});

		synchronized(tr) {
			if (tr.isEmpty()) {
				try {
					tr.wait();
				} catch (InterruptedException e) {
					throw new DIDTransactionException(e);
				}
			}
		}

		log.info("ID transaction complete. {}", tr.toString());

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
