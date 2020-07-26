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

package org.elastos.did.examples;

import java.io.File;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.elastos.did.DID;
import org.elastos.did.DIDBackend;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDStore;
import org.elastos.did.Issuer;
import org.elastos.did.Issuer.CredentialBuilder;
import org.elastos.did.Mnemonic;
import org.elastos.did.VerifiableCredential;
import org.elastos.did.VerifiablePresentation;
import org.elastos.did.adapter.DummyAdapter;
import org.elastos.did.exception.DIDException;
import org.elastos.did.jwt.Claims;
import org.elastos.did.jwt.Header;
import org.elastos.did.jwt.Jws;
import org.elastos.did.jwt.JwsSignatureException;
import org.elastos.did.jwt.JwtParser;
import org.elastos.did.jwt.JwtParserBuilder;

public class PresentationInJWT {
	// DummyAdapter only for demo and testing.
	private static DummyAdapter adapter;

	public static class Entity {
		// Mnemonic passphrase and the store password should set by the end user.
		private final static String passphrase = "mypassphrase";
		private final static String storepass = "password";

		private String name;
		private DIDStore store;
		private DID did;

		protected Entity(String name) throws DIDException {
			this.name = name;

			initPrivateIdentity();
			initDid();
		}

		protected void initPrivateIdentity() throws DIDException {
			final String storePath = System.getProperty("java.io.tmpdir")
					+ File.separator + name + ".store";

			// Create a fake adapter, just print the tx payload to console.
			store = DIDStore.open("filesystem", storePath, adapter);

			// Check the store whether contains the root private identity.
			if (store.containsPrivateIdentity())
				return; // Already exists

			// Create a mnemonic use default language(English).
			Mnemonic mg = Mnemonic.getInstance();
			String mnemonic = mg.generate();

			System.out.format("[%s] Please write down your mnemonic and passwords:%n", name);
			System.out.println("  Mnemonic: " + mnemonic);
			System.out.println("  Mnemonic passphrase: " + passphrase);
			System.out.println("  Store password: " + storepass);

			// Initialize the root identity.
			store.initPrivateIdentity(null, mnemonic, passphrase, storepass);
		}

		protected void initDid() throws DIDException {
			// Check the DID store already contains owner's DID(with private key).
			List<DID> dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
			if (dids.size() > 0) {
				for (DID did : dids) {
					if (did.getMetadata().getAlias().equals("me")) {
						// Already create my DID.
						System.out.format("[%s] My DID: %s%n", name, did);
						this.did = did;

						// This only for dummy backend.
						// normally don't need this on ID sidechain.
						store.publishDid(did, storepass);
						return;
					}
				}
			}

			DIDDocument doc = store.newDid("me", storepass);
			this.did = doc.getSubject();
			System.out.format("[%s] My new DID created: %s%n", name, did);
			store.publishDid(did, storepass);
		}

		protected DIDStore getDIDStore() {
			return store;
		}

		public DID getDid() {
			return did;
		}

		public DIDDocument getDocument() throws DIDException {
			return store.loadDid(did);
		}

		public String getName() {
			return name;
		}

		protected String getStorePassword() {
			return storepass;
		}
	}

	public static class University extends Entity {
		private Issuer issuer;

		public University(String name) throws DIDException {
			super(name);

			issuer = new Issuer(getDocument());
		}

		public VerifiableCredential issueDiplomaFor(Student student) throws DIDException {
			Map<String, String> subject = new HashMap<String, String>();
			subject.put("name", student.getName());
			subject.put("degree", "bachelor");
			subject.put("institute", "Computer Science");
			subject.put("university", getName());

			Calendar exp = Calendar.getInstance();
			exp.add(Calendar.YEAR, 5);

			CredentialBuilder cb = issuer.issueFor(student.getDid());
			VerifiableCredential vc = cb.id("diploma")
				.type("DiplomaCredential")
				.properties(subject)
				.expirationDate(exp.getTime())
				.seal(getStorePassword());

			return vc;
		}
	}

	public static class Student extends Entity {
		private String gender;
		private String email;
		private List<VerifiableCredential> vcs;

		public Student(String name, String gender, String email) throws DIDException {
			super(name);
			this.gender = gender;
			this.email = email;

			this.vcs = new ArrayList<VerifiableCredential>(4);
		}

