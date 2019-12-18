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

import org.elastos.did.DID;
import org.elastos.did.DIDAdapter;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDURL;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.did.exception.DIDStoreException;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class DIDBackend {
	private DIDAdapter adapter;

	public DIDBackend(DIDAdapter adapter) {
		this.adapter = adapter;
	}

	public DIDAdapter getAdapter() {
		return adapter;
	}

	public boolean create(DIDDocument doc, DIDURL signKey,
			String storepass) throws DIDStoreException {
		IDChainRequest request = IDChainRequest.create(doc, signKey, storepass);
		String json = request.toJson(true);

		try {
			return adapter.createIdTransaction(json, null);
		} catch (DIDException e) {
			throw new DIDStoreException("Create ID transaction error.", e);
		}
	}

	public ResolveResult resolve(DID did) throws DIDResolveException {
		String res = adapter.resolve(did.getMethodSpecificId(), false);
		if (res == null)
			return null;

		ObjectMapper mapper = new ObjectMapper();
		JsonNode node = null;

		try {
			node = mapper.readTree(res);
		} catch (IOException e) {
			throw new DIDResolveException("Parse resolved json error.", e);
		}

		JsonNode result = node.get("result");
		if (result == null || result.isNull()) {
			JsonNode error = node.get("error");
			throw new DIDResolveException("Resolve DID error("
					+ error.get("code").longValue() + "): "
					+ error.get("message").textValue());
		}

		return ResolveResult.fromJson(result);
	}

	public boolean update(DIDDocument doc, String previousTxid, DIDURL signKey,
			String storepass) throws DIDStoreException {
		IDChainRequest request = IDChainRequest.update(doc,
				previousTxid, signKey, storepass);
		String json = request.toJson(true);

		try {
			return adapter.createIdTransaction(json, null);
		} catch (DIDException e) {
			throw new DIDStoreException("Create ID transaction error.", e);
		}
	}

	public boolean deactivate(DID did, DIDURL signKey,
			String storepass) throws DIDStoreException {
		IDChainRequest request = IDChainRequest.deactivate(did, signKey, storepass);
		String json = request.toJson(true);

		try {
			return adapter.createIdTransaction(json, null);
		} catch (DIDException e) {
			throw new DIDStoreException("Create ID transaction error.", e);
		}
	}
}
