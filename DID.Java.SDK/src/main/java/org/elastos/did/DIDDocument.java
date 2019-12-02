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

import org.elastos.credential.MalformedCredentialException;
import org.elastos.credential.VerifiableCredential;
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

	private boolean readonly;

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

	protected DIDDocument() {
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

	protected void setSubject(DID subject) {
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

		return DIDStore.getInstance().containsPrivateKey(getSubject(), id);
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
		if (readonly)
			return false;

		if (publicKeys == null) {
			publicKeys = new TreeMap<DIDURL, PublicKey>();
		} else {
			// Check the existence
			if (publicKeys.containsKey(pk.getId()))
				return false;
		}

		publicKeys.put(pk.getId(), pk);
		return true;
	}

	public boolean addPublicKey(DIDURL id, DID controller, String pk) {
		if (id == null || controller == null || pk == null)
			throw new IllegalArgumentException();

		if ( Base58.decode(pk).length != HDKey.PUBLICKEY_BYTES)
			throw new IllegalArgumentException("Invalid public key.");

		PublicKey key = new PublicKey(id, controller, pk);
		return addPublicKey(key);
	}

	public boolean addPublicKey(String id, String controller, String pk)
			throws MalformedDIDURLException, MalformedDIDException {
		return addPublicKey(new DIDURL(getSubject(), id),
				new DID(controller), pk);
	}

	public boolean removePublicKey(DIDURL id, boolean force) {
		if (id == null)
			throw new IllegalArgumentException();

		if (readonly)
			return false;

		// Can not remove default public key
		if (getDefaultPublicKey().equals(id))
			return false;

		if (isAuthenticationKey(id)) {
			if (force)
				removeAuthenticationKey(id);
			else
				return false;
		}

		if (isAuthorizationKey(id)) {
			if (force)
				removeAuthorizationKey(id);
			else
				return false;
		}

		boolean removed = removeEntry(publicKeys, id);
		if (removed) {
			try {
				DIDStore.getInstance().deletePrivateKey(getSubject(), id);
			} catch (DIDStoreException ignore) {
				// TODO: CHECKME!
			}
		}

		return removed;
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
		if (readonly)
			return false;

		// Check the controller is current DID subject
		if (!pk.getController().equals(getSubject()))
			return false;

		// Confirm add the new pk to PublicKeys
		addPublicKey(pk);

		if (authentications == null) {
			authentications = new TreeMap<DIDURL, PublicKey>();
		} else {
			if (authentications.containsKey(pk.getId()))
				return false;
		}

		authentications.put(pk.getId(), pk);
		return true;
	}

	public boolean addAuthenticationKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		PublicKey pk = getPublicKey(id);
		if (pk == null)
			return false;

		return addAuthenticationKey(pk);
	}

	public boolean addAuthenticationKey(String id)
			throws MalformedDIDURLException {
		return addAuthenticationKey(new DIDURL(getSubject(), id));
	}

	public boolean addAuthenticationKey(DIDURL id, String pk) {
		if (id == null || pk == null)
			throw new IllegalArgumentException();

		if (Base58.decode(pk).length != HDKey.PUBLICKEY_BYTES)
			throw new IllegalArgumentException("Invalid public key.");

		PublicKey key = new PublicKey(id, getSubject(), pk);
		return addAuthenticationKey(key);
	}

	public boolean addAuthenticationKey(String id, String pk)
			throws MalformedDIDURLException {
		return addAuthenticationKey(new DIDURL(getSubject(), id), pk);
	}

	public boolean removeAuthenticationKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		if (readonly)
			return false;

		// Can not remove default public key
		if (getDefaultPublicKey().equals(id))
			return false;

		return removeEntry(authentications, id);
	}

	public boolean removeAuthenticationKey(String id)
			throws MalformedDIDURLException {
		return removeAuthenticationKey(new DIDURL(getSubject(), id));
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
		if (readonly)
			return false;

		// Can not authorize to self
		if (pk.getController().equals(getSubject()))
			return false;

		// Confirm add the new pk to PublicKeys
		addPublicKey(pk);

		if (authorizations == null) {
			authorizations = new TreeMap<DIDURL, PublicKey>();
		} else {
			if (authorizations.containsKey(pk.getId()))
				return false;
		}

		authorizations.put(pk.getId(), pk);
		return true;
	}

	public boolean addAuthorizationKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		PublicKey pk = getPublicKey(id);
		if (pk == null)
			return false;

		return addAuthorizationKey(pk);
	}

	public boolean addAuthorizationKey(String id)
			throws MalformedDIDURLException {
		return addAuthorizationKey(new DIDURL(getSubject(), id));
	}

	public boolean addAuthorizationKey(DIDURL id, DID controller, String pk) {
		if (id == null || controller == null || pk == null)
			throw new IllegalArgumentException();

		if (Base58.decode(pk).length != HDKey.PUBLICKEY_BYTES)
			throw new IllegalArgumentException("Invalid public key.");

		PublicKey key = new PublicKey(id, controller, pk);
		return addAuthorizationKey(key);
	}

	public boolean addAuthorizationKey(String id, String controller, String pk)
			throws MalformedDIDURLException, MalformedDIDException {
		return addAuthorizationKey(new DIDURL(getSubject(), id), new DID(controller), pk);
	}

	public boolean authorizationDid(DIDURL id, DID controller, DIDURL key)
			throws DIDException {
		if (id == null || controller == null)
			throw new IllegalArgumentException();

		// Can not authorize to self
		if (controller.equals(getSubject()))
			return false;

		DIDDocument doc = controller.resolve();
		if (doc == null)
			return false;

		if (key == null)
			key = doc.getDefaultPublicKey();

		PublicKey refPk = doc.getPublicKey(key);
		if (refPk == null)
			return false;

		// The public key should belongs to controller
		if (!refPk.getController().equals(controller))
			return false;

		PublicKey pk = new PublicKey(id,refPk.getType(),
				controller, refPk.getPublicKeyBase58());

		return addAuthorizationKey(pk);
	}

	public boolean authorizationDid(DIDURL id, DID controller)
			throws DIDException {
		if (id == null || controller == null)
			throw new IllegalArgumentException();

		return authorizationDid(id, controller);
	}

	public boolean authorizationDid(String id, String controller, String key)
			throws MalformedDIDURLException, MalformedDIDException, DIDException {
		DID controllerId = new DID(controller);
		DIDURL keyid = key == null ? null : new DIDURL(controllerId, key);
		return authorizationDid(new DIDURL(getSubject(), id), controllerId, keyid);
	}

	public boolean authorizationDid(String id, String controller)
			throws MalformedDIDURLException, MalformedDIDException, DIDException {
		return authorizationDid(id, controller, null);
	}

	public boolean removeAuthorizationKey(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		if (readonly)
			return false;

		return removeEntry(authorizations, id);
	}

	public boolean removeAuthorizationKey(String id)
			throws MalformedDIDURLException {
		return removeAuthorizationKey(new DIDURL(getSubject(), id));
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

	public boolean addCredential(VerifiableCredential vc) {
		if (vc == null)
			throw new IllegalArgumentException();

		if (readonly)
			return false;

		// Check the credential belongs to current DID.
		if (!vc.getSubject().getId().equals(getSubject()))
			return false;

		if (credentials == null) {
			credentials = new TreeMap<DIDURL, VerifiableCredential>();
		} else {
			if (credentials.containsKey(vc.getId()))
				return false;
		}

		EmbeddedCredential ec = new EmbeddedCredential(vc);
		credentials.put(ec.getId(), ec);
		return true;
	}

	public boolean removeCredential(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		if (readonly)
			return false;

		return removeEntry(credentials, id);
	}

	public boolean removeCredential(String id) throws MalformedDIDURLException {
		return removeCredential(new DIDURL(getSubject(), id));
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
		if (readonly)
			return false;

		if (services == null)
			services = new TreeMap<DIDURL, Service>();
		else {
			if (services.containsKey(svc.getId()))
				return false;
		}

		services.put(svc.getId(), svc);
		return true;
	}

	public boolean addService(DIDURL id, String type, String endpoint) {
		if (id == null || type == null || type.isEmpty() ||
				endpoint == null || endpoint.isEmpty() )
			throw new IllegalArgumentException();

		Service svc = new Service(id, type, endpoint);
		return addService(svc);
	}

	public boolean addService(String id, String type, String endpoint)
			throws MalformedDIDURLException {
		return addService(new DIDURL(getSubject(), id), type, endpoint);
	}

	public boolean removeService(DIDURL id) {
		if (id == null)
			throw new IllegalArgumentException();

		if (readonly)
			return false;

		return removeEntry(services, id);
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
		expires = getMaxExpires().getTime();
	}

	public boolean setExpires(Date expires) {
		if (readonly)
			return false;

		Calendar cal = Calendar.getInstance(Constants.UTC);
		cal.setTime(expires);

		if (cal.after(getMaxExpires()))
			return false;

		this.expires = cal.getTime();
		return true;
	}

	public Date getExpires() {
		return expires;
	}

	protected void setReadonly(boolean readonly) {
		this.readonly = readonly;
	}

	public boolean isReadonly() {
		return readonly;
	}

	public boolean modify() {
		setReadonly(false);
		return true;
	}

	public String sign(String storepass, byte[] ... data)
			throws DIDStoreException {
		DIDURL key = getDefaultPublicKey();
		return sign(key, storepass, data);
	}

	public String sign(DIDURL id, String storepass, byte[] ... data)
			throws DIDStoreException {
		if (id == null || storepass == null || data == null)
			throw new IllegalArgumentException();

		return DIDStore.getInstance().sign(getSubject(), id, storepass, data);
	}

	public String sign(String id, String storepass, byte[] ... data)
			throws MalformedDIDURLException, DIDStoreException {
		return DIDStore.getInstance().sign(getSubject(),
				new DIDURL(getSubject(), id), storepass, data);
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

	public boolean verify(DIDURL id, String signature, byte[] ... data)
			throws DIDException {
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
		if (reader == null)
			throw new IllegalArgumentException();

		DIDDocument doc = new DIDDocument();
		doc.parse(reader);
		doc.setReadonly(true);

		return doc;
	}

	public static DIDDocument fromJson(InputStream in)
			throws MalformedDocumentException {
		if (in == null)
			throw new IllegalArgumentException();

		DIDDocument doc = new DIDDocument();
		doc.parse(in);
		doc.setReadonly(true);

		return doc;
	}

	public static DIDDocument fromJson(String json)
			throws MalformedDocumentException {
		if (json == null || json.isEmpty())
			throw new IllegalArgumentException();

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
		if (out == null)
			throw new IllegalArgumentException();

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
		if (out == null)
			throw new IllegalArgumentException();

		if (charsetName == null)
			charsetName = "UTF-8";

		toJson(new OutputStreamWriter(out, charsetName), compact);
	}

	public void toJson(OutputStream out, boolean compact) throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

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
