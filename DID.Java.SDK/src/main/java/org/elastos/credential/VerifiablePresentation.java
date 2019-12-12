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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.StringWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDException;
import org.elastos.did.DIDURL;
import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class VerifiablePresentation {
	private String type;
	private Date created;
	private Map<DIDURL, VerifiableCredential> credentials;
	private Proof proof;

	static public class Proof {
		private String type;
		private DIDURL verificationMethod;
		private String realm;
		private String nonce;
		private String signature;

		protected Proof(String type, DIDURL method, String realm,
				String nonce, String signature) {
			this.type = type;
			this.verificationMethod = method;
			this.realm = realm;
			this.nonce = nonce;
			this.signature = signature;
		}

		protected Proof(DIDURL method, String realm,
				String nonce, String signature) {
			this(Constants.defaultPublicKeyType, method, realm, nonce, signature);
		}

	    public String getType() {
	    	return type;
	    }

	    public DIDURL getVerificationMethod() {
	    	return verificationMethod;
	    }

	    public String getRealm() {
	    	return realm;
	    }

	    public String getNonce() {
	    	return nonce;
	    }

	    public String getSignature() {
	    	return signature;
	    }

		protected static Proof fromJson(JsonNode node, DID ref)
				throws MalformedPresentationException {
			Class<MalformedPresentationException> clazz = MalformedPresentationException.class;

			String type = JsonHelper.getString(node, Constants.type,
					true, Constants.defaultPublicKeyType,
					"presentation proof type", clazz);

			DIDURL method = JsonHelper.getDidUrl(node,
					Constants.verificationMethod, ref,
					"presentation proof verificationMethod", clazz);

			String realm = JsonHelper.getString(node, Constants.realm,
					false, null, "presentation proof realm", clazz);

			String nonce = JsonHelper.getString(node, Constants.nonce,
					false, null, "presentation proof nonce", clazz);

			String signature = JsonHelper.getString(node, Constants.signature,
					false, null, "presentation proof signature", clazz);

			return new Proof(type, method, realm, nonce, signature);
		}

		protected void toJson(JsonGenerator generator) throws IOException {
			generator.writeStartObject();

			// type
			generator.writeFieldName(Constants.type);
			generator.writeString(type);

			// method
			generator.writeFieldName(Constants.verificationMethod);
			generator.writeString(verificationMethod.toString());

			// realm
			generator.writeFieldName(Constants.realm);
			generator.writeString(realm);

			// nonce
			generator.writeFieldName(Constants.nonce);
			generator.writeString(nonce);

			// signature
			generator.writeFieldName(Constants.signature);
			generator.writeString(signature);

			generator.writeEndObject();
		}
	}

	protected VerifiablePresentation() {
		type = Constants.defaultPresentationType;

		Calendar cal = Calendar.getInstance(Constants.UTC);
		created = cal.getTime();

		credentials = new TreeMap<DIDURL, VerifiableCredential>();
	}

	public String getType() {
		return type;
	}

	protected void setType(String type) {
		this.type = type;
	}

	public Date getCreated() {
		return created;
	}

	protected void setCreated(Date created) {
		this.created = created;
	}

	public int getCredentialCount() {
		return credentials.size();
	}

	public List<VerifiableCredential> getCredentials() {
		List<VerifiableCredential> lst = new ArrayList<VerifiableCredential>(
				credentials.size());

		lst.addAll(credentials.values());
		return lst;
	}

	protected void addCredential(VerifiableCredential credential) {
		credentials.put(credential.getId(), credential);
	}

	public VerifiableCredential getCredential(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return credentials.get(id);
	}

	public VerifiableCredential getCredential(String id) {
		return credentials.get(new DIDURL(getSigner(), id));
	}

	public DID getSigner() {
		return proof.getVerificationMethod().getDid();
	}

	public boolean isGenuine() throws DIDException {
		DID signer = getSigner();
		DIDDocument signerDoc = signer.resolve();
		if (signerDoc == null)
			return false;

		// Check the integrity of signer' document.
		if (!signerDoc.isGenuine())
			return false;

		// Unsupported public key type;
		if (!proof.getType().equals(Constants.defaultPublicKeyType))
			return false;

		// Credential should signed by authentication key.
		if (!signerDoc.isAuthenticationKey(proof.getVerificationMethod()))
			return false;

		// All credentials should owned by signer
		for (VerifiableCredential vc : credentials.values()) {
			if (!vc.getSubject().getId().equals(signer))
				return false;

			if (!vc.isGenuine())
				return false;
		}

		String json = toJson(true);
		return signerDoc.verify(proof.getVerificationMethod(),
				proof.getSignature(), json.getBytes(),
				proof.getRealm().getBytes(), proof.getNonce().getBytes());
	}

	public boolean isValid() throws DIDException {
		DID signer = getSigner();
		DIDDocument signerDoc = signer.resolve();

		// Check the validity of signer' document.
		if (!signerDoc.isValid())
			return false;

		// Unsupported public key type;
		if (!proof.getType().equals(Constants.defaultPublicKeyType))
			return false;

		// Credential should signed by authentication key.
		if (!signerDoc.isAuthenticationKey(proof.getVerificationMethod()))
			return false;

		// All credentials should owned by signer
		for (VerifiableCredential vc : credentials.values()) {
			if (!vc.getSubject().getId().equals(signer))
				return false;

			if (!vc.isValid())
				return false;
		}

		String json = toJson(true);
		return signerDoc.verify(proof.getVerificationMethod(),
				proof.getSignature(), json.getBytes(),
				proof.getRealm().getBytes(), proof.getNonce().getBytes());
	}

	public Proof getProof() {
		return proof;
	}

	protected void setProof(Proof proof) {
		this.proof = proof;
	}

	private void parse(Reader reader) throws MalformedPresentationException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(reader);
			parse(node);
		} catch (IOException e) {
			throw new MalformedPresentationException("Parse presentation error.", e);
		}
	}

	private void parse(InputStream in) throws MalformedPresentationException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(in);
			parse(node);
		} catch (IOException e) {
			throw new MalformedPresentationException("Parse presentation error.", e);
		}
	}

	private void parse(String json) throws MalformedPresentationException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(json);
			parse(node);
		} catch (IOException e) {
			throw new MalformedPresentationException("Parse presentation error.", e);
		}
	}

	private void parse(JsonNode presentation) throws MalformedPresentationException {
		Class<MalformedPresentationException> clazz = MalformedPresentationException.class;

		String type = JsonHelper.getString(presentation, Constants.type,
				false, null, "presentation type", clazz);
		if (!type.contentEquals(Constants.defaultPresentationType))
			throw new MalformedPresentationException("Unknown presentation type: " + type);
		else
			setType(type);

		Date created = JsonHelper.getDate(presentation, Constants.created,
				false, null, "presentation created date", clazz);
		setCreated(created);

		JsonNode node = presentation.get(Constants.verifiableCredential);
		if (node == null)
			throw new MalformedPresentationException("Missing credentials.");
		parseCredential(node);

		node = presentation.get(Constants.proof);
		if (node == null)
			throw new MalformedPresentationException("Missing credentials.");
		Proof proof = Proof.fromJson(node, null);
		setProof(proof);
	}

	private void parseCredential(JsonNode node)
			throws MalformedPresentationException {
		if (!node.isArray())
			throw new MalformedPresentationException(
					"Invalid verifiableCredentia, should be an array.");

		if (node.size() == 0)
			throw new MalformedPresentationException(
					"Invalid verifiableCredentia, should not be an empty array.");

		for (int i = 0; i < node.size(); i++) {
			try {
				VerifiableCredential vc = VerifiableCredential.fromJson(node.get(i));
				addCredential(vc);
			} catch (MalformedCredentialException e) {
				throw new MalformedPresentationException(e.getMessage(), e);
			}
		}
	}

	public static VerifiablePresentation fromJson(Reader reader)
			throws MalformedPresentationException {
		if (reader == null)
			throw new IllegalArgumentException();

		VerifiablePresentation vp = new VerifiablePresentation();
		vp.parse(reader);

		return vp;
	}

	public static VerifiablePresentation fromJson(InputStream in)
			throws MalformedPresentationException {
		if (in == null)
			throw new IllegalArgumentException();

		VerifiablePresentation vp = new VerifiablePresentation();
		vp.parse(in);

		return vp;
	}

	public static VerifiablePresentation fromJson(String json)
			throws MalformedPresentationException {
		if (json == null || json.isEmpty())
			throw new IllegalArgumentException();

		VerifiablePresentation vp = new VerifiablePresentation();
		vp.parse(json);

		return vp;
	}

	/*
	 * Normalized serialization order:
	 *
	 * - type
	 * - created
	 * - verifiableCredential (ordered by name(case insensitive/ascending)
	 * + proof
	 *   - type
	 *   - verificationMethod
	 *   - realm
	 *   - nonce
	 *   - signature
	 */
	protected void toJson(JsonGenerator generator, boolean forSign)
			throws IOException {
		generator.writeStartObject();

		// type
		generator.writeFieldName(Constants.type);
		generator.writeString(type);

		// created
		generator.writeFieldName(Constants.created);
		generator.writeString(JsonHelper.format(created));

		// credentials
		generator.writeFieldName(Constants.verifiableCredential);
		generator.writeStartArray();
		for (VerifiableCredential vc : credentials.values())
			vc.toJson(generator, null, true);
		generator.writeEndArray();

		// proof
		if (!forSign ) {
			generator.writeFieldName(Constants.proof);
			proof.toJson(generator);
		}

		generator.writeEndObject();
	}

	protected void toJson(Writer out, boolean forSign) throws IOException {
		JsonFactory factory = new JsonFactory();
		JsonGenerator generator = factory.createGenerator(out);
		toJson(generator, forSign);
		generator.close();
	}

	public void toJson(Writer out) throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

		toJson(out, false);
	}

	public void toJson(OutputStream out, String charsetName)
			throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

		if (charsetName == null)
			charsetName = "UTF-8";

		toJson(new OutputStreamWriter(out, charsetName));
	}

	public void toJson(OutputStream out) throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

		toJson(new OutputStreamWriter(out));
	}

	protected String toJson(boolean forSign) {
		Writer out = new StringWriter(4096);

		try {
			toJson(out, forSign);
		} catch (IOException ignore) {
		}

		return out.toString();
	}

	@Override
	public String toString() {
		return toJson(false);
	}

	public static Builder createFor(DID did, DIDURL signKey) throws DIDException {
		if (did == null)
			throw new IllegalArgumentException();

		return new Builder(did, signKey);
	}

	public static Builder createFor(DID did) throws DIDException {
		return new Builder(did, null);
	}

	public static class Builder {
		private DIDDocument signer;
		private DIDURL signKey;
		private String realm;
		private String nonce;
		private VerifiablePresentation presentation;

		protected Builder(DID did, DIDURL signKey) throws DIDException {
			this.signer = did.resolve();
			if (signer == null)
				throw new DIDException("Can not resolve DID.");

			if (signKey == null) {
				signKey = signer.getDefaultPublicKey();
			} else {
				if (!signer.isAuthenticationKey(signKey))
					throw new DIDException("Invalid sign key id.");
			}

			if (!signer.hasPrivateKey(signKey))
				throw new DIDException("No private key.");

			this.signKey = signKey;
			this.presentation = new VerifiablePresentation();
		}

		public Builder credentials(VerifiableCredential ... credentials)
				throws DIDException {
			if (presentation == null)
				throw new IllegalStateException("Presentation already sealed.");

			for (VerifiableCredential vc : credentials) {
				if (!vc.getSubject().getId().equals(signer.getSubject()))
					throw new IllegalArgumentException("Credential '" +
							vc.getId() + "' not match with requested did");

				if (!vc.isValid())
					throw new IllegalArgumentException("Credential '" +
							vc.getId() + "' is invalid");

				presentation.addCredential(vc);
			}

			return this;
		}

		public Builder realm(String realm) {
			if (presentation == null)
				throw new IllegalStateException("Presentation already sealed.");

			if (realm == null || realm.isEmpty())
				throw new IllegalArgumentException();

			this.realm = realm;
			return this;
		}

		public Builder nonce(String nonce) {
			if (presentation == null)
				throw new IllegalStateException("Presentation already sealed.");

			if (nonce == null || nonce.isEmpty())
				throw new IllegalArgumentException();

			this.nonce = nonce;
			return this;
		}

		public VerifiablePresentation seal(String storepass)
				throws DIDException {
			if (presentation == null)
				throw new IllegalStateException("Presentation already sealed.");

			if (storepass == null || storepass.isEmpty())
				throw new IllegalArgumentException();

			String json = presentation.toJson(true);
			String sig = signer.sign(signKey, storepass, json.getBytes(),
					realm.getBytes(), nonce.getBytes());

			Proof proof = new Proof(signKey, realm, nonce, sig);
			presentation.setProof(proof);

			// Invalidate builder
			VerifiablePresentation vp = presentation;
			this.presentation = null;

			return vp;
		}
	}
}
