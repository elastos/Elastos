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

package org.elastos.did.backend;

import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDURL;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.util.Base64;
import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class IDChainRequest {
	public static final String CURRENT_SPECIFICATION = "elastos/did/1.0";

	private static final String HEADER = "header";
	private static final String SPECIFICATION = "specification";
	private static final String OPERATION = "operation";
	private static final String PREVIOUS_TXID = "previousTxid";
	private static final String PAYLOAD = "payload";
	private static final String PROOF = "proof";
	private static final String KEY_TYPE = "type";
	private static final String VERIFICATION_METHOD = "verificationMethod";
	private static final String SIGNATURE = "signature";

	private static final String DEFAULT_PUBLICKEY_TYPE = Constants.DEFAULT_PUBLICKEY_TYPE;

	public enum Operation {
		CREATE, UPDATE, DEACTIVATE;

		@Override
		public String toString() {
			return name().toLowerCase();
		}
	}

	// header
	private String specification;
	private Operation operation;
	private String previousTxid;

	// payload
	private DID did;
	private DIDDocument doc;
	private String payload;

	// signature
	private String keyType;
	private DIDURL signKey;
	private String signature;

	private IDChainRequest(Operation op) {
		this.specification = CURRENT_SPECIFICATION;
		this.operation = op;
	}

	public static IDChainRequest create(DIDDocument doc, DIDURL signKey,
			String storepass) throws DIDStoreException {
		IDChainRequest request = new IDChainRequest(Operation.CREATE);
		request.setPayload(doc);
		request.seal(signKey, storepass);

		return request;
	}

	public static IDChainRequest update(DIDDocument doc, String previousTxid,
			DIDURL signKey, String storepass) throws DIDStoreException {
		IDChainRequest request = new IDChainRequest(Operation.UPDATE);
		request.setPreviousTxid(previousTxid);
		request.setPayload(doc);
		request.seal(signKey, storepass);

		return request;
	}

	public static IDChainRequest deactivate(DID did, DIDURL signKey,
			String storepass) throws DIDStoreException {
		IDChainRequest request = new IDChainRequest(Operation.DEACTIVATE);
		request.setPayload(did);
		request.seal(signKey, storepass);

		return request;
	}

	public Operation getOperation() {
		return operation;
	}

	public String getPreviousTxid() {
		return previousTxid;
	}

	private void setPreviousTxid(String previousTxid) {
		this.previousTxid = previousTxid != null ? previousTxid : "";
	}

	public String getPayload() {
		return payload;
	}

	public DID getDid() {
		return did;
	}

	public DIDDocument getDocument() {
		return doc;
	}

	private void setPayload(DID did) {
		this.did = did;
		this.doc = null;
		this.payload = did.toString();
	}

	private void setPayload(DIDDocument doc) {
		this.did = doc.getSubject();
		this.doc = doc;

		String json = doc.toString(false);

		payload = Base64.encodeToString(json.getBytes(),
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
	}

	private void setPayload(String payload) throws DIDResolveException {
		try {
			if (operation != Operation.DEACTIVATE) {
				String json = new String(Base64.decode(payload,
						Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP));

				doc = DIDDocument.fromJson(json);
				did = doc.getSubject();
			} else {
				did = new DID(payload);
				doc = null;
			}
		} catch (DIDException e) {
			throw new DIDResolveException("Parse payload error.", e);
		}

		this.payload = payload;
	}

	private void setProof(String keyType, DIDURL signKey, String signature) {
		this.keyType = keyType;
		this.signKey = signKey;
		this.signature = signature;
	}

	private void seal(DIDURL signKey, String storepass) throws DIDStoreException {
		String prevtxid = operation == Operation.UPDATE ? previousTxid : "";

		byte[][] inputs = new byte[][] {
			specification.getBytes(),
			operation.toString().getBytes(),
			prevtxid.getBytes(),
			payload.getBytes()
		};

		this.signature = doc.sign(signKey, storepass, inputs);
		this.signKey = signKey;
		this.keyType = DEFAULT_PUBLICKEY_TYPE;
	}

	public boolean isValid() throws DIDException {
		DIDDocument doc;
		if (operation != Operation.DEACTIVATE) {
			doc = this.doc;
			if (!doc.isAuthenticationKey(signKey))
				return false;
		} else {
			doc = did.resolve();
			if (!doc.isAuthenticationKey(signKey) &&
					!doc.isAuthorizationKey(signKey))
				return false;
		}

		String prevtxid = operation == Operation.UPDATE ? previousTxid : "";

		byte[][] inputs = new byte[][] {
			specification.getBytes(),
			operation.toString().getBytes(),
			prevtxid.getBytes(),
			payload.getBytes()
		};

		return doc.verify(signKey, signature, inputs);
	}

	public void toJson(JsonGenerator generator, boolean normalized) throws IOException {
		generator.writeStartObject();

		// header
		generator.writeFieldName(HEADER);
		generator.writeStartObject();

		generator.writeFieldName(SPECIFICATION);
		generator.writeString(specification);

		generator.writeFieldName(OPERATION);
		generator.writeString(operation.toString());

		if (operation == Operation.UPDATE) {
			generator.writeFieldName(PREVIOUS_TXID);
			generator.writeString(previousTxid);
		}

		generator.writeEndObject();

		// payload
		generator.writeFieldName(PAYLOAD);
		generator.writeString(payload);

		// signature
		generator.writeFieldName(PROOF);
		generator.writeStartObject();

		String keyId;

		if (normalized) {
			generator.writeFieldName(KEY_TYPE);
			generator.writeString(keyType);

			keyId = signKey.toString();
		} else {
			keyId = "#" + signKey.getFragment();
		}

		generator.writeFieldName(VERIFICATION_METHOD);
		generator.writeString(keyId);


		generator.writeFieldName(SIGNATURE);
		generator.writeString(signature);

		generator.writeEndObject();
		generator.writeEndObject();
	}

	public void toJson(Writer out, boolean normalized) throws IOException {
		JsonFactory factory = new JsonFactory();
		JsonGenerator generator = factory.createGenerator(out);
		toJson(generator, normalized);
		generator.close();
	}

	public String toJson(boolean normalized) {
		Writer out = new StringWriter(2048);

		try {
			toJson(out, normalized);
		} catch (IOException ignore) {
		}

		return out.toString();
	}

	public static IDChainRequest fromJson(JsonNode node)
			throws DIDResolveException {
		Class<DIDResolveException> clazz = DIDResolveException.class;

		JsonNode header = node.get(HEADER);
		if (header == null)
			throw new DIDResolveException("Missing header.");

		String spec = JsonHelper.getString(header, SPECIFICATION, false,
				null, SPECIFICATION, clazz);
		if (!spec.equals(CURRENT_SPECIFICATION))
			throw new DIDResolveException("Unknown DID specifiction.");

		String opstr = JsonHelper.getString(header, OPERATION, false,
				null, OPERATION, clazz);
		Operation op = Operation.valueOf(opstr.toUpperCase());

		IDChainRequest request = new IDChainRequest(op);

		if (op == Operation.UPDATE) {
			String txid = JsonHelper.getString(header, PREVIOUS_TXID, false,
					null, PREVIOUS_TXID, clazz);
			request.setPreviousTxid(txid);
		}

		String payload = JsonHelper.getString(node, PAYLOAD, false,
				null, PAYLOAD, clazz);
		request.setPayload(payload);

		JsonNode proof = node.get(PROOF);
		if (proof == null)
			throw new DIDResolveException("Missing proof.");

		String keyType = JsonHelper.getString(proof, KEY_TYPE, true,
				DEFAULT_PUBLICKEY_TYPE, KEY_TYPE, clazz);
		if (!keyType.equals(DEFAULT_PUBLICKEY_TYPE))
			throw new DIDResolveException("Unknown signature key type.");

		DIDURL signKey = JsonHelper.getDidUrl(proof, VERIFICATION_METHOD,
				request.getDid(), VERIFICATION_METHOD, clazz);
		//if (doc.getAuthenticationKey(signKey) == null)
		//	throw new DIDResolveException("Unknown signature key.");

		String sig = JsonHelper.getString(proof, SIGNATURE, false,
				null, SIGNATURE, clazz);

		request.setProof(keyType, signKey, sig);
		return request;
	}

	public static IDChainRequest fromJson(String json)
			throws DIDResolveException {
		if (json == null || json.isEmpty())
			throw new IllegalArgumentException();

		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(json);
			return fromJson(node);
		} catch (IOException e) {
			throw new DIDResolveException("Parse resolved result error.", e);
		}
	}

}
