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

package org.elastos.credential;

import java.util.Calendar;
import java.util.Date;
import java.util.Map;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDException;
import org.elastos.did.DIDStore;
import org.elastos.did.DIDStoreException;
import org.elastos.did.DIDURL;
import org.elastos.did.PublicKey;

public class Issuer {
	private DIDDocument self;
	private DIDURL defaultSignKey;

	public Issuer(DID did, DIDURL defaultSignKey)
			throws DIDException {
		DIDStore store = DIDStore.getInstance();

		this.self = store.resolveDid(did);
		if (this.self == null)
			throw new DIDException("Can not resolve DID.");

		if (defaultSignKey == null) {
			defaultSignKey = self.getDefaultPublicKey();
		} else {
			if (self.getAuthenticationKey(defaultSignKey) == null)
				throw new DIDException("Invalid sign key id.");
		}

		if (!store.containsPrivateKey(did, defaultSignKey))
			throw new DIDException("No private key.");

		this.defaultSignKey = defaultSignKey;
	}

	public Issuer(DID did) throws DIDException {
		this(did, null);
	}

	public CredentialBuilder issueFor(DID did) {
		return new CredentialBuilder(did);
	}

	public class CredentialBuilder {
		private DID target;
		private VerifiableCredential vc;

		protected CredentialBuilder(DID target) {
			this.target = target;

			vc = new VerifiableCredential();
			vc.setIssuer(self.getSubject());
		}

		public CredentialBuilder id(String id) {
			vc.setId(new DIDURL(target, id));
			return this;
		}

		public CredentialBuilder type(String[] type) {
			vc.setType(type);
			return this;
		}

		public CredentialBuilder issuer(DID issuer) {
			vc.setIssuer(self.getSubject());
			return this;
		}

		public CredentialBuilder expirationDate(Date expirationDate) {
			Calendar cal = Calendar.getInstance(Constants.UTC);
			cal.setTime(expirationDate);
			cal.set(Calendar.MINUTE, 0);
			cal.set(Calendar.SECOND, 0);
			cal.set(Calendar.MILLISECOND, 0);
			vc.setExpirationDate(cal.getTime());

			return this;
		}

		public CredentialBuilder properties(Map<String, String> properties) {
			VerifiableCredential.CredentialSubject subject =
					new VerifiableCredential.CredentialSubject(target);
			subject.addProperties(properties);
			vc.setSubject(subject);

			return this;
		}

		public VerifiableCredential sign(String passphrase)
				throws DIDStoreException {
			Calendar cal = Calendar.getInstance(Constants.UTC);
			cal.set(Calendar.MINUTE, 0);
			cal.set(Calendar.SECOND, 0);
			cal.set(Calendar.MILLISECOND, 0);
			vc.setIssuanceDate(cal.getTime());

			String json = vc.toJsonForSign(false);
			String sig = DIDStore.getInstance().sign(self.getSubject(),
					defaultSignKey, passphrase, json.getBytes());

			VerifiableCredential.Proof proof = new VerifiableCredential.Proof(
					Constants.defaultPublicKeyType, defaultSignKey, sig);
			vc.setProof(proof);

			return vc;
		}
	}
}
