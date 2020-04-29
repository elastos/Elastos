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
import java.security.KeyPair;
import java.security.PrivateKey;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.function.Function;

import org.elastos.did.crypto.Base58;
import org.elastos.did.crypto.Base64;
import org.elastos.did.crypto.EcdsaSigner;
import org.elastos.did.crypto.HDKey;
import org.elastos.did.crypto.HDKey.DerivedKey;
import org.elastos.did.exception.DIDBackendException;
import org.elastos.did.exception.DIDNotFoundException;
import org.elastos.did.exception.DIDObjectAlreadyExistException;
import org.elastos.did.exception.DIDObjectNotExistException;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.InvalidKeyException;
import org.elastos.did.exception.MalformedCredentialException;
import org.elastos.did.exception.MalformedDIDException;
import org.elastos.did.exception.MalformedDocumentException;
import org.elastos.did.jwt.JwtBuilder;
import org.elastos.did.jwt.JwtParserBuilder;
import org.elastos.did.jwt.KeyProvider;
import org.elastos.did.meta.DIDMeta;
import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class DIDDocument {
	private final static String ID = "id";
	private final static String PUBLICKEY = "publicKey";
	private final static String TYPE = "type";
	private final static String CONTROLLER = "controller";
	private final static String PUBLICKEY_BASE58 = "publicKeyBase58";
	private final static String AUTHENTICATION = "authentication";
	private final static String AUTHORIZATION = "authorization";
	private final static String SERVICE = "service";
	private final static String VERIFIABLE_CREDENTIAL = "verifiableCredential";
	private final static String SERVICE_ENDPOINT = "serviceEndpoint";
	private final static String EXPIRES = "expires";
	private final static String PROOF = "proof";
	private final static String CREATOR = "creator";
	private final static String CREATED = "created";
	private final static String SIGNATURE_VALUE = "signatureValue";

	private final static String DEFAULT_PUBLICKEY_TYPE = Constants.DEFAULT_PUBLICKEY_TYPE;
	private final static int MAX_VALID_YEARS = Constants.MAX_VALID_YEARS;

	private DID subject;
	private Map<DIDURL, PublicKey> publicKeys;
	private Map<DIDURL, VerifiableCredential> credentials;
	private Map<DIDURL, Service> services;
	private Date expires;
	private Proof proof;

	private DIDMeta meta;

	public static class PublicKey extends DIDObject {
		private DID controller;
		private String keyBase58;
		private boolean authenticationKey;
		private boolean authorizationKey;

		protected PublicKey(DIDURL id, String type, DID controller, String keyBase58) {
			super(id, type);
			this.controller = controller;
			this.keyBase58 = keyBase58;
		}

		protected PublicKey(DIDURL id, DID controller, String keyBase58) {
			this(id, DEFAULT_PUBLICKEY_TYPE, controller, keyBase58);
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

		public boolean isAuthenticationKey() {
			return authenticationKey;
		}

		private void setAuthenticationKey(boolean authenticationKey) {
			this.authenticationKey = authenticationKey;
		}

		public boolean isAuthorizationKey() {
			return authorizationKey;
		}

		private void setAuthorizationKey(boolean authorizationKey) {
			this.authorizationKey = authorizationKey;
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

			DIDURL id = JsonHelper.getDidUrl(node, ID,
					ref, "publicKey' id", clazz);

			String type = JsonHelper.getString(node, TYPE, true,
					DEFAULT_PUBLICKEY_TYPE, "publicKey' type", clazz);

			DID controller = JsonHelper.getDid(node, CONTROLLER,
					true, ref, "publicKey' controller", clazz);

			String keyBase58 = JsonHelper.getString(node, PUBLICKEY_BASE58,
					false, null, "publicKeyBase58", clazz);

			return new PublicKey(id, type, controller, keyBase58);
		}

		protected void toJson(JsonGenerator generator, DID ref, boolean normalized)
				throws IOException {
			String value;

			generator.writeStartObject();

			// id
			generator.writeFieldName(ID);
			if (normalized || ref == null || !getId().getDid().equals(ref))
				value = getId().toString();
			else
				value = "#" + getId().getFragment();
			generator.writeString(value);

			// type
			if (normalized || !getType().equals(DEFAULT_PUBLICKEY_TYPE)) {
				generator.writeFieldName(TYPE);
				generator.writeString(getType());
			}

			// controller
			if (normalized || ref == null || !controller.equals(ref)) {
				generator.writeFieldName(CONTROLLER);
				generator.writeString(controller.toString());
			}

			// publicKeyBase58
			generator.writeFieldName(PUBLICKEY_BASE58);
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

			DIDURL id = JsonHelper.getDidUrl(node, ID,
					ref, "service' id", clazz);

			String type = JsonHelper.getString(node, TYPE, false,
					null, "service' type", clazz);

			String endpoint = JsonHelper.getString(node, SERVICE_ENDPOINT,
					false, null, "service' endpoint", clazz);

			return new Service(id, type, endpoint);
		}

		public void toJson(JsonGenerator generator, DID ref, boolean normalized)
				throws IOException {
			String value;

			generator.writeStartObject();

			// id
			generator.writeFieldName(ID);
			if (normalized || ref == null || !getId().getDid().equals(ref))
				value = getId().toString();
			else
				value = "#" + getId().getFragment();
			generator.writeString(value);

			// type
			generator.writeFieldName(TYPE);
			generator.writeString(getType());

			// endpoint
			generator.writeFieldName(SERVICE_ENDPOINT);
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
			this(DEFAULT_PUBLICKEY_TYPE,
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

			String type = JsonHelper.getString(node, TYPE, true,
					DEFAULT_PUBLICKEY_TYPE, "document proof type", clazz);

			Date created = JsonHelper.getDate(node, CREATED,
					true, null, "proof created date", clazz);

			DIDURL creator = JsonHelper.getDidUrl(node, CREATOR, true,
					refSignKey.getDid(), "document proof creator", clazz);
			if (creator == null)
				creator = refSignKey;

			String signature = JsonHelper.getString(node, SIGNATURE_VALUE,
					false, null, "document proof signature", clazz);

			return new Proof(type, created, creator, signature);
		}

		protected void toJson(JsonGenerator generator, boolean normalized)
				throws IOException {
			generator.writeStartObject();

			// type
			if (normalized || !type.equals(DEFAULT_PUBLICKEY_TYPE)) {
				generator.writeFieldName(TYPE);
				generator.writeString(type);
			}

			// created
			if (created != null) {
				generator.writeFieldName(CREATED);
				generator.writeString(JsonHelper.formatDate(created));
			}

			// creator
			if (normalized) {
				generator.writeFieldName(CREATOR);
				generator.writeString(creator.toString());
			}

			// signature
			generator.writeFieldName(SIGNATURE_VALUE);
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

		if (doc.credentials != null)
			this.credentials = new TreeMap<DIDURL, VerifiableCredential>(doc.credentials);

		if (doc.services != null)
			this.services = new TreeMap<DIDURL, Service>(doc.services);

		this.expires = doc.expires;
		this.proof = doc.proof;
		this.meta = doc.meta;
	}

	private <K, V extends DIDObject> int getEntryCount(Map<K, V> entries,
			Function<DIDObject, Boolean> filter) {
		if (entries == null || entries.isEmpty())
			return 0;

		if (filter == null) {
			return entries.size();
		} else {
			int count = 0;
			for (V entry : entries.values()) {
				if (filter.apply(entry))
					count++;
			}

			return count;
		}
	}

	private <K, V extends DIDObject> int getEntryCount(Map<K, V> entries) {
		return getEntryCount(entries, null);
	}

	private <K, V extends DIDObject> List<V> getEntries(Map<K, V> entries,
			Function<DIDObject, Boolean> filter) {
		List<V> lst = new ArrayList<V>(entries == null ? 0 : entries.size());

		if (entries != null && !entries.isEmpty()) {
			if (filter == null) {
				lst.addAll(entries.values());
			} else {
				for (V entry : entries.values()) {
					if (filter.apply(entry))
						lst.add(entry);
				}
			}
		}

		return lst;
	}

	private <K, V extends DIDObject> List<V> getEntries(Map<K, V> entries) {
		return getEntries(entries, null);
	}

	private <K, V extends DIDObject> V getEntry(Map<K, V> entries, K id) {
		if (entries == null || entries.isEmpty())
			return null;

		return entries.get(id);
	}

	private <K, V extends DIDObject> void removeEntry(Map<K, V> entries, K id) {
		if (entries == null || entries.isEmpty() || !entries.containsKey(id))
			throw new DIDObjectNotExistException(id.toString() + " not exists.");

		entries.remove(id);
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

	public List<PublicKey> selectPublicKeys(DIDURL id, String type) {
		if (id == null && type == null)
			throw new IllegalArgumentException();

		return getEntries(publicKeys, (v) -> {
			if (id != null && !v.getId().equals(id))
				return false;

			if (type != null && !v.getType().equals(type))
				return false;

			return true;
		});
	}

	public List<PublicKey> selectPublicKeys(String id, String type) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return selectPublicKeys(_id, type);
	}

	public PublicKey getPublicKey(String id) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return getPublicKey(_id);
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

	public boolean hasPublicKey(String id) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return hasPublicKey(_id);
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

	public boolean hasPrivateKey(String id) throws DIDStoreException {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return hasPrivateKey(_id);
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

		throw new IllegalStateException("DID Document internal error.");
	}

	public KeyPair getKeyPair(DIDURL id) throws InvalidKeyException {
		if (id == null)
			throw new IllegalArgumentException();

		if (!hasPublicKey(id))
			throw new InvalidKeyException("Key no exist");

		return DerivedKey.getKeyPair(getPublicKey(id).getPublicKeyBytes(), null);
	}

	public KeyPair getKeyPair(String id) throws InvalidKeyException {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return getKeyPair(_id);
	}

	private KeyPair getKeyPair(DIDURL id, String storepass)
			throws InvalidKeyException, DIDStoreException {
		if (id == null || storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException();

		if (!hasPublicKey(id))
			throw new InvalidKeyException("Key no exist");

		if (!getMeta().attachedStore())
			throw new DIDStoreException("Not attached with DID store.");

		if (!getMeta().getStore().containsPrivateKey(getSubject(), id))
			throw new InvalidKeyException("Don't have private key");

		return DerivedKey.getKeyPair(getPublicKey(id).getPublicKeyBytes(),
				getMeta().getStore().loadPrivateKey(getSubject(), id, storepass));
	}

	@SuppressWarnings("unused")
	private KeyPair getKeyPair(String id, String storepass)
			throws InvalidKeyException, DIDStoreException {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return getKeyPair(_id, storepass);
	}

	protected void addPublicKey(PublicKey pk) {
		if (publicKeys == null) {
			publicKeys = new TreeMap<DIDURL, PublicKey>();
		} else {
			// Check the existence, both id and keyBase58
			for (PublicKey key : publicKeys.values()) {
				if (key.getId().equals(pk.getId()))
					throw new DIDObjectAlreadyExistException("PublicKey id '"
							+ pk.getId() + "' already exist.");

				if (key.getPublicKeyBase58().equals(pk.getPublicKeyBase58()))
					throw new DIDObjectAlreadyExistException("PublicKey '"
							+ pk.getPublicKeyBase58() + "' already exist.");
			}
		}

		publicKeys.put(pk.getId(), pk);
	}

	protected void removePublicKey(DIDURL id, boolean force) {
		PublicKey pk = getEntry(publicKeys, id);
		if (pk == null)
			throw new DIDObjectNotExistException("PublicKey id '"
					+ id + "' not exist.");

		// Can not remove default public key
		if (getDefaultPublicKey().equals(id))
			throw new UnsupportedOperationException(
					"Cannot remove the default PublicKey.");

		if (!force) {
			if (pk.isAuthenticationKey() || pk.isAuthorizationKey())
				throw new UnsupportedOperationException("Key has references.");
		}

		removeEntry(publicKeys, id);
		try {
			if (getMeta().attachedStore())
				getMeta().getStore().deletePrivateKey(getSubject(), id);
		} catch (DIDStoreException ignore) {
			// TODO: CHECKME!
		}
	}

	public int getAuthenticationKeyCount() {
		return getEntryCount(publicKeys,
				(v) -> ((PublicKey)v).isAuthenticationKey());
	}

	public List<PublicKey> getAuthenticationKeys() {
		return getEntries(publicKeys,
				(v) -> ((PublicKey)v).isAuthenticationKey());
	}

	public List<PublicKey> selectAuthenticationKeys(DIDURL id, String type) {
		if (id == null && type == null)
			throw new IllegalArgumentException();

		return getEntries(publicKeys, (v) -> {
			if (!((PublicKey)v).isAuthenticationKey())
				return false;

			if (id != null && !v.getId().equals(id))
				return false;

			if (type != null && !v.getType().equals(type))
				return false;

			return true;
		});
	}

	public List<PublicKey> selectAuthenticationKeys(String id, String type) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return selectAuthenticationKeys(_id, type);
	}

	public PublicKey getAuthenticationKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		PublicKey pk = getEntry(publicKeys, id);
		if (pk != null && pk.isAuthenticationKey())
			return pk;
		else
			return null;
	}

	public PublicKey getAuthenticationKey(String id) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return getAuthenticationKey(_id);
	}

	public boolean isAuthenticationKey(DIDURL id) {
		return getAuthenticationKey(id) != null;
	}

	public boolean isAuthenticationKey(String id) {
		return getAuthenticationKey(id) != null;
	}

	protected void addAuthenticationKey(DIDURL id) {
		PublicKey key = getPublicKey(id);
		if (key == null)
			throw new DIDObjectNotExistException("PublicKey '" + id + "' not exists.");

		// Check the controller is current DID subject
		if (!key.getController().equals(getSubject()))
			throw new UnsupportedOperationException("Key cannot used for authentication.");

		key.setAuthenticationKey(true);
	}

	protected void removeAuthenticationKey(DIDURL id) {
		PublicKey pk = getEntry(publicKeys, id);
		if (pk == null)
			throw new DIDObjectNotExistException("PublicKey id '"
					+ id + "' not exist.");

		// Can not remove default public key
		if (getDefaultPublicKey().equals(id))
			throw new UnsupportedOperationException(
					"Cannot remove the default PublicKey from authentication.");

		pk.setAuthenticationKey(false);
	}

	public int getAuthorizationKeyCount() {
		return getEntryCount(publicKeys,
				(v) -> ((PublicKey)v).isAuthorizationKey());
	}

	public List<PublicKey> getAuthorizationKeys() {
		return getEntries(publicKeys,
				(v) -> ((PublicKey)v).isAuthorizationKey());
	}

	public List<PublicKey> selectAuthorizationKeys(DIDURL id, String type) {
		if (id == null && type == null)
			throw new IllegalArgumentException();

		return getEntries(publicKeys, (v) -> {
			if (!((PublicKey)v).isAuthorizationKey())
				return false;

			if (id != null && !v.getId().equals(id))
				return false;

			if (type != null && !v.getType().equals(type))
				return false;

			return true;
		});
	}

	public List<PublicKey> selectAuthorizationKeys(String id, String type) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return selectAuthorizationKeys(_id, type);
	}

	public PublicKey getAuthorizationKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		PublicKey pk = getEntry(publicKeys, id);
		if (pk != null && pk.isAuthorizationKey())
			return pk;
		else
			return null;
	}

	public PublicKey getAuthorizationKey(String id) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return getAuthorizationKey(_id);
	}

	public boolean isAuthorizationKey(DIDURL id) {
		return getAuthorizationKey(id) != null;
	}

	public boolean isAuthorizationKey(String id) {
		return getAuthorizationKey(id) != null;
	}

	protected void addAuthorizationKey(DIDURL id) {
		PublicKey key = getPublicKey(id);
		if (key == null)
			throw new DIDObjectNotExistException("PublicKey '" + id + "' not exists.");

		// Can not authorize to self
		if (key.getController().equals(getSubject()))
			throw new UnsupportedOperationException("Key cannot used for authorization.");

		key.setAuthorizationKey(true);
	}

	protected void removeAuthorizationKey(DIDURL id) {
		PublicKey pk = getEntry(publicKeys, id);
		if (pk == null)
			throw new DIDObjectNotExistException("PublicKey id '"
					+ id + "' not exist.");

		pk.setAuthorizationKey(false);
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

		return getEntries(credentials, (v) -> {
			if (id != null && !v.getId().equals(id))
				return false;

			if (type != null) {
				// Credential's type is a list.
				VerifiableCredential vc = (VerifiableCredential)v;
				if (!Arrays.asList(vc.getTypes()).contains(type))
					return false;
			}

			return true;
		});
	}

	public List<VerifiableCredential> selectCredentials(String id, String type) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return selectCredentials(_id, type);
	}

	public VerifiableCredential getCredential(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return getEntry(credentials, id);
	}

	public VerifiableCredential getCredential(String id) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return getCredential(_id);
	}

	protected void addCredential(VerifiableCredential vc) {
		// Check the credential belongs to current DID.
		if (!vc.getSubject().getId().equals(getSubject()))
			throw new UnsupportedOperationException("Credential not owned by self.");

		if (credentials == null) {
			credentials = new TreeMap<DIDURL, VerifiableCredential>();
		} else {
			if (credentials.containsKey(vc.getId()))
				throw new DIDObjectAlreadyExistException("Credential '"
						+ vc.getId() + "' already exist.");
		}

		credentials.put(vc.getId(), vc);
	}

	protected void removeCredential(DIDURL id) {
		removeEntry(credentials, id);
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

		return getEntries(services, (v) -> {
			if (id != null && !v.getId().equals(id))
				return false;

			if (type != null && !v.getType().equals(type))
				return false;

			return true;
		});
	}

	public List<Service> selectServices(String id, String type) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return selectServices(_id, type);
	}

	public Service getService(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		return getEntry(services, id);
	}

	public Service getService(String id) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return getService(_id);
	}

	protected void addService(Service svc) {
		if (services == null)
			services = new TreeMap<DIDURL, Service>();
		else {
			if (services.containsKey(svc.getId()))
				throw new DIDObjectAlreadyExistException("Service '"
						+ svc.getId() + "' already exist.");
		}

		services.put(svc.getId(), svc);
	}

	protected void removeService(DIDURL id) {
		removeEntry(services, id);
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
		subject.setMeta(meta);
	}

	protected DIDMeta getMeta() {
		if (meta == null) {
			meta = new DIDMeta();
			subject.setMeta(meta);
		}

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

	public boolean isDeactivated() {
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
		if (!proof.getType().equals(DEFAULT_PUBLICKEY_TYPE))
			return false;

		String json = toJson(true, true);
		return verify(proof.getCreator(), proof.getSignature(), json.getBytes());
	}

	public boolean isValid() {
		return !isDeactivated() && !isExpired() && isGenuine();
	}

	public Builder edit() {
		return new Builder(this);
	}

	public String sign(DIDURL id, String storepass, byte[] ... data)
			throws DIDStoreException {
		if (id == null || data == null || data.length == 0 ||
				storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException();

		byte[] digest = EcdsaSigner.sha256Digest(data);
		return signDigest(id, storepass, digest);
	}

	public String sign(String id, String storepass, byte[] ... data)
			throws DIDStoreException {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return sign(_id, storepass, data);
	}

	public String sign(String storepass, byte[] ... data)
			throws DIDStoreException {
		DIDURL key = getDefaultPublicKey();
		return sign(key, storepass, data);
	}

	public String signDigest(DIDURL id, String storepass, byte[] digest)
			throws DIDStoreException {
		if (id == null || digest == null || storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException();

		if (!getMeta().attachedStore())
			throw new DIDStoreException("Not attached with DID store.");

		return getMeta().getStore().sign(getSubject(), id, storepass, digest);
	}

	public String signDigest(String id, String storepass, byte[] digest)
			throws DIDStoreException {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return signDigest(_id, storepass, digest);
	}

	public String signDigest(String storepass, byte[] digest)
			throws DIDStoreException {
		DIDURL key = getDefaultPublicKey();
		return signDigest(key, storepass, digest);
	}

	public boolean verify(DIDURL id, String signature, byte[] ... data) {
		if (id == null || signature == null || signature.isEmpty() ||
				data == null || data.length == 0)
			throw new IllegalArgumentException();

		byte[] digest = EcdsaSigner.sha256Digest(data);
		return verifyDigest(id, signature, digest);
	}

	public boolean verify(String id, String signature, byte[] ... data) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return verify(_id, signature, data);
	}

	public boolean verify(String signature, byte[] ... data) {
		DIDURL key = getDefaultPublicKey();
		return verify(key, signature, data);
	}

	public boolean verifyDigest(DIDURL id, String signature, byte[] digest) {
		if (id == null || signature == null || signature.isEmpty() || digest == null)
			throw new IllegalArgumentException();

		PublicKey pk = getPublicKey(id);
		byte[] binkey = pk.getPublicKeyBytes();
		byte[] sig = Base64.decode(signature,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);

		return EcdsaSigner.verify(binkey, sig, digest);
	}

	public boolean verifyDigest(String id, String signature, byte[] digest) {
		DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
		return verifyDigest(_id, signature, digest);
	}

	public boolean verifyDigest(String signature, byte[] digest) {
		DIDURL key = getDefaultPublicKey();
		return verifyDigest(key, signature, digest);
	}

	private void parse(JsonNode doc) throws MalformedDocumentException {
		Class<MalformedDocumentException> clazz = MalformedDocumentException.class;

		setSubject(JsonHelper.getDid(doc, ID,
				false, null, "subject", clazz));

		JsonNode node = doc.get(PUBLICKEY);
		if (node == null)
			throw new MalformedDocumentException("Missing publicKey.");
		parsePublicKey(node);

		node = doc.get(AUTHENTICATION);
		if (node != null)
			parseAuthentication(node);

		DIDURL defaultPk = getDefaultPublicKey();
		if (defaultPk == null)
			throw new MalformedDocumentException("Missing default publicKey.");

		// Add default public key to authentication keys if needed.
		if (isAuthenticationKey(defaultPk))
			addAuthenticationKey(defaultPk);

		node = doc.get(AUTHORIZATION);
		if (node != null)
			parseAuthorization(node);

		node = doc.get(VERIFIABLE_CREDENTIAL);
		if (node != null)
			parseCredential(node);

		node = doc.get(SERVICE);
		if (node != null)
			parseService(node);

		expires = JsonHelper.getDate(doc, EXPIRES, true, null, "expires", clazz);

		node = doc.get(PROOF);
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

			addAuthenticationKey(pk.getId());
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

			addAuthorizationKey(pk.getId());
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
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(reader);
			doc.parse(node);
		} catch (IOException e) {
			throw new MalformedDocumentException("Parse JSON document error.", e);
		}

		return doc;
	}

	public static DIDDocument fromJson(InputStream in)
			throws MalformedDocumentException {
		if (in == null)
			throw new IllegalArgumentException();

		DIDDocument doc = new DIDDocument();
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(in);
			doc.parse(node);
		} catch (IOException e) {
			throw new MalformedDocumentException("Parse JSON document error.", e);
		}

		return doc;
	}

	public static DIDDocument fromJson(String json)
			throws MalformedDocumentException {
		if (json == null || json.isEmpty())
			throw new IllegalArgumentException();

		DIDDocument doc = new DIDDocument();
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(json);
			doc.parse(node);
		} catch (IOException e) {
			throw new MalformedDocumentException("Parse JSON document error.", e);
		}

		return doc;
	}

	protected static DIDDocument fromJson(JsonNode node)
			throws MalformedDocumentException {
		DIDDocument doc = new DIDDocument();
		doc.parse(node);
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
	private void toJson(JsonGenerator generator, boolean normalized,
			boolean forSign) throws IOException {
		generator.writeStartObject();

		// subject
		generator.writeFieldName(ID);
		generator.writeString(getSubject().toString());

		// publicKey
		generator.writeFieldName(PUBLICKEY);
		generator.writeStartArray();
		for (PublicKey pk : publicKeys.values())
			pk.toJson(generator, getSubject(), normalized);
		generator.writeEndArray();

		// authentication
		generator.writeFieldName(AUTHENTICATION);
		generator.writeStartArray();
		for (PublicKey pk : publicKeys.values()) {
			String value;

			if (!pk.isAuthenticationKey())
				continue;

			if (normalized || !pk.getId().getDid().equals(getSubject()))
				value = pk.getId().toString();
			else
				value = "#" + pk.getId().getFragment();

			generator.writeString(value);
		}
		generator.writeEndArray();

		// authorization
		if (getAuthorizationKeyCount() != 0) {
			generator.writeFieldName(AUTHORIZATION);
			generator.writeStartArray();
			for (PublicKey pk : publicKeys.values()) {
				String value;

				if (!pk.isAuthorizationKey())
					continue;

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
			generator.writeFieldName(VERIFIABLE_CREDENTIAL);
			generator.writeStartArray();
			for (VerifiableCredential vc : credentials.values())
				vc.toJson(generator, getSubject(), normalized);
			generator.writeEndArray();
		}

		// service
		if (services != null && services.size() != 0) {
			generator.writeFieldName(SERVICE);
			generator.writeStartArray();
			for (Service svc : services.values())
				svc.toJson(generator, getSubject(), normalized);
			generator.writeEndArray();
		}

		// expires
		if (expires != null) {
			generator.writeFieldName(EXPIRES);
			generator.writeString(JsonHelper.formatDate(expires));
		}

		// proof
		if (proof != null && !forSign) {
			generator.writeFieldName(PROOF);
			proof.toJson(generator, normalized);
		}

		generator.writeEndObject();
	}

	protected void toJson(JsonGenerator generator, boolean normalized)
			throws IOException {
		toJson(generator, normalized, false);
	}

	private void toJson(Writer out, boolean normalized, boolean forSign)
			throws IOException {
		JsonFactory factory = new JsonFactory();
		JsonGenerator generator = factory.createGenerator(out);
		toJson(generator, normalized, forSign);
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

	public JwtBuilder jwtBuilder() {
		JwtBuilder builder = new JwtBuilder(getSubject().toString(), new KeyProvider() {

			@Override
			public java.security.PublicKey getPublicKey(String id)
					throws InvalidKeyException {
				DIDURL _id = id == null ? getDefaultPublicKey() :
					new DIDURL(getSubject(), id);

				return getKeyPair(_id).getPublic();
			}

			@Override
			public PrivateKey getPrivateKey(String id, String storepass)
					throws InvalidKeyException, DIDStoreException {
				DIDURL _id = id == null ? getDefaultPublicKey() :
					new DIDURL(getSubject(), id);

				return getKeyPair(_id, storepass).getPrivate();
			}
		});

		return builder.setIssuer(getSubject().toString());
	}

	public JwtParserBuilder jwtParserBuilder() {
		return new JwtParserBuilder(new KeyProvider() {

			@Override
			public java.security.PublicKey getPublicKey(String id)
					throws InvalidKeyException {
				DIDURL _id = id == null ? getDefaultPublicKey() :
					new DIDURL(getSubject(), id);

				return getKeyPair(_id).getPublic();
			}

			@Override
			public PrivateKey getPrivateKey(String id, String storepass) {
				return null;
			}
		});
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

		public Builder addPublicKey(DIDURL id, DID controller, String pk) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || controller == null || pk == null)
				throw new IllegalArgumentException();

			if ( Base58.decode(pk).length != HDKey.PUBLICKEY_BYTES)
				throw new IllegalArgumentException("Invalid public key.");

			PublicKey key = new PublicKey(id, controller, pk);
			document.addPublicKey(key);

			return this;
		}

		public Builder addPublicKey(String id, String controller, String pk) {
			DID _controller = null;
			try {
				_controller = new DID(controller);
			} catch (MalformedDIDException e) {
				throw new IllegalArgumentException();
			}

			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addPublicKey(_id, _controller, pk);
		}

		public Builder removePublicKey(DIDURL id, boolean force) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			document.removePublicKey(id, force);

			return this;
		}

		public Builder removePublicKey(String id, boolean force) {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return removePublicKey(_id, force);
		}

		public Builder removePublicKey(DIDURL id) {
			return removePublicKey(id, false);
		}

		public Builder removePublicKey(String id) {
			return removePublicKey(id, false);
		}

		public Builder addAuthenticationKey(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			document.addAuthenticationKey(id);

			return this;
		}

		public Builder addAuthenticationKey(String id) {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addAuthenticationKey(_id);
		}

		public Builder addAuthenticationKey(DIDURL id, String pk) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || pk == null)
				throw new IllegalArgumentException();

			if (Base58.decode(pk).length != HDKey.PUBLICKEY_BYTES)
				throw new IllegalArgumentException("Invalid public key.");

			PublicKey key = new PublicKey(id, getSubject(), pk);
			key.setAuthenticationKey(true);
			document.addPublicKey(key);

			return this;
		}

		public Builder addAuthenticationKey(String id, String pk) {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addAuthenticationKey(_id, pk);
		}

		public Builder removeAuthenticationKey(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			document.removeAuthenticationKey(id);

			return this;
		}

		public Builder removeAuthenticationKey(String id) {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return removeAuthenticationKey(_id);
		}

		public Builder addAuthorizationKey(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			document.addAuthorizationKey(id);

			return this;
		}

		public Builder addAuthorizationKey(String id) {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addAuthorizationKey(_id);
		}

		public Builder addAuthorizationKey(DIDURL id, DID controller, String pk) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || controller == null || pk == null)
				throw new IllegalArgumentException();

			// Can not authorize to self
			if (controller.equals(getSubject()))
				throw new UnsupportedOperationException("Invalid controller.");

			if (Base58.decode(pk).length != HDKey.PUBLICKEY_BYTES)
				throw new IllegalArgumentException("Invalid public key.");

			PublicKey key = new PublicKey(id, controller, pk);
			key.setAuthorizationKey(true);
			document.addPublicKey(key);

			return this;
		}

		public Builder addAuthorizationKey(String id, String controller, String pk) {
			DID _controller = null;
			try {
				_controller = new DID(controller);
			} catch (MalformedDIDException e) {
				throw new IllegalArgumentException();
			}

			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addAuthorizationKey(_id, _controller, pk);
		}

		public Builder authorizationDid(DIDURL id, DID controller, DIDURL key)
				throws DIDResolveException, DIDBackendException, InvalidKeyException {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || controller == null)
				throw new IllegalArgumentException();

			// Can not authorize to self
			if (controller.equals(getSubject()))
				throw new UnsupportedOperationException("Invalid controller.");

			DIDDocument controllerDoc = controller.resolve();
			if (controllerDoc == null)
				throw new DIDNotFoundException(id.toString());

			if (key == null)
				key = controllerDoc.getDefaultPublicKey();

			// Check the key should be a authentication key.
			PublicKey targetPk = controllerDoc.getAuthenticationKey(key);
			if (targetPk == null)
				throw new InvalidKeyException(key.toString());

			PublicKey pk = new PublicKey(id, targetPk.getType(),
					controller, targetPk.getPublicKeyBase58());
			pk.setAuthorizationKey(true);
			document.addPublicKey(pk);

			return this;
		}

		public Builder authorizationDid(DIDURL id, DID controller)
				throws DIDResolveException, DIDBackendException, InvalidKeyException {
			return authorizationDid(id, controller, null);
		}

		public Builder authorizationDid(String id, String controller, String key)
				throws DIDResolveException, DIDBackendException, InvalidKeyException {
			DID controllerId = null;
			try {
				controllerId = new DID(controller);
			} catch (MalformedDIDException e) {
				throw new IllegalArgumentException(e);
			}

			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			DIDURL _key = key == null ? null : new DIDURL(controllerId, key);
			return authorizationDid(_id, controllerId, _key);
		}

		public Builder authorizationDid(String id, String controller)
				throws DIDResolveException, DIDBackendException, InvalidKeyException {
			return authorizationDid(id, controller, null);
		}

		public Builder removeAuthorizationKey(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			document.removeAuthorizationKey(id);

			return this;
		}

		public Builder removeAuthorizationKey(String id) {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return removeAuthorizationKey(_id);
		}

		public Builder addCredential(VerifiableCredential vc) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (vc == null)
				throw new IllegalArgumentException();

			document.addCredential(vc);

			return this;
		}

		public Builder addCredential(DIDURL id, String[] types,
				Map<String, String> subject, Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || subject == null || subject.isEmpty() ||
					storepass == null || storepass.isEmpty())
				throw new IllegalArgumentException();

			Issuer issuer = new Issuer(document);
			Issuer.CredentialBuilder cb = issuer.issueFor(document.getSubject());
			if (types == null)
				types = new String[]{ "SelfProclaimedCredential" };

			if (expirationDate == null)
				expirationDate = document.getExpires();

			try {
				VerifiableCredential vc = cb.id(id)
						.type(types)
						.properties(subject)
						.expirationDate(expirationDate)
						.seal(storepass);

				document.addCredential(vc);
			} catch (MalformedCredentialException ignore) {
			}

			return this;
		}

		public Builder addCredential(String id, String[] types,
				Map<String, String> subject, Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, types, subject, expirationDate, storepass);
		}

		public Builder addCredential(DIDURL id, Map<String, String> subject,
				Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			return addCredential(id, null, subject, expirationDate, storepass);
		}

		public Builder addCredential(String id, Map<String, String> subject,
				Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, subject, expirationDate, storepass);
		}

		public Builder addCredential(DIDURL id, String[] types,
				Map<String,String> subject, String storepass)
				throws DIDStoreException, InvalidKeyException {
			return addCredential(id, types, subject, null, storepass);
		}

		public Builder addCredential(String id, String[] types,
				Map<String, String> subject, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, types, subject, storepass);
		}

		public Builder addCredential(DIDURL id, Map<String, String> subject,
				String storepass)
				throws DIDStoreException, InvalidKeyException {
			return addCredential(id, null, subject, null, storepass);
		}

		public Builder addCredential(String id, Map<String, String> subject,
				String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, subject, storepass);
		}

		public Builder addCredential(DIDURL id, String[] types,
				String json, Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || json == null || json.isEmpty() ||
					storepass == null || storepass.isEmpty())
				throw new IllegalArgumentException();

			Issuer issuer = new Issuer(document);
			Issuer.CredentialBuilder cb = issuer.issueFor(document.getSubject());
			if (types == null)
				types = new String[]{ "SelfProclaimedCredential" };

			if (expirationDate == null)
				expirationDate = document.expires;

			try {
				VerifiableCredential vc = cb.id(id)
						.type(types)
						.properties(json)
						.expirationDate(expirationDate)
						.seal(storepass);

				document.addCredential(vc);
			} catch (MalformedCredentialException ignore) {
			}

			return this;
		}

		public Builder addCredential(String id, String[] types,
				String json, Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, types, json, expirationDate, storepass);
		}

		public Builder addCredential(DIDURL id, String json,
				Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			return addCredential(id, null, json, expirationDate, storepass);
		}

		public Builder addCredential(String id, String json,
				Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, json, expirationDate, storepass);
		}

		public Builder addCredential(DIDURL id, String[] types,
				String json, String storepass)
				throws DIDStoreException, InvalidKeyException {
			return addCredential(id, types, json, null, storepass);
		}

		public Builder addCredential(String id, String[] types,
				String json, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, types, json, storepass);
		}

		public Builder addCredential(DIDURL id, String json, String storepass)
				throws DIDStoreException, InvalidKeyException {
			return addCredential(id, null, json, null, storepass);
		}

		public Builder addCredential(String id, String json, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, json, storepass);
		}

		public Builder addCredential(DIDURL id, String[] types,
				JsonNode node, Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || node == null || node.size() == 0 ||
					storepass == null || storepass.isEmpty())
				throw new IllegalArgumentException();

			Issuer issuer = new Issuer(document);
			Issuer.CredentialBuilder cb = issuer.issueFor(document.getSubject());
			if (types == null)
				types = new String[]{ "SelfProclaimedCredential" };

			if (expirationDate == null)
				expirationDate = document.expires;

			try {
				VerifiableCredential vc = cb.id(id)
						.type(types)
						.properties(node)
						.expirationDate(expirationDate)
						.seal(storepass);

				document.addCredential(vc);
			} catch (MalformedCredentialException ignore) {
			}

			return this;
		}

		public Builder addCredential(String id, String[] types,
				JsonNode node, Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, types, node, expirationDate, storepass);
		}

		public Builder addCredential(DIDURL id, JsonNode node,
				Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			return addCredential(id, null, node, expirationDate, storepass);
		}

		public Builder addCredential(String id, JsonNode node,
				Date expirationDate, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, node, expirationDate, storepass);
		}

		public Builder addCredential(DIDURL id, String[] types,
				JsonNode node, String storepass)
				throws DIDStoreException, InvalidKeyException {
			return addCredential(id, types, node, null, storepass);
		}

		public Builder addCredential(String id, String[] types,
				JsonNode node, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, types, node, storepass);
		}

		public Builder addCredential(DIDURL id, JsonNode node, String storepass)
				throws DIDStoreException, InvalidKeyException {
			return addCredential(id, null, node, null, storepass);
		}

		public Builder addCredential(String id, JsonNode node, String storepass)
				throws DIDStoreException, InvalidKeyException {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addCredential(_id, node, storepass);
		}

		public Builder removeCredential(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			document.removeCredential(id);

			return this;
		}

		public Builder removeCredential(String id) {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return removeCredential(_id);
		}

		public Builder addService(DIDURL id, String type, String endpoint) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null || type == null || type.isEmpty() ||
					endpoint == null || endpoint.isEmpty() )
				throw new IllegalArgumentException();

			Service svc = new Service(id, type, endpoint);
			document.addService(svc);

			return this;
		}

		public Builder addService(String id, String type, String endpoint) {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return addService(_id, type, endpoint);
		}

		public Builder removeService(DIDURL id) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (id == null)
				throw new IllegalArgumentException();

			document.removeService(id);

			return this;
		}

		public Builder removeService(String id) {
			DIDURL _id = id == null ? null : new DIDURL(getSubject(), id);
			return removeService(_id);
		}

		private Calendar getMaxExpires() {
			Calendar cal = Calendar.getInstance(Constants.UTC);
			cal.add(Calendar.YEAR, MAX_VALID_YEARS);
			return cal;
		}

		public Builder setDefaultExpires() {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			document.setExpires(getMaxExpires().getTime());
			return this;
		}

		public Builder setExpires(Date expires) {
			if (document == null)
				throw new IllegalStateException("Document already sealed.");

			if (expires == null)
				throw new IllegalArgumentException();

			Calendar cal = Calendar.getInstance(Constants.UTC);
			cal.setTime(expires);

			if (cal.after(getMaxExpires()))
				throw new IllegalArgumentException("Invalid date.");

			document.setExpires(expires);
			return this;
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