		public VerifiableCredential createSelfProclaimedCredential() throws DIDException {
			Map<String, String> subject = new HashMap<String, String>();
			subject.put("name", getName());
			subject.put("gender", gender);
			subject.put("email", email);

			Calendar exp = Calendar.getInstance();
			exp.add(Calendar.YEAR, 1);

			CredentialBuilder cb = new Issuer(getDocument()).issueFor(getDid());
			VerifiableCredential vc = cb.id("profile")
				.type("ProfileCredential", "SelfProclaimedCredential")
				.properties(subject)
				.expirationDate(exp.getTime())
				.seal(getStorePassword());

			return vc;
		}

		public void addCredential(VerifiableCredential vc) {
			vcs.add(vc);
		}

		public VerifiablePresentation createPresentation(String realm, String nonce) throws DIDException {
			VerifiablePresentation.Builder vpb = VerifiablePresentation.createFor(getDid(), getDIDStore());

			return vpb.credentials(vcs.toArray(new VerifiableCredential[vcs.size()]))
				.realm(realm)
				.nonce(nonce)
				.seal(getStorePassword());
		}
	}

	private static void initDIDBackend() throws DIDException {
		// Get DID resolve cache dir.
		final String cacheDir = System.getProperty("user.home") + File.separator + ".cache"
				+ File.separator + "elastos.did";

		// Dummy adapter for easy to use
		adapter = new DummyAdapter();

		// Initializa the DID backend globally.
		DIDBackend.initialize(adapter, cacheDir);
	}

	public static void main(String args[]) {
		try {
			initDIDBackend();

			University university = new University("Elastos");
			Student student = new Student("John Smith", "Male", "johnsmith@example.org");

			VerifiableCredential vc = university.issueDiplomaFor(student);
			System.out.println("The diploma credential:");
			System.out.println("  " + vc);
			System.out.println("  Genuine: " + vc.isGenuine());
			System.out.println("  Expired: " + vc.isExpired());
			System.out.println("  Valid: " + vc.isValid());
			student.addCredential(vc);

			vc = student.createSelfProclaimedCredential();
			System.out.println("The profile credential:");
			System.out.println("  " + vc);
			System.out.println("  Genuine: " + vc.isGenuine());
			System.out.println("  Expired: " + vc.isExpired());
			System.out.println("  Valid: " + vc.isValid());
			student.addCredential(vc);

			VerifiablePresentation vp = student.createPresentation("test", "873172f58701a9ee686f0630204fee59");
			System.out.println("The verifiable presentation:");
			System.out.println("  " + vp);
			System.out.println("  Genuine: " + vp.isGenuine());
			System.out.println("  Valid: " + vp.isValid());

			Calendar cal = Calendar.getInstance();
			cal.set(Calendar.MILLISECOND, 0);
			Date iat = cal.getTime();
			Date nbf = cal.getTime();
			cal.add(Calendar.MONTH, 3);
			Date exp = cal.getTime();

			// Create JWT token with presentation.
			String token = student.getDocument().jwtBuilder()
					.addHeader(Header.TYPE, Header.JWT_TYPE)
					.setId("test00000000")
					.setAudience(university.getDid().toString())
					.setIssuedAt(iat)
					.setNotBefore(nbf)
					.setExpiration(exp)
					.claimWithJson("presentation", vp.toString())
					.sign(student.getStorePassword())
					.compact();

			System.out.println("JWT Token:");
			System.out.println("  " + token);

			// Verify the token automatically
			JwtParser jp = new JwtParserBuilder().build();
			Jws<Claims> jwt = jp.parseClaimsJws(token);

			// Get claims from the token
			String preJson = jwt.getBody().getAsJson("presentation");
			vp = VerifiablePresentation.fromJson(preJson);
			System.out.println("Presentation from JWT:");
			System.out.println("  " + vp);
			System.out.println("  Genuine: " + vp.isGenuine());
			System.out.println("  Valid: " + vp.isValid());

			// Verify the token based on a DID
			// This will success, because the JWT was signed by the student
			jp = student.getDocument().jwtParserBuilder().build();
			jwt = jp.parseClaimsJws(token);

			// This will failed, because the JWT was signed by the student not by the university
			jp = university.getDocument().jwtParserBuilder().build();
			try {
				jwt = jp.parseClaimsJws(token);
			} catch (JwsSignatureException e) {
				// Should be here.
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
