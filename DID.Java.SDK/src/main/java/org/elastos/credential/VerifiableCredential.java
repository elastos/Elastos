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
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDObject;
import org.elastos.did.DIDURL;
import org.elastos.did.MalformedCredentialException;
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

	static class StringComparator implements Comparator<String> {
		@Override
		public int compare(String key1, String key2) {
			return key1.compareToIgnoreCase(key2);
		}
	};

	static public class CredentialSubject {
		private DID id;
		private Map<String, String> properties;

		protected CredentialSubject(DID id) {
			this.id = id;
			properties = new TreeMap<String, String>(new StringComparator());
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

		static CredentialSubject fromJson(JsonNode node, DID ref)
				throws MalformedCredentialException {
			Class<MalformedCredentialException> clazz = MalformedCredentialException.class;

			// id
			DID id = JsonHelper.getDid(node, Constants.id, ref != null, ref,
					"crendentialSubject id", clazz);

			CredentialSubject cs = new CredentialSubject(id);

			if (node.size() <= 1) {
				System.out.println("Empty credentialSubject.");
				return cs;
			}

			Iterator<Map.Entry<String, JsonNode>> props = node.fields();
			Map.Entry<String, JsonNode> prop;
			while (props.hasNext()) {
				prop = props.next();

				if (prop.getKey().equals(Constants.id))
					continue;

				cs.addProperty(prop.getKey(), prop.getValue().asText());
			}

			return cs;
		}

		void toJson(JsonGenerator generator, DID ref, boolean compact)
				throws IOException {
			compact = (ref != null && compact);

			generator.writeStartObject();

			// id
			if (!compact || !getId().equals(ref)) {
				generator.writeFieldName(Constants.id);
				generator.writeString(getId().toExternalForm());
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

		Proof(String type, DIDURL method, String signature) {
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

		static Proof fromJson(JsonNode node, DID ref)
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

		void toJson(JsonGenerator generator, DID ref, boolean compact)
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

	public String[] getAllType() {
		return types == null ? null : (String[])types.toArray();
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

		// id
		DIDURL id = JsonHelper.getDidUrl(node, Constants.id,
				ref, "crendential id", clazz);
		setId(id);

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

		// IMPORTANT: help resolve full method in proof
		if (ref == null)
			ref = issuer;

		// proof
		valueNode = node.get(Constants.proof);
		if (valueNode == null)
			throw new MalformedCredentialException("Missing credential proof.");
		proof = Proof.fromJson(valueNode, ref);
	}

	public static VerifiableCredential fromJson(Reader reader)
			throws MalformedCredentialException {
		VerifiableCredential vc = new VerifiableCredential();
		vc.parse(reader);

		return vc;
	}

	public static VerifiableCredential fromJson(InputStream in)
			throws MalformedCredentialException {
		VerifiableCredential vc = new VerifiableCredential();
		vc.parse(in);

		return vc;
	}

	public static VerifiableCredential fromJson(String json)
			throws MalformedCredentialException {
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

	protected void toJson(JsonGenerator generator, DID ref, boolean compact)
			throws IOException {
		toJson(generator, ref, compact, false);
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
	protected void toJson(JsonGenerator generator, DID ref, boolean compact,
			boolean forSign) throws IOException {
		generator.writeStartObject();

		// id
		String value;
		generator.writeFieldName(Constants.id);
		if (compact && ref != null && getId().getDid().equals(ref))
			value = "#" + getId().getFragment();
		else
			value = getId().toExternalForm();
		generator.writeString(value);

		// type
		generator.writeFieldName(Constants.type);
		generator.writeStartArray();
		Collections.sort(types, new StringComparator());
		for (String s : types) {
			generator.writeString(s);
		}
		generator.writeEndArray();

		// issuer
		if (!compact || !issuer.equals(subject.getId())) {
			generator.writeFieldName(Constants.issuer);
			generator.writeString(issuer.toExternalForm());
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
		subject.toJson(generator, ref, compact);

		// proof
		if (!forSign ) {
			generator.writeFieldName(Constants.proof);
			proof.toJson(generator, issuer, compact);
		}

		generator.writeEndObject();
	}

	public void toJson(Writer out, boolean compact)
			throws IOException {
		toJson(out, compact, false);
	}

	protected void toJson(Writer out, boolean compact, boolean forSign) throws IOException {
		JsonFactory factory = new JsonFactory();
		JsonGenerator generator = factory.createGenerator(out);

		toJson(generator, null, compact, forSign);

		generator.close();
	}

	public void toJson(OutputStream out, String charsetName, boolean compact)
			throws IOException {
		toJson(new OutputStreamWriter(out, charsetName), compact);
	}

	public void toJson(OutputStream out, boolean compact) throws IOException {
		toJson(new OutputStreamWriter(out), compact);
	}

	protected String toJsonForSign(boolean compact) {
		Writer out = new StringWriter(2048);

		try {
			toJson(out, compact, true);
		} catch (IOException ignore) {
		}

		return out.toString();
	}

	public String toExternalForm(boolean compact) {
		Writer out = new StringWriter(2048);

		try {
			toJson(out, compact);
		} catch (IOException ignore) {
		}

		return out.toString();
	}

	@Override
	public String toString() {
		return toExternalForm(true);
	}
}
