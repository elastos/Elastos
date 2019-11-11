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
import org.elastos.did.DIDException;
import org.elastos.did.DIDStore;
import org.elastos.did.DIDStoreException;
import org.elastos.did.DIDURL;
import org.elastos.did.util.Base64;
import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;

class IDChainRequest {
	public static final String CURRENT_SPECIFICATION = "elastos/did/1.0";

	private static final String HEADER = "header";
	private static final String SPECIFICATION = "specification";
	private static final String OPERATION = "operation";
	private static final String PAYLOAD = "payload";
	private static final String PROOF = Constants.proof;
	private static final String KEY_TYPE = Constants.type;
	private static final String KEY_ID = Constants.verificationMethod;
	private static final String SIGNATURE = Constants.signature;


	public enum Operation {
		CREATE, UPDATE, DEACRIVATE;

		@Override
		public String toString() {
			return name().toLowerCase();
		}
	}

	// header
	private String specification;
	private Operation operation;

	// payload
	private DID did;
	private DIDDocument doc;
	private String payload;

	// signature
	private DIDURL signKey;
	private String keyType;
	private String signature;

	IDChainRequest(Operation op, DID did) {
		if (op != Operation.DEACRIVATE)
			throw new IllegalArgumentException("Operation need a DIDDocument.");

		this.specification = CURRENT_SPECIFICATION;
		this.operation = op;
		this.did = did;
	}

	IDChainRequest(Operation op, DIDDocument doc) {
		this.specification = CURRENT_SPECIFICATION;
		this.operation = op;
		this.did = doc.getSubject();
		this.doc = doc;
	}

	public IDChainRequest sign(DIDURL key, String passphrase)
			throws DIDStoreException {
		// operation
		String op = operation.toString();

		// payload: did or doc
		if (operation == Operation.DEACRIVATE)
			payload = did.toExternalForm();
		else {
			payload = doc.toExternalForm(true);

			payload = Base64.encodeToString(payload.getBytes(),
					Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		}

		byte[][] inputs = new byte[][] {
			specification.getBytes(),
			op.getBytes(),
			payload.getBytes()
		};

		signature = DIDStore.getInstance().sign(did, key, passphrase, inputs);
		signKey = key;
		keyType = Constants.defaultPublicKeyType;

		return this;
	}

	boolean verify() throws DIDException {
		String op = operation.toString();

		byte[][] inputs = new byte[][] {
			specification.getBytes(),
			op.getBytes(),
			payload.getBytes()
		};

		return doc.verify(signKey, signature, inputs);
	}

	DIDDocument getDocument() {
		return doc;
	}

	public void toJson(Writer out, boolean compact) throws IOException {
		JsonFactory factory = new JsonFactory();
		JsonGenerator generator = factory.createGenerator(out);

		generator.writeStartObject();

		// header
		generator.writeFieldName(HEADER);
		generator.writeStartObject();

		generator.writeFieldName(SPECIFICATION);
		generator.writeString(specification);

		generator.writeFieldName(OPERATION);
		generator.writeString(operation.toString());

		generator.writeEndObject();

		// payload
		generator.writeFieldName(PAYLOAD);
		generator.writeString(payload);

		// signature
		generator.writeFieldName(PROOF);
		generator.writeStartObject();

		String keyId;

		if (!compact) {
			generator.writeFieldName(KEY_TYPE);
			generator.writeString(keyType);

			keyId = signKey.toExternalForm();
		} else {
			keyId = "#" + signKey.getFragment();
		}

		generator.writeFieldName(KEY_ID);
		generator.writeString(keyId);


		generator.writeFieldName(SIGNATURE);
		generator.writeString(signature);

		generator.writeEndObject();
		generator.close();
	}

	public String toJson(boolean compact) {
		Writer out = new StringWriter(2048);

		try {
			toJson(out, compact);
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

		// TODO: remove this after IDSidechain fix the wrong field name.
		String op = "Operation";
		op = JsonHelper.getString(header, op, false,
						null, OPERATION, clazz);
		//String op = JsonHelper.getString(header, OPERATION, false,
		//		null, OPERATION, clazz);
		if (!op.contentEquals(Operation.CREATE.toString()))
			if (!spec.equals(CURRENT_SPECIFICATION))
				throw new DIDResolveException("Invalid DID operation verb.");

		String payload = JsonHelper.getString(node, PAYLOAD, false,
				null, PAYLOAD, clazz);
		String docJson = new String(Base64.decode(payload,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP));
		DIDDocument doc = null;
		try {
			doc = DIDDocument.fromJson(docJson);
		} catch (DIDException e) {
			throw new DIDResolveException("Invalid document payload.", e);
		}

		JsonNode proof = node.get(PROOF);
		if (proof == null)
			throw new DIDResolveException("Missing proof.");

		String keyType = JsonHelper.getString(proof, KEY_TYPE, false,
				null, KEY_TYPE, clazz);
		if (!keyType.equals(Constants.defaultPublicKeyType))
			throw new DIDResolveException("Unknown signature key type.");

		DIDURL signKey = JsonHelper.getDidUrl(proof, KEY_ID, doc.getSubject(),
				KEY_ID, clazz);
		if (doc.getAuthenticationKey(signKey) == null)
			throw new DIDResolveException("Unknown signature key.");

		String sig = JsonHelper.getString(proof, SIGNATURE, false,
				null, SIGNATURE, clazz);

		IDChainRequest request = new IDChainRequest(Operation.CREATE, doc);
		request.payload = payload;
		request.keyType = keyType;
		request.signKey = signKey;
		request.signature = sig;

		return request;
	}
}
