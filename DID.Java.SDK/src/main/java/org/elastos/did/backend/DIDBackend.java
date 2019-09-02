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

import org.elastos.did.DID;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDException;
import org.elastos.did.DIDStoreException;
import org.elastos.did.DIDURL;

public class DIDBackend {
	private static DIDAdaptor adaptor;

	public static void initialize(DIDAdaptor adaptor) {
		DIDBackend.adaptor = adaptor;
	}

	public static boolean create(DIDDocument doc, DIDURL signKey,
			String passphrase) throws DIDStoreException {
		IDChainRequest request = new IDChainRequest(
				IDChainRequest.Operation.CREATE, doc);

		String json = request.sign(signKey, passphrase).toJson(true);

		try {
			return adaptor.createIdTransaction(json, null);
		} catch (DIDException e) {
			throw new DIDStoreException("Create ID transaction error.", e);
		}
	}

	public static DIDDocument resolve(DID did) throws DIDStoreException {
		try {
			String docJson = adaptor.resolve(did.toExternalForm());
			if (docJson == null)
				return null;

			DIDDocument doc = DIDDocument.fromJson(docJson);
			return doc;
		} catch (DIDException e) {
			throw new DIDStoreException("Resolve DID error.", e);
		}
	}

	public static boolean update(DIDDocument doc, DIDURL signKey,
			String passphrase) throws DIDStoreException {
		IDChainRequest request = new IDChainRequest(
				IDChainRequest.Operation.UPDATE, doc);

		String json = request.sign(signKey, passphrase).toJson(true);

		try {
			return adaptor.createIdTransaction(json, null);
		} catch (DIDException e) {
			throw new DIDStoreException("Create ID transaction error.", e);
		}
	}

	public static boolean deactivate(DID did, DIDURL signKey,
			String passphrase) throws DIDStoreException {
		IDChainRequest request = new IDChainRequest(
				IDChainRequest.Operation.CREATE, did);

		String json = request.sign(signKey, passphrase).toJson(true);

		try {
			return adaptor.createIdTransaction(json, null);
		} catch (DIDException e) {
			throw new DIDStoreException("Create ID transaction error.", e);
		}
	}
}
