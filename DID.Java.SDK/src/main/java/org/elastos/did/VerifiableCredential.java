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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.StringWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.MalformedCredentialException;
import org.elastos.did.meta.CredentialMeta;
import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class VerifiableCredential extends DIDObject {
	private List<String> types;
	private DID issuer;
	private Date issuanceDate;
	private Date expirationDate;
	private CredentialSubject subject;
	private Proof proof;

	private CredentialMeta meta;

	static public class CredentialSubject {
		private DID id;
		// TODO: use JSON object?
		private Map<String, String> properties;

		protected CredentialSubject(DID id) {
			this.id = id;
			properties = new TreeMap<String, String>();
		}

		public DID getId() {
			return id;
		}

		public int getPropertyCount() {
			return properties.size();
		}

		public Map<String, String> getProperties() {
			// Make a copy
			return new LinkedHashMap<String, String>(properties);
		}

		public String getProperty(String name) {
			return properties.get(name);
		}

		protected void addProperty(String name, String value) {
			properties.put(name, value);
		}

		protected void addProperties(Map<String, String> props) {
			properties.putAll(props);
		}

		protected static CredentialSubject fromJson(JsonNode node, DID ref)
				throws MalformedCredentialException {
			Class<MalformedCredentialException> clazz = MalformedCredentialException.class;

			// id
			DID id = JsonHelper.getDid(node, Constants.id, ref != null, ref,
					"crendentialSubject id", clazz);

			CredentialSubject cs = new CredentialSubject(id);

			Iterator<Map.Entry<String, JsonNode>> props = node.fields();
			Map.Entry<String, JsonNode> prop;
			while (props.hasNext()) {
				prop = props.next();

				if (prop.getKey().equals(Constants.id))
					continue;

				cs.addProperty(prop.getKey(), prop.getValue().asText());
			}

			// TODO: should check whether the subject is empty?

			return cs;
		}

		protected void toJson(JsonGenerator generator, DID ref, boolean normalized)
				throws IOException {
			generator.writeStartObject();

			// id
			if (normalized || ref == null || !getId().equals(ref)) {
				generator.writeFieldName(Constants.id);
				generator.writeString(getId().toString());
			}

			// Properties
			for (Map.Entry<String, String> prop : properties.entrySet()) {
				generator.writeFieldName(prop.getKey());
				generator.writeString(prop.getValue());
			}

			generator.writeEndObject();
		}
	}

	static public class Proof {
		private String type;
		private DIDURL verificationMethod;
		private String signature;

		protected Proof(String type, DIDURL method, String signature) {
			this.type = type;
			this.verificationMethod = method;
			this.signature = signature;
		}

	    public String getType() {
	    	return type;
	    }

	    public DIDURL getVerificationMethod() {
	    	return verificationMethod;
	    }

	    public String getSignature() {
	    	return signature;
	    }

		protected static Proof fromJson(JsonNode node, DID ref)
				throws MalformedCredentialException {
			Class<MalformedCredentialException> clazz = MalformedCredentialException.class;

			String type = JsonHelper.getString(node, Constants.type,
					true, Constants.defaultPublicKeyType,
					"crendential proof type", clazz);

			DIDURL method = JsonHelper.getDidUrl(node,
					Constants.verificationMethod, ref,
					"crendential proof verificationMethod", clazz);

			String signature = JsonHelper.getString(node, Constants.signature,
					false, null, "crendential proof signature", clazz);

			return new Proof(type, method, signature);
		}

		protected void toJson(JsonGenerator generator, DID ref, boolean normalized)
				throws IOException {
			generator.writeStartObject();

			// type
			if (normalized || !type.equals(Constants.defaultPublicKeyType)) {
				generator.writeFieldName(Constants.type);
				generator.writeString(type);
			}

			// method
			String value;
			generator.writeFieldName(Constants.verificationMethod);
			if (normalized || ref == null || !verificationMethod.getDid().equals(ref))
				value = verificationMethod.toString();
			else
				value = "#" + verificationMethod.getFragment();
			generator.writeString(value);

			// signature
			generator.writeFieldName(Constants.signature);
			generator.writeString(signature);

			generator.writeEndObject();
		}
	}

	protected VerifiableCredential() {
		super(null, null);
	}

	protected VerifiableCredential(VerifiableCredential vc) {
		setId(vc.getId());

		this.types = vc.types;
		this.issuer = vc.issuer;
		this.issuanceDate = vc.issuanceDate;
		this.expirationDate = vc.expirationDate;
		this.subject = vc.subject;
		this.proof = vc.proof;
	}

	@Override
	protected void setId(DIDURL id) {
		super.setId(id);
	}

	@Override
	public String getType() {
		StringBuilder builder = new StringBuilder(512);
		boolean initial = true;

		builder.append("[");

		if (types != null) {
			for (String t : types) {
				if (initial)
					initial = false;
				else
					builder.append(", ");

				builder.append(t);
			}
		}

		builder.append("]");

		return builder.toString();
	}

	public String[] getTypes() {
		return types == null ? null : types.toArray(new String[0]);
	}

	protected void addType(String type) {
		if (types == null)
			types = new ArrayList<String>(4);

		types.add(type);
	}

	protected void setType(String[] type) {
		if (types == null)
			types = new ArrayList<String>(type.length);

		for (String t : type)
			types.add(t);
	}

	public DID getIssuer() {
		return issuer;
	}

	protected void setIssuer(DID issuer) {
		this.issuer = issuer;
	}

	public Date getIssuanceDate() {
		return issuanceDate;
	}

	protected void setIssuanceDate(Date issuanceDate) {
		this.issuanceDate = issuanceDate;
	}

	public Date getExpirationDate() {
		return expirationDate;
	}

	protected void setMeta(CredentialMeta meta) {
		this.meta = meta;
	}

	protected CredentialMeta getMeta() {
		if (meta == null)
			meta = new CredentialMeta();

		return meta;
	}

	public void setExtra(String name, String value) throws DIDStoreException {
		if (name == null || name.isEmpty())
			throw new IllegalArgumentException();

		getMeta().setExtra(name, value);

		if (getMeta().attachedStore())
			getMeta().getStore().storeCredentialMeta(getSubject().getId(),
					getId(), meta);
	}

	public String getExtra(String name) {
		if (name == null || name.isEmpty())
			throw new IllegalArgumentException();

		return getMeta().getExtra(name);
	}

	public void setAlias(String alias) throws DIDStoreException {
		getMeta().setAlias(alias);

		if (getMeta().attachedStore())
			getMeta().getStore().storeCredentialMeta(getSubject().getId(),
					getId(), meta);
	}

	public String getAlias() {
		return getMeta().getAlias();
	}

	public boolean isSelfProclaimed() {
		return issuer.equals(subject.id);
	}

	private static final int RULE_EXPIRE = 1;
	private static final int RULE_GENUINE = 2;
	private static final int RULE_VALID = 3;

	private boolean traceCheck(int rule) throws DIDException {
		DIDDocument controllerDoc = subject.id.resolve();
		if (controllerDoc == null)
			return false;

		switch (rule) {
		case RULE_EXPIRE:
			if (controllerDoc.isExpired())
				return true;
			break;

		case RULE_GENUINE:
			if (!controllerDoc.isGenuine())
				return false;
			break;

		case RULE_VALID:
			if (!controllerDoc.isValid())
				return false;
			break;
		}

		if (!isSelfProclaimed()) {
			DIDDocument issuerDoc = issuer.resolve();
			switch (rule) {
			case RULE_EXPIRE:
				if (issuerDoc.isExpired())
					return true;
				break;

			case RULE_GENUINE:
				if (!issuerDoc.isGenuine())
					return false;
				break;

			case RULE_VALID:
				if (!issuerDoc.isValid())
					return false;
				break;
			}
		}

		return rule != RULE_EXPIRE;
	}

	private boolean checkExpired() throws DIDException {
		if (expirationDate != null) {
			Calendar now = Calendar.getInstance(Constants.UTC);

			Calendar expireDate  = Calendar.getInstance(Constants.UTC);
			expireDate.setTime(expirationDate);

			return now.after(expireDate);
		}

		return false;
	}

	public boolean isExpired() throws DIDException {
		if (traceCheck(RULE_EXPIRE))
			return true;

		return checkExpired();
	}

	private boolean checkGenuine() throws DIDException {
		DIDDocument issuerDoc = issuer.resolve();

		// Credential should signed by authentication key.
		if (!issuerDoc.isAuthenticationKey(proof.getVerificationMethod()))
			return false;

		// Unsupported public key type;
		if (!proof.getType().equals(Constants.defaultPublicKeyType))
			return false;

		String json = toJson(true, true);

		return issuerDoc.verify(proof.getVerificationMethod(),
				proof.getSignature(), json.getBytes());
	}

	public boolean isGenuine() throws DIDException {
		if (!traceCheck(RULE_GENUINE))
			return false;

		return checkGenuine();
	}

	public boolean isValid() throws DIDException {
		if (!traceCheck(RULE_VALID))
			return false;

		return !checkExpired() && checkGenuine();
	}

	protected void setExpirationDate(Date expirationDate) {
		this.expirationDate = expirationDate;
	}

	public CredentialSubject getSubject() {
		return subject;
	}

	protected void setSubject(CredentialSubject subject) {
		this.subject = subject;
	}

	public Proof getProof() {
		return proof;
	}

	protected void setProof(Proof proof) {
		this.proof = proof;
	}

	private void parse(Reader reader) throws MalformedCredentialException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(reader);
			parse(node, null);
		} catch (IOException e) {
			throw new MalformedCredentialException("Parse JSON document error.", e);
		}
	}

	private void parse(InputStream in) throws MalformedCredentialException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(in);
			parse(node, null);
		} catch (IOException e) {
			throw new MalformedCredentialException("Parse JSON document error.", e);
		}
	}

	private void parse(String json) throws MalformedCredentialException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(json);
			parse(node, null);
		} catch (IOException e) {
			throw new MalformedCredentialException("Parse JSON document error.", e);
		}
	}

	/*
	private void parse(Reader reader, DID ref) throws MalformedCredentialException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(reader);
			parse(node, ref);
		} catch (IOException e) {
			throw new MalformedCredentialException("Parse JSON document error.", e);
		}
	}

	private void parse(JsonNode node) throws MalformedCredentialException {
		parse(node, null);
	}
	*/

	private void parse(JsonNode node, DID ref) throws MalformedCredentialException {
		Class<MalformedCredentialException> clazz = MalformedCredentialException.class;

		// type
		JsonNode valueNode = node.get(Constants.type);
		if (valueNode == null)
			throw new MalformedCredentialException("Missing credential type.");

		if (!valueNode.isArray() || valueNode.size() == 0)
			throw new MalformedCredentialException(
					"Invalid credential type, should be an array.");

		for (int i = 0; i < valueNode.size(); i++) {
			String t = valueNode.get(i).asText();
			if (t != null && !t.isEmpty())
				addType(t);
		}

		// issuer
		issuer = JsonHelper.getDid(node, Constants.issuer,
				true, ref, "crendential issuer", clazz);

		// issuanceDate
		issuanceDate = JsonHelper.getDate(node, Constants.issuanceDate,
				false, null, "credential issuanceDate", clazz);

		// expirationDate
		expirationDate = JsonHelper.getDate(node, Constants.expirationDate,
				true, null, "credential expirationDate", clazz);

		// credentialSubject
		valueNode = node.get(Constants.credentialSubject);
		if (valueNode == null)
			throw new MalformedCredentialException("Missing credentialSubject.");
		subject = CredentialSubject.fromJson(valueNode, ref);

		// id
		DIDURL id = JsonHelper.getDidUrl(node, Constants.id,
				ref != null ? ref : subject.getId(), "crendential id", clazz);
		setId(id);

		// IMPORTANT: help resolve full method in proof
		if (issuer == null)
			issuer = subject.getId();

		// proof
		valueNode = node.get(Constants.proof);
		if (valueNode == null)
			throw new MalformedCredentialException("Missing credential proof.");
		proof = Proof.fromJson(valueNode, issuer);
	}

	public static VerifiableCredential fromJson(Reader reader)
			throws MalformedCredentialException {
		if (reader == null)
			throw new IllegalArgumentException();

		VerifiableCredential vc = new VerifiableCredential();
		vc.parse(reader);

		return vc;
	}

	public static VerifiableCredential fromJson(InputStream in)
			throws MalformedCredentialException {
		if (in == null)
			throw new IllegalArgumentException();

		VerifiableCredential vc = new VerifiableCredential();
		vc.parse(in);

		return vc;
	}

	public static VerifiableCredential fromJson(String json)
			throws MalformedCredentialException {
		if (json == null || json.isEmpty())
			throw new IllegalArgumentException();

		VerifiableCredential vc = new VerifiableCredential();
		vc.parse(json);

		return vc;
	}

	protected static VerifiableCredential fromJson(JsonNode node, DID ref)
			throws MalformedCredentialException {
		VerifiableCredential vc = new VerifiableCredential();
		vc.parse(node, ref);
		return vc;
	}

	protected static VerifiableCredential fromJson(JsonNode node)
			throws MalformedCredentialException {
		return fromJson(node, null);
	}

	protected void toJson(JsonGenerator generator, DID ref, boolean normalized)
			throws IOException {
		toJson(generator, ref, normalized, false);
	}

	/*
	 * Normalized serialization order:
	 *
	 * - id
	 * - type ordered names array(case insensitive/ascending)
	 * - issuer
	 * - issuanceDate
	 * - expirationDate
	 * + credentialSubject
	 *   - id
	 *   - properties ordered by name(case insensitive/ascending)
	 * + proof
	 *   - type
	 *   - method
	 *   - signature
	 */
	protected void toJson(JsonGenerator generator, DID ref, boolean normalized,
			boolean forSign) throws IOException {
		generator.writeStartObject();

		// id
		String value;
		generator.writeFieldName(Constants.id);

		if (normalized || ref == null || !getId().getDid().equals(ref))
			value = getId().toString();
		else
			value = "#" + getId().getFragment();

		generator.writeString(value);

		// type
		generator.writeFieldName(Constants.type);
		generator.writeStartArray();
		Collections.sort(types);
		for (String s : types) {
			generator.writeString(s);
		}
		generator.writeEndArray();

		// issuer
		if (normalized || !issuer.equals(subject.getId())) {
			generator.writeFieldName(Constants.issuer);
			generator.writeString(issuer.toString());
		}

		// issuanceDate
		generator.writeFieldName(Constants.issuanceDate);
		generator.writeString(JsonHelper.format(issuanceDate));

		// expirationDate
		if (expirationDate != null) {
			generator.writeFieldName(Constants.expirationDate);
			generator.writeString(JsonHelper.format(expirationDate));
		}

		// credentialSubject
		generator.writeFieldName(Constants.credentialSubject);
		subject.toJson(generator, ref, normalized);

		// proof
		if (!forSign ) {
			generator.writeFieldName(Constants.proof);
			proof.toJson(generator, issuer, normalized);
		}

		generator.writeEndObject();
	}

	public void toJson(Writer out, boolean normalized) throws IOException {
		toJson(out, normalized, false);
	}

	protected void toJson(Writer out, boolean normalized, boolean forSign) throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

		JsonFactory factory = new JsonFactory();
		JsonGenerator generator = factory.createGenerator(out);

		toJson(generator, null, normalized, forSign);

		generator.close();
	}

	public void toJson(OutputStream out, String charsetName, boolean normalized)
			throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

		toJson(new OutputStreamWriter(out, charsetName), normalized);
	}

	public void toJson(OutputStream out, boolean normalized) throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

		toJson(new OutputStreamWriter(out), normalized);
	}

	protected String toJson(boolean normalized, boolean forSign) {
		Writer out = new StringWriter(2048);

		try {
			toJson(out, normalized, forSign);
		} catch (IOException ignore) {
		}

		return out.toString();
	}

	public String toString(boolean normalized) {
		return toJson(normalized, false);
	}

	@Override
	public String toString() {
		return toString(false);
	}
}
