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
import java.util.Comparator;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.elastos.credential.VerifiableCredential;
import org.elastos.did.util.HDKey;
import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class DIDDocument {
	private DID subject;
	private Map<DIDURL, PublicKey> publicKeys;
	private Map<DIDURL, PublicKey> authentications;
	private Map<DIDURL, PublicKey> authorizations;
	private Map<DIDURL, VerifiableCredential> credentials;
	private Map<DIDURL, Service> services;
	private Date expires;

	private boolean readonly;

	static class DIDURLComparator implements Comparator<DIDURL> {
		@Override
		public int compare(DIDURL id1, DIDURL id2) {
			return id1.toExternalForm().compareToIgnoreCase(
					id2.toExternalForm());
		}
	}

	/* Just expose from/toJson method for current package */
	static class EmbeddedCredential extends VerifiableCredential {
		protected EmbeddedCredential() {
			super();
		}

		protected EmbeddedCredential(VerifiableCredential vc) {
			super(vc);
		}

		protected static VerifiableCredential fromJson(JsonNode node, DID ref)
				throws MalformedCredentialException {
			return new EmbeddedCredential(VerifiableCredential.fromJson(node, ref));
		}

		@Override
		protected void toJson(JsonGenerator generator, DID ref, boolean compact)
				throws IOException {
			super.toJson(generator, ref, compact);
		}
	}

	DIDDocument() {
		readonly = false;
		setDefaultExpires();
	}

	private <K, V extends DIDObject> int getEntryCount(Map<K, V> entries) {
		if (entries == null || entries.isEmpty())
			return 0;

		return entries.size();
	}

	private <K, V extends DIDObject> List<V> getEntries(Map<K, V> entries) {
		List<V> lst = new ArrayList<V>(entries == null ? 0 : entries.size());

		if (entries != null && !entries.isEmpty())
			lst.addAll(entries.values());

		return lst;
	}

	private <K, V extends DIDObject> List<V> selectEntry(
			Map<K, V> entries, K id, String type) {
		List<V> lst = new ArrayList<V>(entries.size());

		if (entries == null || entries.isEmpty())
			return lst;

		for (V entry : entries.values()) {
			if (id != null) {
				if (!entry.getId().equals(id))
					continue;
			}

			if (type != null) {
				if (!entry.getType().equals(type))
					continue;
			}

			lst.add(entry);
		}

		return lst;
	}

	private <K, V extends DIDObject> V getEntry(Map<K, V> entries, K id) {
		if (entries == null || entries.isEmpty())
			return null;

		return entries.get(id);
	}

	private <K, V extends DIDObject> boolean removeEntry(Map<K, V> entries, K id) {
		if (entries == null || entries.isEmpty())
			return false;

		return entries.remove(id) != null;
	}

	public DID getSubject() {
		return subject;
	}

	void setSubject(DID subject) {
		this.subject = subject;
	}

	public int getPublicKeyCount() {
		return getEntryCount(publicKeys);
	}

	public List<PublicKey> getPublicKeys() {
		return getEntries(publicKeys);
	}

	public List<PublicKey> selectPublicKey(String id, String type)
			throws MalformedDIDURLException {
		return selectEntry(publicKeys, new DIDURL(id), type);
	}

	public List<PublicKey> selectPublicKey(DIDURL id, String type) {
		return selectEntry(publicKeys, id, type);
	}

	public PublicKey getPublicKey(String id) throws MalformedDIDURLException {
		return getEntry(publicKeys, new DIDURL(id));
	}

	public PublicKey getPublicKey(DIDURL id) {
		return getEntry(publicKeys, id);
	}

	public PublicKey getDefaultPublicKey() {
		DID self = getSubject();

		for (PublicKey pk : publicKeys.values()) {
			if (!pk.getController().equals(self))
				continue;

			String address = HDKey.DerivedKey.getAddress(
					pk.getPublicKeyBytes());
			if (address.equals(self.getMethodSpecificId()))
				return pk;
		}

		return null;
	}

	boolean addPublicKey(PublicKey pk) {
		if (readonly)
			return false;

		if (publicKeys == null)
			publicKeys = new TreeMap<DIDURL, PublicKey>(new DIDURLComparator());

		publicKeys.put(pk.getId(), pk);
		return true;
	}

	public boolean addPublicKey(String id) {
		// TODO: generate a new key pair, and add it to DIDDocument
		return false;
	}

	// TODO: Add an existing key to document!

	public boolean removePublicKey(DIDURL id) {
		if (readonly)
			return false;

		// Can not remove default public key
		if (getDefaultPublicKey().getId().equals(id))
			return false;

		boolean removed = removeEntry(publicKeys, id);
		if (removed) {
			// TODO: remove private key if exist.
		}

		return removed;
	}

	public int getAuthenticationKeyCount() {
		return getEntryCount(authentications);
	}

	public List<PublicKey> getAuthenticationKeys() {
		return getEntries(authentications);
	}

	public List<PublicKey> selectAuthenticationKey(DIDURL id, String type) {
		return selectEntry(authentications, id, type);
	}

	public List<PublicKey> selectAuthenticationKey(String id, String type)
			throws MalformedDIDURLException {
		return selectEntry(authentications, new DIDURL(id), type);
	}

	public PublicKey getAuthenticationKey(DIDURL id) {
		return getEntry(authentications, id);
	}

	public PublicKey getAuthenticationKey(String id)
			throws MalformedDIDURLException {
		return getEntry(authentications, new DIDURL(id));
	}

	boolean addAuthenticationKey(PublicKey pk) {
		if (readonly)
			return false;

		// Check the controller is current DID subject
		if (!pk.getController().equals(getSubject()))
			return false;

		if (authentications == null)
			authentications = new TreeMap<DIDURL, PublicKey>(
					new DIDURLComparator());

		authentications.put(pk.getId(), pk);
		return true;
	}

	public boolean addAuthenticationKey(DIDURL id) {
		if (readonly)
			return false;

		PublicKey pk = getPublicKey(id);
		if (pk == null)
			return false;

		return addAuthenticationKey(pk);
	}

	public boolean removeAuthenticationKey(DIDURL id) {
		if (readonly)
			return false;

		return removeEntry(authentications, id);
	}

	public int getAuthorizationKeyCount() {
		return getEntryCount(authorizations);
	}

	public List<PublicKey> getAuthorizationKeys() {
		return getEntries(authorizations);
	}

	public List<PublicKey> selectAuthorizationKey(DIDURL id, String type) {
		return selectEntry(authorizations, id, type);
	}

	public List<PublicKey> selectAuthorizationKey(String id, String type)
			throws MalformedDIDURLException {
		return selectEntry(authorizations, new DIDURL(id), type);
	}

	public PublicKey getAuthorizationKey(DIDURL id) {
		return getEntry(authorizations, id);
	}

	public PublicKey getAuthorizationKey(String id)
			throws MalformedDIDURLException {
		return getEntry(authorizations, new DIDURL(id));
	}

	boolean addAuthorizationKey(PublicKey pk) {
		if (readonly)
			return false;

		// Can not authorize to self
		if (pk.getController().equals(getSubject()))
			return false;

		if (authorizations == null)
			authorizations = new TreeMap<DIDURL, PublicKey>(
					new DIDURLComparator());

		authorizations.put(pk.getId(), pk);
		return true;
	}

	public boolean addAuthorizationKey(String id, DID did, DIDURL key) throws DIDException {
		if (readonly)
			return false;

		DIDDocument doc = DIDStore.getInstance().resolveDid(did);
		PublicKey refPk = doc.getPublicKey(key);
		if (refPk == null)
			return false;

		if (!refPk.getController().equals(did))
			return false;

		PublicKey pk = new PublicKey(new DIDURL(getSubject(), id),
				refPk.getType(), did, refPk.getPublicKeyBase58());

		addPublicKey(pk);

		return addAuthorizationKey(pk);
	}

	public boolean removeAuthorizationKey(DIDURL id) {
		if (readonly)
			return false;

		return removeEntry(authorizations, id);
	}

	public int getCredentialCount() {
		return getEntryCount(credentials);
	}

	public List<VerifiableCredential> getCredentials() {
		return getEntries(credentials);
	}

	public List<VerifiableCredential> selectCredential(DIDURL id, String type) {
		return selectEntry(credentials, id, type);
	}

	public List<VerifiableCredential> selectCredential(String id, String type)
			throws MalformedDIDURLException {
		return selectEntry(credentials, new DIDURL(id), type);
	}

	public VerifiableCredential getCredential(DIDURL id) {
		return getEntry(credentials, id);
	}

	public VerifiableCredential getCredential(String id)
			throws MalformedDIDURLException {
		return getEntry(credentials, new DIDURL(id));
	}

	public boolean addCredential(VerifiableCredential vc) {
		if (readonly)
			return false;

		// Check the credential belongs to current DID.
		if (!vc.getSubject().getId().equals(getSubject()))
			return false;

		if (credentials == null)
			credentials = new TreeMap<DIDURL, VerifiableCredential>(
					new DIDURLComparator());

		EmbeddedCredential ec = new EmbeddedCredential(vc);
		credentials.put(ec.getId(), ec);
		return true;
	}

	public boolean removeCredential(DIDURL id) {
		if (readonly)
			return false;

		return removeEntry(credentials, id);
	}

	public int getServiceCount() {
		return getEntryCount(services);
	}

	public List<Service> getServices() {
		return getEntries(services);
	}

	public List<Service> selectServices(DIDURL id, String type) {
		return selectEntry(services, id, type);
	}

	public List<Service> selectServices(String id, String type)
			throws MalformedDIDURLException {
		return selectEntry(services, new DIDURL(id), type);
	}

	public Service getService(DIDURL id) {
		return getEntry(services, id);
	}

	public Service getService(String id) throws MalformedDIDURLException {
		return getEntry(services, new DIDURL(id));
	}

	public boolean addService(String id, String type, String endpoint) {
		DIDURL idUrl = new DIDURL(getSubject(), id);
		Service svc = new Service(idUrl, type, endpoint);
		return addService(svc);
	}

	boolean addService(Service svc) {
		if (readonly)
			return false;

		if (services == null)
			services = new TreeMap<DIDURL, Service>(new DIDURLComparator());

		services.put(svc.getId(), svc);
		return true;
	}

	public boolean removeService(DIDURL id) {
		if (readonly)
			return false;

		return removeEntry(services, id);
	}

	private Calendar getMaxExpires() {
		Calendar cal = Calendar.getInstance(Constants.UTC);
		cal.add(Calendar.YEAR, Constants.MAX_VALID_YEARS);
		cal.set(Calendar.MINUTE, 0);
		cal.set(Calendar.SECOND, 0);
		cal.set(Calendar.MILLISECOND, 0);
		return cal;
	}

	public void setDefaultExpires() {
		expires = getMaxExpires().getTime();
	}

	public boolean setExpires(Date expires) {
		if (readonly)
			return false;

		Calendar cal = Calendar.getInstance(Constants.UTC);
		cal.setTime(expires);
		cal.set(Calendar.MINUTE, 0);
		cal.set(Calendar.SECOND, 0);
		cal.set(Calendar.MILLISECOND, 0);

		if (cal.after(getMaxExpires()))
			return false;

		this.expires = cal.getTime();
		return true;
	}

	public Date getExpires() {
		return expires;
	}

	void setReadonly(boolean readonly) {
		this.readonly = readonly;
	}

	public boolean isReadonly() {
		return readonly;
	}

	public boolean modify() {
		// TODO: Check owner

		setReadonly(false);
		return true;
	}

	private void parse(Reader reader) throws MalformedDocumentException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(reader);
			parse(node);
		} catch (IOException e) {
			throw new MalformedDocumentException("Parse JSON document error.", e);
		}
	}

	private void parse(InputStream in) throws MalformedDocumentException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(in);
			parse(node);
		} catch (IOException e) {
			throw new MalformedDocumentException("Parse JSON document error.", e);
		}
	}

	private void parse(String json) throws MalformedDocumentException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(json);
			parse(node);
		} catch (IOException e) {
			throw new MalformedDocumentException("Parse JSON document error.", e);
		}
	}

	private void parse(JsonNode doc) throws MalformedDocumentException {
		Class<MalformedDocumentException> clazz = MalformedDocumentException.class;

		setSubject(JsonHelper.getDid(doc, Constants.id,
				false, null, "subject", clazz));

		JsonNode node = doc.get(Constants.publicKey);
		if (node == null)
			throw new MalformedDocumentException("Missing publicKey.");
		parsePublicKey(node);

		node = doc.get(Constants.authentication);
		if (node != null)
			parseAuthentication(node);

		node = doc.get(Constants.authorization);
		if (node != null)
			parseAuthorization(node);

		node = doc.get(Constants.credential);
		if (node != null)
			parseCredential(node);

		node = doc.get(Constants.service);
		if (node != null)
			parseService(node);

		expires = JsonHelper.getDate(doc, Constants.expires,
				true, null, "expires", clazz);
	}

	private void parsePublicKey(JsonNode node)
			throws MalformedDocumentException {
		if (!node.isArray())
			throw new MalformedDocumentException(
					"Invalid publicKey, should be an array.");

		if (node.size() == 0)
			throw new MalformedDocumentException(
					"Invalid publicKey, should not be an empty array.");

		for (int i = 0; i < node.size(); i++) {
			PublicKey pk = PublicKey.fromJson(node.get(i), getSubject());
			addPublicKey(pk);
		}
	}

	private void parseAuthentication(JsonNode node)
			throws MalformedDocumentException {
		if (!node.isArray())
			throw new MalformedDocumentException(
					"Invalid authentication, should be an array.");

		if (node.size() == 0)
			return;

		PublicKey pk;

		for (int i = 0; i < node.size(); i++) {
			JsonNode keyNode = node.get(i);
			if (keyNode.isObject()) {
				pk = PublicKey.fromJson(keyNode, getSubject());
				addPublicKey(pk);
			} else {
				DIDURL id = JsonHelper.getDidUrl(keyNode, getSubject(),
						"authentication publicKey id",
						MalformedDocumentException.class);

				pk = publicKeys.get(id);
				if (pk == null) {
					System.out.println("Unknown authentication publickey: " + id);
					continue;
				}
			}

			addAuthenticationKey(pk);
		}
	}

	private void parseAuthorization(JsonNode node)
			throws MalformedDocumentException {
		if (!node.isArray())
			throw new MalformedDocumentException(
					"Invalid authorization, should be an array.");

		if (node.size() == 0)
			return;

		PublicKey pk;

		for (int i = 0; i < node.size(); i++) {
			JsonNode keyNode = node.get(i);
			if (keyNode.isObject()) {
				pk = PublicKey.fromJson(keyNode, getSubject());
				publicKeys.put(pk.getId(), pk);
			} else {
				DIDURL id = JsonHelper.getDidUrl(keyNode, getSubject(),
						"authorization publicKey id",
						MalformedDocumentException.class);

				pk = publicKeys.get(id);
				if (pk == null) {
					System.out.println("Unknown authorization publickey: " + id);
					continue;
				}
			}

			addAuthorizationKey(pk);
		}
	}

	private void parseCredential(JsonNode node)
			throws MalformedDocumentException {
		if (!node.isArray())
			throw new MalformedDocumentException(
					"Invalid credential, should be an array.");

		if (node.size() == 0)
			return;

		for (int i = 0; i < node.size(); i++) {
			VerifiableCredential vc;
			try {
				vc = EmbeddedCredential.fromJson(node.get(i), getSubject());
			} catch (MalformedCredentialException e) {
				throw new MalformedDocumentException(e.getMessage(), e);
			}

			addCredential(vc);
		}
	}

	private void parseService(JsonNode node) throws MalformedDocumentException {
		if (!node.isArray())
			throw new MalformedDocumentException(
					"Invalid service, should be an array.");

		if (node.size() == 0)
			return;

		for (int i = 0; i < node.size(); i++) {
			Service svc = Service.fromJson(node.get(i), getSubject());
			addService(svc);
		}
	}

	public static DIDDocument fromJson(Reader reader)
			throws MalformedDocumentException {
		DIDDocument doc = new DIDDocument();
		doc.parse(reader);
		doc.setReadonly(true);

		return doc;
	}

	public static DIDDocument fromJson(InputStream in)
			throws MalformedDocumentException {
		DIDDocument doc = new DIDDocument();
		doc.parse(in);
		doc.setReadonly(true);

		return doc;
	}

	public static DIDDocument fromJson(String json)
			throws MalformedDocumentException {
		DIDDocument doc = new DIDDocument();
		doc.parse(json);
		doc.setReadonly(true);

		return doc;
	}

	/*
	 * Normalized serialization order:
	 *
	 * - id
	 * + publickey
	 *   + public keys array ordered by id(case insensitive/ascending)
	 *     - id
	 *     - type
	 *     - controller
	 *     - publicKeyBase58
	 * + authentication
	 *   - ordered by public key' ids(case insensitive/ascending)
	 * + authorization
	 *   - ordered by public key' ids(case insensitive/ascending)
	 * + verifiableCredential
	 *   - credentials array ordered by id(case insensitive/ascending)
	 * + service
	 *   + services array ordered by id(case insensitive/ascending)
	 *     - id
	 *     - type
	 *     - endpoint
	 * - expires
	 */
	public void toJson(Writer out, boolean compact) throws IOException {
		JsonFactory factory = new JsonFactory();
		JsonGenerator generator = factory.createGenerator(out);

		generator.writeStartObject();

		// subject
		generator.writeFieldName(Constants.id);
		generator.writeString(getSubject().toExternalForm());

		// publicKey
		generator.writeFieldName(Constants.publicKey);
		generator.writeStartArray();
		for (PublicKey pk : publicKeys.values())
			pk.toJson(generator, getSubject(), compact);
		generator.writeEndArray();

		// authentication
		generator.writeFieldName(Constants.authentication);
		generator.writeStartArray();
		for (PublicKey pk : authentications.values()) {
			String value;

			if (compact && pk.getId().getDid().equals(getSubject()))
				value = "#" + pk.getId().getFragment();
			else
				value = pk.getId().toExternalForm();

			generator.writeString(value);
		}
		generator.writeEndArray();

		// authorization
		if (authorizations != null && authorizations.size() != 0) {
			generator.writeFieldName(Constants.authorization);
			generator.writeStartArray();
			for (PublicKey pk : authorizations.values()) {
				String value;

				if (compact && pk.getId().getDid().equals(getSubject()))
					value = "#" + pk.getId().getFragment();
				else
					value = pk.getId().toExternalForm();

				generator.writeString(value);
			}
			generator.writeEndArray();
		}

		// credential
		if (credentials != null && credentials.size() != 0) {
			generator.writeFieldName(Constants.credential);
			generator.writeStartArray();
			for (VerifiableCredential vc : credentials.values())
				((EmbeddedCredential)vc).toJson(generator, getSubject(), compact);
			generator.writeEndArray();
		}

		// service
		if (services != null && services.size() != 0) {
			generator.writeFieldName(Constants.service);
			generator.writeStartArray();
			for (Service svc : services.values())
				svc.toJson(generator, getSubject(), compact);
			generator.writeEndArray();
		}

		// expires
		if (expires != null) {
			generator.writeFieldName(Constants.expires);
			generator.writeString(JsonHelper.format(expires));
		}

		generator.writeEndObject();
		generator.close();
	}

	public void toJson(OutputStream out, String charsetName, boolean compact)
			throws IOException {
		toJson(new OutputStreamWriter(out, charsetName), compact);
	}

	public void toJson(OutputStream out, boolean compact) throws IOException {
		toJson(new OutputStreamWriter(out), compact);
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
