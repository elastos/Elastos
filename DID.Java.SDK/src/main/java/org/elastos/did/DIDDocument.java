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
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.MalformedCredentialException;
import org.elastos.did.exception.MalformedDIDException;
import org.elastos.did.exception.MalformedDIDURLException;
import org.elastos.did.exception.MalformedDocumentException;
import org.elastos.did.meta.DIDMeta;
import org.elastos.did.util.Base58;
import org.elastos.did.util.Base64;
import org.elastos.did.util.EcdsaSigner;
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
	private Proof proof;

	private DIDMeta meta;

	public static class PublicKey extends DIDObject {
		private DID controller;
		private String keyBase58;

		protected PublicKey(DIDURL id, String type, DID controller, String keyBase58) {
			super(id, type);
			this.controller = controller;
			this.keyBase58 = keyBase58;
		}

		protected PublicKey(DIDURL id, DID controller, String keyBase58) {
			this(id, Constants.defaultPublicKeyType, controller, keyBase58);
		}

		public DID getController() {
			return controller;
		}

		public String getPublicKeyBase58() {
			return keyBase58;
		}

		public byte[] getPublicKeyBytes() {
			return Base58.decode(keyBase58);
		}

		@Override
		public boolean equals(Object obj) {
			if (this == obj)
				return true;

			if (obj instanceof PublicKey) {
				PublicKey ref = (PublicKey)obj;

				if (getId().equals(ref.getId()) &&
						getType().equals(ref.getType()) &&
						getController().equals(ref.getController()) &&
						getPublicKeyBase58().equals(ref.getPublicKeyBase58()))
					return true;
			}

			return false;
		}

		protected static PublicKey fromJson(JsonNode node, DID ref)
				throws MalformedDocumentException {
			Class<MalformedDocumentException> clazz = MalformedDocumentException.class;

			DIDURL id = JsonHelper.getDidUrl(node, Constants.id,
						ref, "publicKey' id", clazz);

			String type = JsonHelper.getString(node, Constants.type, true,
						Constants.defaultPublicKeyType, "publicKey' type", clazz);

			DID controller = JsonHelper.getDid(node, Constants.controller,
						true, ref, "publicKey' controller", clazz);

			String keyBase58 = JsonHelper.getString(node, Constants.publicKeyBase58,
						false, null, "publicKeyBase58", clazz);

			return new PublicKey(id, type, controller, keyBase58);
		}

		protected void toJson(JsonGenerator generator, DID ref, boolean normalized)
				throws IOException {
			String value;

			generator.writeStartObject();

			// id
			generator.writeFieldName(Constants.id);
			if (normalized || ref == null || !getId().getDid().equals(ref))
				value = getId().toString();
			else
				value = "#" + getId().getFragment();
			generator.writeString(value);

			// type
			if (normalized || !getType().equals(Constants.defaultPublicKeyType)) {
				generator.writeFieldName(Constants.type);
				generator.writeString(getType());
			}

			// controller
			if (normalized || ref == null || !controller.equals(ref)) {
				generator.writeFieldName(Constants.controller);
				generator.writeString(controller.toString());
			}

			// publicKeyBase58
			generator.writeFieldName(Constants.publicKeyBase58);
			generator.writeString(keyBase58);

			generator.writeEndObject();
		}
	}

	public static class Service extends DIDObject {
		private String endpoint;

		protected Service(DIDURL id, String type, String endpoint) {
			super(id, type);
			this.endpoint = endpoint;
		}

		public String getServiceEndpoint() {
			return endpoint;
		}

		protected static Service fromJson(JsonNode node, DID ref)
				throws MalformedDocumentException {
			Class<MalformedDocumentException> clazz = MalformedDocumentException.class;

			DIDURL id = JsonHelper.getDidUrl(node, Constants.id,
					ref, "service' id", clazz);

			String type = JsonHelper.getString(node, Constants.type, false,
					null, "service' type", clazz);

			String endpoint = JsonHelper.getString(node, Constants.serviceEndpoint,
					false, null, "service' endpoint", clazz);

			return new Service(id, type, endpoint);
		}

		public void toJson(JsonGenerator generator, DID ref, boolean normalized)
				throws IOException {
			String value;

			generator.writeStartObject();

			// id
			generator.writeFieldName(Constants.id);
			if (normalized || ref == null || !getId().getDid().equals(ref))
				value = getId().toString();
			else
				value = "#" + getId().getFragment();
			generator.writeString(value);

			// type
			generator.writeFieldName(Constants.type);
			generator.writeString(getType());

			// endpoint
			generator.writeFieldName(Constants.serviceEndpoint);
			generator.writeString(endpoint);

			generator.writeEndObject();
		}
	}

	public static class Proof {
		private String type;
		private Date created;
		private DIDURL creator;
		private String signature;

		protected Proof(String type, Date created, DIDURL creator, String signature) {
			this.type = type;
			this.created = created;
			this.creator = creator;
			this.signature = signature;
		}

		protected Proof(DIDURL creator, String signature) {
			this(Constants.defaultPublicKeyType,
					Calendar.getInstance(Constants.UTC).getTime(),
					creator, signature);
		}

	    public String getType() {
	    	return type;
	    }

	    public Date getCreated() {
	    	return created;
	    }

	    public DIDURL getCreator() {
	    	return creator;
	    }

	    public String getSignature() {
	    	return signature;
	    }

		protected static Proof fromJson(JsonNode node, DIDURL refSignKey)
				throws MalformedDocumentException {
			Class<MalformedDocumentException> clazz = MalformedDocumentException.class;

			String type = JsonHelper.getString(node, Constants.type,
					true, Constants.defaultPublicKeyType,
					"document proof type", clazz);

			Date created = JsonHelper.getDate(node, Constants.created,
					true, null, "proof created date", clazz);

			DIDURL creator = JsonHelper.getDidUrl(node, Constants.creator, true,
					refSignKey.getDid(), "document proof creator", clazz);
			if (creator == null)
				creator = refSignKey;

			String signature = JsonHelper.getString(node, Constants.signatureValue,
					false, null, "document proof signature", clazz);

			return new Proof(type, created, creator, signature);
		}

		protected void toJson(JsonGenerator generator, boolean normalized)
				throws IOException {
			generator.writeStartObject();

			// type
			if (normalized || !type.equals(Constants.defaultPublicKeyType)) {
				generator.writeFieldName(Constants.type);
				generator.writeString(type);
			}

			// created
			if (created != null) {
				generator.writeFieldName(Constants.created);
				generator.writeString(JsonHelper.format(created));
			}

			// creator
			if (normalized) {
				generator.writeFieldName(Constants.creator);
				generator.writeString(creator.toString());
			}

			// signature
			generator.writeFieldName(Constants.signatureValue);
			generator.writeString(signature);

			generator.writeEndObject();
		}
	}

	private DIDDocument() {
	}

	protected DIDDocument(DID subject) {
		this();
		this.subject = subject;
	}

	protected DIDDocument(DIDDocument doc) {
		this();

		// Copy constructor
		this.subject = doc.subject;

		if (doc.publicKeys != null)
			this.publicKeys = new TreeMap<DIDURL, PublicKey>(doc.publicKeys);

		if (doc.authentications != null)
			this.authentications = new TreeMap<DIDURL, PublicKey>(doc.authentications);

		if (doc.authorizations != null)
			this.authorizations = new TreeMap<DIDURL, PublicKey>(doc.authorizations);

		if (doc.credentials != null)
			this.credentials = new TreeMap<DIDURL, VerifiableCredential>(doc.credentials);

		if (doc.services != null)
			this.services = new TreeMap<DIDURL, Service>(doc.services);

		this.expires = doc.expires;
		this.proof = doc.proof;
		this.meta = doc.meta;
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
				// Credential's type is a list.
				if (entry instanceof VerifiableCredential) {
					VerifiableCredential vc = (VerifiableCredential)entry;

					if (!Arrays.asList(vc.getTypes()).contains(type))
						continue;
				} else {
					if (!entry.getType().equals(type))
						continue;
				}
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

	private void setSubject(DID subject) {
		this.subject = subject;
	}

	public int getPublicKeyCount() {
		return getEntryCount(publicKeys);
	}

	public List<PublicKey> getPublicKeys() {
		return getEntries(publicKeys);
	}

	public List<PublicKey> selectPublicKeys(String id, String type)
			throws MalformedDIDURLException {
		return selectEntry(publicKeys, new DIDURL(getSubject(), id), type);
	}

	public List<PublicKey> selectPublicKeys(DIDURL id, String type) {
		if (id == null && type == null)
			throw new IllegalArgumentException();

		return selectEntry(publicKeys, id, type);
	}

	public PublicKey getPublicKey(String id) throws MalformedDIDURLException {
		return getEntry(publicKeys, new DIDURL(getSubject(), id));
	}

	public PublicKey getPublicKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return getEntry(publicKeys, id);
	}

	public boolean hasPublicKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return getEntry(publicKeys, id) != null;
	}

	public boolean hasPublicKey(String id) throws MalformedDIDURLException {
		return hasPublicKey(new DIDURL(getSubject(), id));
	}

	public boolean hasPrivateKey(DIDURL id) throws DIDStoreException {
		if (id == null)
			throw new IllegalArgumentException();

		if (getEntry(publicKeys, id) == null)
			return false;

		if (!getMeta().attachedStore())
			return false;

		return getMeta().getStore().containsPrivateKey(getSubject(), id);
	}

	public boolean hasPrivateKey(String id)
			throws MalformedDIDURLException, DIDStoreException {
		return hasPrivateKey(new DIDURL(getSubject(), id));
	}

	public DIDURL getDefaultPublicKey() {
		DID self = getSubject();

		for (PublicKey pk : publicKeys.values()) {
			if (!pk.getController().equals(self))
				continue;

			String address = HDKey.DerivedKey.getAddress(
					pk.getPublicKeyBytes());
			if (address.equals(self.getMethodSpecificId()))
				return pk.getId();
		}

		return null;
	}

	protected boolean addPublicKey(PublicKey pk) {
		if (publicKeys == null) {
			publicKeys = new TreeMap<DIDURL, PublicKey>();
		} else {
			// Check the existence, both id and keyBase58
			for (PublicKey key : publicKeys.values()) {
				if (key.getId().equals(pk.getId()))
					return false;

				if (key.getPublicKeyBase58().equals(pk.getPublicKeyBase58()))
					return false;
			}
		}

		publicKeys.put(pk.getId(), pk);
		return true;
	}

	protected boolean removePublicKey(DIDURL id, boolean force) {
		// Can not remove default public key
		if (getDefaultPublicKey().equals(id))
			return false;

		if (force) {
			removeAuthenticationKey(id);
			removeAuthorizationKey(id);
		} else {
			if (isAuthenticationKey(id) || isAuthorizationKey(id))
				return false;
		}

		boolean removed = removeEntry(publicKeys, id);
		if (removed) {
			try {
				if (getMeta().attachedStore())
					getMeta().getStore().deletePrivateKey(getSubject(), id);
			} catch (DIDStoreException ignore) {
				// TODO: CHECKME!
			}
		}

		return removed;
	}

	public int getAuthenticationKeyCount() {
		return getEntryCount(authentications);
	}

	public List<PublicKey> getAuthenticationKeys() {
		return getEntries(authentications);
	}

	public List<PublicKey> selectAuthenticationKeys(DIDURL id, String type) {
		if (id == null && type == null)
			throw new IllegalArgumentException();

		return selectEntry(authentications, id, type);
	}

	public List<PublicKey> selectAuthenticationKeys(String id, String type)
			throws MalformedDIDURLException {
		return selectEntry(authentications, new DIDURL(getSubject(), id), type);
	}

	public PublicKey getAuthenticationKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return getEntry(authentications, id);
	}

	public PublicKey getAuthenticationKey(String id)
			throws MalformedDIDURLException {
		return getEntry(authentications, new DIDURL(getSubject(), id));
	}

	public boolean isAuthenticationKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return getAuthenticationKey(id) != null;
	}

	public boolean isAuthenticationKey(String id)
			throws MalformedDIDURLException {
		return getAuthenticationKey(id) != null;
	}

	protected boolean addAuthenticationKey(PublicKey pk) {
		// Check the controller is current DID subject
		if (!pk.getController().equals(getSubject()))
			return false;

		PublicKey key = getPublicKey(pk.getId());
		if (key == null) {
			// Add the new pk to PublicKeys if not exist.
			addPublicKey(pk);
		} else {
			if (!key.equals(pk)) // Key conflict.
				return false;
			else // Already has this key.
				pk = key;
		}

		if (authentications == null) {
			authentications = new TreeMap<DIDURL, PublicKey>();
		} else {
			if (authentications.containsKey(pk.getId()))
				return false;
		}

		authentications.put(pk.getId(), pk);
		return true;
	}

	protected boolean removeAuthenticationKey(DIDURL id) {
		// Can not remove default public key
		if (getDefaultPublicKey().equals(id))
			return false;

		return removeEntry(authentications, id);
	}

	public int getAuthorizationKeyCount() {
		return getEntryCount(authorizations);
	}

	public List<PublicKey> getAuthorizationKeys() {
		return getEntries(authorizations);
	}

	public List<PublicKey> selectAuthorizationKeys(DIDURL id, String type) {
		if (id == null && type == null)
			throw new IllegalArgumentException();

		return selectEntry(authorizations, id, type);
	}

	public List<PublicKey> selectAuthorizationKeys(String id, String type)
			throws MalformedDIDURLException {
		return selectEntry(authorizations, new DIDURL(getSubject(), id), type);
	}

	public PublicKey getAuthorizationKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return getEntry(authorizations, id);
	}

	public PublicKey getAuthorizationKey(String id)
			throws MalformedDIDURLException {
		return getEntry(authorizations, new DIDURL(getSubject(), id));
	}

	public boolean isAuthorizationKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return getAuthorizationKey(id) != null;
	}

	public boolean isAuthorizationKey(String id)
			throws MalformedDIDURLException {
		return isAuthorizationKey(new DIDURL(getSubject(), id));
	}

	protected boolean addAuthorizationKey(PublicKey pk) {
		// Can not authorize to self
		if (pk.getController().equals(getSubject()))
			return false;

		PublicKey key = getPublicKey(pk.getId());
		if (key == null) {
			// Add the new pk to PublicKeys if not exist.
			addPublicKey(pk);
		} else {
			if (!key.equals(pk)) // Key conflict.
				return false;
			else // Already has this key.
				pk = key;
		}

		if (authorizations == null) {
			authorizations = new TreeMap<DIDURL, PublicKey>();
		} else {
			if (authorizations.containsKey(pk.getId()))
				return false;
		}

		authorizations.put(pk.getId(), pk);
		return true;
	}

	protected boolean removeAuthorizationKey(DIDURL id) {
		return removeEntry(authorizations, id);
	}

	public int getCredentialCount() {
		return getEntryCount(credentials);
	}

	public List<VerifiableCredential> getCredentials() {
		return getEntries(credentials);
	}

	public List<VerifiableCredential> selectCredentials(DIDURL id, String type) {
		if (id == null && type == null)
			throw new IllegalArgumentException();

		return selectEntry(credentials, id, type);
	}

	public List<VerifiableCredential> selectCredentials(String id, String type)
			throws MalformedDIDURLException {
		return selectEntry(credentials, new DIDURL(getSubject(), id), type);
	}

	public VerifiableCredential getCredential(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return getEntry(credentials, id);
	}

	public VerifiableCredential getCredential(String id)
			throws MalformedDIDURLException {
		return getEntry(credentials, new DIDURL(getSubject(), id));
	}

	protected boolean addCredential(VerifiableCredential vc) {
		// Check the credential belongs to current DID.
		if (!vc.getSubject().getId().equals(getSubject()))
			return false;

		if (credentials == null) {
			credentials = new TreeMap<DIDURL, VerifiableCredential>();
		} else {
			if (credentials.containsKey(vc.getId()))
				return false;
		}

		credentials.put(vc.getId(), vc);
		return true;
	}

	protected boolean removeCredential(DIDURL id) {
		return removeEntry(credentials, id);
	}

	public int getServiceCount() {
		return getEntryCount(services);
	}

	public List<Service> getServices() {
		return getEntries(services);
	}

	public List<Service> selectServices(DIDURL id, String type) {
		if (id == null && type == null)
			throw new IllegalArgumentException();

		return selectEntry(services, id, type);
	}

	public List<Service> selectServices(String id, String type)
			throws MalformedDIDURLException {
		return selectEntry(services, new DIDURL(getSubject(), id), type);
	}

	public Service getService(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return getEntry(services, id);
	}

	public Service getService(String id) throws MalformedDIDURLException {
		return getEntry(services, new DIDURL(getSubject(), id));
	}

	protected boolean addService(Service svc) {
		if (services == null)
			services = new TreeMap<DIDURL, Service>();
		else {
			if (services.containsKey(svc.getId()))
				return false;
		}

		services.put(svc.getId(), svc);
		return true;
	}

	protected boolean removeService(DIDURL id) {
		return removeEntry(services, id);
	}

	public Date getExpires() {
		return expires;
	}

	protected void setExpires(Date expires) {
		this.expires = expires;
	}

	public Proof getProof() {
		return proof;
	}

	private void setProof(Proof proof) {
		this.proof = proof;
	}

	protected void setMeta(DIDMeta meta) {
		this.meta = meta;
	}

	protected DIDMeta getMeta() {
		if (meta == null)
			meta = new DIDMeta();

		return meta;
	}

	public void setExtra(String name, String value) throws DIDStoreException {
		if (name == null || name.isEmpty())
			throw new IllegalArgumentException();

		getMeta().setExtra(name, value);

		if (getMeta().attachedStore())
			getMeta().getStore().storeDidMeta(getSubject(), meta);
	}

	public String getExtra(String name) {
		if (name == null || name.isEmpty())
			throw new IllegalArgumentException();

		return getMeta().getExtra(name);
	}

	public void setAlias(String alias) throws DIDStoreException {
		getMeta().setAlias(alias);

		if (getMeta().attachedStore())
			getMeta().getStore().storeDidMeta(getSubject(), meta);
	}

	public String getAlias() {
		return getMeta().getAlias();
	}

	public String getTransactionId() {
		return getMeta().getTransactionId();
	}

	public Date getUpdated() {
		return getMeta().getUpdated();
	}

	public boolean isDeactivated() throws DIDException {
		return getMeta().isDeactivated();
	}

	public boolean isExpired() {
		Calendar now = Calendar.getInstance(Constants.UTC);

		Calendar expireDate  = Calendar.getInstance(Constants.UTC);
		expireDate.setTime(expires);

		return now.after(expireDate);
	}

	public boolean isGenuine() {
		// Document should signed(only) by default public key.
		if (!proof.getCreator().equals(getDefaultPublicKey()))
			return false;

		// Unsupported public key type;
		if (!proof.getType().equals(Constants.defaultPublicKeyType))
			return false;

		String json = toJson(true, true);
		return verify(proof.getCreator(), proof.getSignature(), json.getBytes());
	}

	public boolean isValid() throws DIDException {
		return !isDeactivated() && !isExpired() && isGenuine();
	}

	public Builder edit() {
		return new Builder(this);
	}

	public String sign(String storepass, byte[] ... data)
			throws DIDStoreException {
		DIDURL key = getDefaultPublicKey();
		return sign(key, storepass, data);
	}

	public String sign(DIDURL id, String storepass, byte[] ... data)
			throws DIDStoreException {
		if (id == null || data == null ||
				storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException();

		if (!getMeta().attachedStore())
			throw new DIDStoreException("Not attached with DID store.");

		return getMeta().getStore().sign(getSubject(), id, storepass, data);
	}

	public String sign(String id, String storepass, byte[] ... data)
			throws MalformedDIDURLException, DIDStoreException {
		return sign(new DIDURL(getSubject(), id), storepass, data);
	}

	public boolean verify(String signature, byte[] ... data)
			throws DIDException {
		DIDURL key = getDefaultPublicKey();
		return verify(key, signature, data);
	}

	public boolean verify(String id, String signature, byte[] ... data)
			throws MalformedDIDURLException, DIDException {
		return verify(new DIDURL(getSubject(), id), signature, data);
	}

	public boolean verify(DIDURL id, String signature, byte[] ... data) {
		if (id == null || signature == null || signature.isEmpty() || data == null)
			throw new IllegalArgumentException();

		PublicKey pk = getPublicKey(id);
		byte[] binkey = pk.getPublicKeyBytes();
		byte[] sig = Base64.decode(signature,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);

		return EcdsaSigner.verify(binkey, sig, data);
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

		DIDURL defaultPk = getDefaultPublicKey();
		if (defaultPk == null)
			throw new MalformedDocumentException("Missing default publicKey.");

		// Add default public key to authentication keys if needed.
		if (isAuthenticationKey(defaultPk))
			addAuthenticationKey(getPublicKey(defaultPk));

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

		node = doc.get(Constants.proof);
		if (node == null)
			throw new MalformedDocumentException("Missing proof.");
		setProof(Proof.fromJson(node, defaultPk));
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
				vc = VerifiableCredential.fromJson(node.get(i), getSubject());
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
		if (reader == null)
			throw new IllegalArgumentException();

		DIDDocument doc = new DIDDocument();
		doc.parse(reader);

		return doc;
	}

	public static DIDDocument fromJson(InputStream in)
			throws MalformedDocumentException {
		if (in == null)
			throw new IllegalArgumentException();

		DIDDocument doc = new DIDDocument();
		doc.parse(in);

		return doc;
	}

	public static DIDDocument fromJson(String json)
			throws MalformedDocumentException {
		if (json == null || json.isEmpty())
			throw new IllegalArgumentException();

		DIDDocument doc = new DIDDocument();
		doc.parse(json);

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
	 * + proof
	 *   - type
	 *   - created
	 *   - creator
	 *   - signatureValue
	 */
	protected void toJson(Writer out, boolean normalized, boolean forSign)
			throws IOException {
		JsonFactory factory = new JsonFactory();
		JsonGenerator generator = factory.createGenerator(out);

		generator.writeStartObject();

		// subject
		generator.writeFieldName(Constants.id);
		generator.writeString(getSubject().toString());

		// publicKey
		generator.writeFieldName(Constants.publicKey);
		generator.writeStartArray();
		for (PublicKey pk : publicKeys.values())
			pk.toJson(generator, getSubject(), normalized);
		generator.writeEndArray();

		// authentication
		generator.writeFieldName(Constants.authentication);
		generator.writeStartArray();
		for (PublicKey pk : authentications.values()) {
			String value;

			if (normalized || !pk.getId().getDid().equals(getSubject()))
				value = pk.getId().toString();
			else
				value = "#" + pk.getId().getFragment();

			generator.writeString(value);
		}
		generator.writeEndArray();

		// authorization
		if (authorizations != null && authorizations.size() != 0) {
			generator.writeFieldName(Constants.authorization);
			generator.writeStartArray();
			for (PublicKey pk : authorizations.values()) {
				String value;

				if (normalized || !pk.getId().getDid().equals(getSubject()))
					value = pk.getId().toString();
				else
					value = "#" + pk.getId().getFragment();

				generator.writeString(value);
			}
			generator.writeEndArray();
		}

		// credential
		if (credentials != null && credentials.size() != 0) {
			generator.writeFieldName(Constants.credential);
			generator.writeStartArray();
			for (VerifiableCredential vc : credentials.values())
				vc.toJson(generator, getSubject(), normalized);
			generator.writeEndArray();
		}

		// service
		if (services != null && services.size() != 0) {
			generator.writeFieldName(Constants.service);
			generator.writeStartArray();
			for (Service svc : services.values())
				svc.toJson(generator, getSubject(), normalized);
			generator.writeEndArray();
		}

		// expires
		if (expires != null) {
			generator.writeFieldName(Constants.expires);
			generator.writeString(JsonHelper.format(expires));
		}

		// proof
		if (proof != null && !forSign) {
			generator.writeFieldName(Constants.proof);
			proof.toJson(generator, normalized);
		}

		generator.writeEndObject();
		generator.close();
	}

	public void toJson(Writer out, boolean normalized)
			throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

		toJson(out, normalized, false);
	}

	public void toJson(OutputStream out, String charsetName, boolean normalized)
			throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

		if (charsetName == null)
			charsetName = "UTF-8";

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

	public static class Builder {
		private DIDDocument document;

		protected Builder(DID did, DIDStore store) {
			this.document = new DIDDocument(did);
			this.document.getMeta().setStore(store);
		}

		protected Builder(DIDDocument doc) {
			// Make a copy.
			this.document = new DIDDocument(doc);
		}

		public DID getSubject() {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			return document.getSubject();
		}

		public boolean addPublicKey(DIDURL id, DID controller, String pk) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || controller == null || pk == null)
				throw new IllegalArgumentException();

			if ( Base58.decode(pk).length != HDKey.PUBLICKEY_BYTES)
				throw new IllegalArgumentException("Invalid public key.");

			PublicKey key = new PublicKey(id, controller, pk);
			return document.addPublicKey(key);
		}

		public boolean addPublicKey(String id, String controller, String pk)
				throws MalformedDIDURLException, MalformedDIDException {
			return addPublicKey(new DIDURL(getSubject(), id),
					new DID(controller), pk);
		}

		public boolean removePublicKey(DIDURL id, boolean force) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			return document.removePublicKey(id, force);
		}

		public boolean removePublicKey(String id, boolean force)
				throws MalformedDIDURLException {
			return removePublicKey(new DIDURL(getSubject(), id), force);
		}

		public boolean removePublicKey(DIDURL id) {
			return removePublicKey(id, false);
		}

		public boolean removePublicKey(String id) throws MalformedDIDURLException {
			return removePublicKey(id, false);
		}

		public boolean addAuthenticationKey(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			PublicKey pk = document.getPublicKey(id);
			if (pk == null)
				return false;

			return document.addAuthenticationKey(pk);
		}

		public boolean addAuthenticationKey(String id)
				throws MalformedDIDURLException {
			return addAuthenticationKey(new DIDURL(getSubject(), id));
		}

		public boolean addAuthenticationKey(DIDURL id, String pk) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || pk == null)
				throw new IllegalArgumentException();

			if (Base58.decode(pk).length != HDKey.PUBLICKEY_BYTES)
				throw new IllegalArgumentException("Invalid public key.");

			PublicKey key = new PublicKey(id, getSubject(), pk);
			return document.addAuthenticationKey(key);
		}

		public boolean addAuthenticationKey(String id, String pk)
				throws MalformedDIDURLException {
			return addAuthenticationKey(new DIDURL(getSubject(), id), pk);
		}

		public boolean removeAuthenticationKey(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			return document.removeAuthenticationKey(id);
		}

		public boolean removeAuthenticationKey(String id)
				throws MalformedDIDURLException {
			return removeAuthenticationKey(new DIDURL(getSubject(), id));
		}

		public boolean addAuthorizationKey(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			PublicKey pk = document.getPublicKey(id);
			if (pk == null)
				return false;

			return document.addAuthorizationKey(pk);
		}

		public boolean addAuthorizationKey(String id)
				throws MalformedDIDURLException {
			return addAuthorizationKey(new DIDURL(getSubject(), id));
		}

		public boolean addAuthorizationKey(DIDURL id, DID controller, String pk) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || controller == null || pk == null)
				throw new IllegalArgumentException();

			if (Base58.decode(pk).length != HDKey.PUBLICKEY_BYTES)
				throw new IllegalArgumentException("Invalid public key.");

			PublicKey key = new PublicKey(id, controller, pk);
			return document.addAuthorizationKey(key);
		}

		public boolean addAuthorizationKey(String id, String controller, String pk)
				throws MalformedDIDURLException, MalformedDIDException {
			return addAuthorizationKey(new DIDURL(getSubject(), id),
					new DID(controller), pk);
		}

		public boolean authorizationDid(DIDURL id, DID controller, DIDURL key)
				throws DIDException {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || controller == null)
				throw new IllegalArgumentException();

			// Can not authorize to self
			if (controller.equals(getSubject()))
				return false;

			DIDDocument controllerDoc = controller.resolve();
			if (controllerDoc == null)
				return false;

			if (key == null)
				key = controllerDoc.getDefaultPublicKey();

			// Check the key should be a authentication key.
			PublicKey targetPk = controllerDoc.getAuthenticationKey(key);
			if (targetPk == null)
				return false;

			PublicKey pk = new PublicKey(id, targetPk.getType(),
					controller, targetPk.getPublicKeyBase58());

			return document.addAuthorizationKey(pk);
		}

		public boolean authorizationDid(DIDURL id, DID controller)
				throws DIDException {
			return authorizationDid(id, controller, null);
		}

		public boolean authorizationDid(String id, String controller, String key)
				throws MalformedDIDURLException, MalformedDIDException, DIDException {
			DID controllerId = new DID(controller);
			DIDURL keyid = key == null ? null : new DIDURL(controllerId, key);

			return authorizationDid(new DIDURL(getSubject(), id),
					controllerId, keyid);
		}

		public boolean authorizationDid(String id, String controller)
				throws MalformedDIDURLException, MalformedDIDException, DIDException {
			return authorizationDid(id, controller, null);
		}

		public boolean removeAuthorizationKey(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			return document.removeAuthorizationKey(id);
		}

		public boolean removeAuthorizationKey(String id)
				throws MalformedDIDURLException {
			return removeAuthorizationKey(new DIDURL(getSubject(), id));
		}

		public boolean addCredential(VerifiableCredential vc) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (vc == null)
				throw new IllegalArgumentException();

			return document.addCredential(vc);
		}

		public boolean removeCredential(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			return document.removeCredential(id);
		}

		public boolean removeCredential(String id)
				throws MalformedDIDURLException {
			return removeCredential(new DIDURL(getSubject(), id));
		}

		public boolean addService(DIDURL id, String type, String endpoint) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || type == null || type.isEmpty() ||
					endpoint == null || endpoint.isEmpty() )
				throw new IllegalArgumentException();

			Service svc = new Service(id, type, endpoint);
			return document.addService(svc);
		}

		public boolean addService(String id, String type, String endpoint)
				throws MalformedDIDURLException {
			return addService(new DIDURL(getSubject(), id), type, endpoint);
		}

		public boolean removeService(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			return document.removeService(id);
		}

		public boolean removeService(String id) throws MalformedDIDURLException {
			return removeService(new DIDURL(getSubject(), id));
		}

		private Calendar getMaxExpires() {
			Calendar cal = Calendar.getInstance(Constants.UTC);
			cal.add(Calendar.YEAR, Constants.MAX_VALID_YEARS);
			return cal;
		}

		public void setDefaultExpires() {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			document.setExpires(getMaxExpires().getTime());
		}

		public boolean setExpires(Date expires) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (expires == null)
				throw new IllegalArgumentException();

			Calendar cal = Calendar.getInstance(Constants.UTC);
			cal.setTime(expires);

			if (cal.after(getMaxExpires()))
				return false;

			document.setExpires(expires);
			return true;
		}

		public DIDDocument seal(String storepass) throws DIDStoreException {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (storepass == null || storepass.isEmpty())
				throw new IllegalArgumentException();

			if (document.getExpires() == null)
				setDefaultExpires();

			DIDURL signKey = document.getDefaultPublicKey();
			String json = document.toJson(true, true);
			String sig = document.sign(signKey, storepass, json.getBytes());
			Proof proof = new Proof(signKey, sig);
			document.setProof(proof);

			// Invalidate builder
			DIDDocument doc = document;
			this.document = null;

			return doc;
		}
	}
}
