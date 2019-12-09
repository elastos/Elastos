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
import org.elastos.did.DIDStoreException;
import org.elastos.did.DIDURL;
import org.elastos.did.MalformedDIDException;

public class Issuer {
	private DIDDocument self;
	private DIDURL signKey;

	public Issuer(DIDDocument doc, DIDURL signKey) throws DIDException {
		if (doc == null)
			throw new IllegalArgumentException();

		init(doc, signKey);
	}

	public Issuer(DIDDocument doc) throws DIDException {
		this(doc, null);
	}

	public Issuer(DID did, DIDURL signKey) throws DIDException {
		if (did == null)
			throw new IllegalArgumentException();

		DIDDocument doc = did.resolve();
		if (doc == null)
			throw new DIDException("Can not resolve DID.");

		init(doc, signKey);
	}

	public Issuer(DID did) throws DIDException {
		this(did, null);
	}

	private void init(DIDDocument doc, DIDURL signKey) throws DIDException {
		this.self = doc;

		if (signKey == null) {
			signKey = self.getDefaultPublicKey();
		} else {
			if (!self.isAuthenticationKey(signKey))
				throw new DIDException("Invalid sign key id.");
		}

		if (!self.hasPrivateKey(signKey))
			throw new DIDException("No private key.");

		this.signKey = signKey;

	}

	public DID getDid() {
		return self.getSubject();
	}

	public DIDURL getSignKey() {
		return signKey;
	}

	public CredentialBuilder issueFor(DID did) {
		if (did == null)
			throw new IllegalArgumentException();

		return new CredentialBuilder(did);
	}

	public CredentialBuilder issueFor(String did) throws MalformedDIDException {
		return issueFor(new DID(did));
	}

	public class CredentialBuilder {
		private DID target;
		private VerifiableCredential credential;

		protected CredentialBuilder(DID target) {
			this.target = target;

			credential = new VerifiableCredential();
			credential.setIssuer(self.getSubject());
		}

		public CredentialBuilder id(DIDURL id) {
			credential.setId(id);
			return this;
		}

		public CredentialBuilder id(String id) {
			return id(new DIDURL(target, id));
		}

		public CredentialBuilder type(String ... type) {
			credential.setType(type);
			return this;
		}

		private Calendar getMaxExpires() {
			Calendar cal = Calendar.getInstance(Constants.UTC);
			if (credential.getIssuanceDate() != null)
				cal.setTime(credential.getIssuanceDate());
			cal.add(Calendar.YEAR, Constants.MAX_VALID_YEARS);

			return cal;
		}

		private void defaultExpirationDate() {
			credential.setExpirationDate(getMaxExpires().getTime());
		}

		public CredentialBuilder expirationDate(Date expirationDate) {
			Calendar cal = Calendar.getInstance(Constants.UTC);
			cal.setTime(expirationDate);

			Calendar maxExpires = getMaxExpires();
			if (cal.after(maxExpires))
				cal = maxExpires;

			credential.setExpirationDate(cal.getTime());

			return this;
		}

		public CredentialBuilder properties(Map<String, String> properties) {
			VerifiableCredential.CredentialSubject subject =
					new VerifiableCredential.CredentialSubject(target);
			subject.addProperties(properties);
			credential.setSubject(subject);

			return this;
		}

		public VerifiableCredential seal(String storepass)
				throws MalformedCredentialException, DIDStoreException {
			if (credential.getId() == null)
				throw new MalformedCredentialException("Missing id.");

			if (credential.getTypes() == null)
				throw new MalformedCredentialException("Missing types.");

			if (credential.getSubject() == null)
				throw new MalformedCredentialException("Missing subject.");

			Calendar cal = Calendar.getInstance(Constants.UTC);
			credential.setIssuanceDate(cal.getTime());

			if (credential.getExpirationDate() == null)
				defaultExpirationDate();

			String json = credential.toJson(true, true);
			String sig = self.sign(signKey, storepass, json.getBytes());

			VerifiableCredential.Proof proof = new VerifiableCredential.Proof(
					Constants.defaultPublicKeyType, signKey, sig);
			credential.setProof(proof);

			// Should clean credential member
			VerifiableCredential vc = credential;
			this.credential = null;

			return vc;
		}
	}
}
