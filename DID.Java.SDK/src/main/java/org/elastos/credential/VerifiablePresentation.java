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

		protected void toJson(JsonGenerator generator, DID ref, boolean compact)
				throws IOException {
			generator.writeStartObject();

			// type
			if (!compact || !type.equals(Constants.defaultPublicKeyType)) {
				generator.writeFieldName(Constants.type);
				generator.writeString(type);
			}

			// method
			String value;
			generator.writeFieldName(Constants.verificationMethod);
			if (compact && ref != null && verificationMethod.getDid().equals(ref))
				value = "#" + verificationMethod.getFragment();
			else
				value = verificationMethod.toExternalForm();
			generator.writeString(value);

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
		return credentials.get(id);
	}

	public DID getSigner() {
		return proof.getVerificationMethod().getDid();
	}

	public boolean verify() throws DIDException {
		DID signer = getSigner();

		for (VerifiableCredential vc : credentials.values()) {
			if (!vc.getSubject().getId().equals(signer))
				return false;

			if (!vc.verify() || vc.isExpired())
				return false;
		}

		String json = toJsonForSign();
		DIDDocument signerDoc = signer.resolve();

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
	 *   - nonce
	 *   - realm
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
			vc.toJson(generator, null, false);
		generator.writeEndArray();

		// proof
		if (!forSign ) {
			generator.writeFieldName(Constants.proof);
			proof.toJson(generator, null, false);
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

	protected String toJsonForSign() {
		Writer out = new StringWriter(4096);

		try {
			toJson(out, true);
		} catch (IOException ignore) {
		}

		return out.toString();
	}

	public String toExternalForm() {
		Writer out = new StringWriter(2048);

		try {
			toJson(out);
		} catch (IOException ignore) {
		}

		return out.toString();
	}

	@Override
	public String toString() {
		return toExternalForm();
	}

	public static Builder createFor(DID did, DIDURL signKey) {
		if (did == null)
			throw new IllegalArgumentException();

		return new Builder(did, signKey);
	}

	public static Builder createFor(DID did) {
		return new Builder(did, null);
	}

	public static class Builder {
		private DID did;
		private DIDURL signKey;
		private VerifiablePresentation presentation;

		protected Builder(DID did, DIDURL signKey) {
			this.did = did;
			this.signKey = signKey;
			this.presentation = new VerifiablePresentation();
		}

		public Builder credentials(VerifiableCredential ... credentials) {
			for (VerifiableCredential credential : credentials) {
				if (!credential.getSubject().getId().equals(did))
					throw new IllegalArgumentException("Credential not match with requested did");

				presentation.addCredential(credential);
			}

			return this;
		}

		public VerifiablePresentation sign(String storepass, String realm, String nonce)
				throws DIDException {
			if (nonce == null || nonce.isEmpty() || realm == null || realm.isEmpty())
				throw new IllegalArgumentException();

			DIDDocument doc = did.resolve();
			if (doc == null)
				throw new DIDException("Can not resolve DID.");

			if (signKey == null)
				signKey = doc.getDefaultPublicKey();

			String json = presentation.toJsonForSign();
			String sig = doc.sign(signKey, storepass, json.getBytes(),
					realm.getBytes(), nonce.getBytes());

			Proof proof = new Proof(signKey, realm, nonce, sig);
			presentation.setProof(proof);

			return presentation;
		}
	}
}
