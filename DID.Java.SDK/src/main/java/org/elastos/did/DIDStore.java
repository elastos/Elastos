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

import java.security.GeneralSecurityException;
import java.util.List;
import java.util.Map;

import org.elastos.credential.VerifiableCredential;
import org.elastos.did.backend.DIDBackend;
import org.elastos.did.util.Aes256cbc;
import org.elastos.did.util.Base64;
import org.elastos.did.util.EcdsaSigner;
import org.elastos.did.util.HDKey;

public abstract class DIDStore {
	public static final int DID_HAS_PRIVATEKEY = 0;
	public static final int DID_NO_PRIVATEKEY = 1;
	public static final int DID_ALL	= 2;

	private static DIDStore instance;

	private DIDBackend backend;
	private	HDKey privateIdentity;
	private int lastIndex;

	static public class Entry<K, V> implements java.io.Serializable {
		private static final long serialVersionUID = -4061538310957041415L;

		private final K key;
		private V value;

		public Entry(K key, V value) {
			this.key = key;
			this.value = value;
		}

		public Entry(Entry<? extends K, ? extends V> entry) {
			this.key = entry.getKey();
			this.value = entry.getValue();
		}

		public K getKey() {
			return key;
		}

		public V getValue() {
			return value;
		}

		public V setValue(V value) {
			V oldValue = this.value;
			this.value = value;
			return oldValue;
		}

		private static boolean eq(Object o1, Object o2) {
			return o1 == null ? o2 == null : o1.equals(o2);
		}

		@Override
		public boolean equals(Object o) {
			if (!(o instanceof Map.Entry))
				return false;
			Map.Entry<?, ?> e = (Map.Entry<?, ?>) o;
			return eq(key, e.getKey()) && eq(value, e.getValue());
		}

		@Override
		public int hashCode() {
			return (key == null   ? 0 : key.hashCode()  ) ^
				   (value == null ? 0 : value.hashCode());
		}

		@Override
		public String toString() {
			return key + "=" + value;
		}

	}

	public static void initialize(String type, String location, String storepass,
				DIDAdaptor adaptor) throws DIDStoreException {
		if (!type.equals("filesystem"))
			throw new IllegalArgumentException("Unsupported store type: " + type);

		instance = new FileSystemStore(location);
		instance.initPrivateIdentity(storepass);

		instance.backend = new DIDBackend(adaptor);
	}

	public static DIDStore getInstance() {
		return instance;
	}

	public abstract boolean hasPrivateIdentity() throws DIDStoreException;

	protected abstract void storePrivateIdentity(String key)
			throws DIDStoreException;

	protected abstract String loadPrivateIdentity() throws DIDStoreException;

	protected abstract void storePrivateIdentityIndex(int index)
			throws DIDStoreException;

	protected abstract int loadPrivateIdentityIndex() throws DIDStoreException;

	private static String encryptToBase64(String passwd, byte[] input)
			throws DIDStoreException {
		byte[] cipher;
		try {
			cipher = Aes256cbc.encrypt(passwd, input);
		} catch (GeneralSecurityException e) {
			throw new DIDStoreException("Encrypt key error.", e);
		}

		return Base64.encodeToString(cipher);
	}

	private static byte[] decryptFromBase64(String passwd, String input)
			throws DIDStoreException {
		byte[] cipher = Base64.decode(input);
		try {
			return Aes256cbc.decrypt(passwd, cipher);
		} catch (GeneralSecurityException e) {
			throw new DIDStoreException("Decrypt key error.", e);
		}
	}

	// Initialize & create new private identity and save it to DIDStore.
	public void initPrivateIdentity(String mnemonic, String passphrase,
			String storepass, boolean force) throws DIDStoreException {
		if (hasPrivateIdentity() && !force)
			throw new DIDStoreException("Already has private indentity.");

		if (passphrase == null)
			passphrase = "";

		privateIdentity = HDKey.fromMnemonic(mnemonic, passphrase);
		lastIndex = 0;

		// Save seed instead of root private key,
		// keep compatible with Native SDK
		String encryptedIdentity = encryptToBase64(storepass,
				privateIdentity.getSeed());
		storePrivateIdentity(encryptedIdentity);
		storePrivateIdentityIndex(lastIndex);
	}

	// initialized from saved private identity from DIDStore.
	protected boolean initPrivateIdentity(String storepass)
			throws DIDStoreException {
		if (!hasPrivateIdentity())
			return false;

		byte[] seed = decryptFromBase64(storepass, loadPrivateIdentity());
		privateIdentity = HDKey.fromSeed(seed);

		lastIndex = 0;
		try {
			lastIndex = loadPrivateIdentityIndex();
		} catch (DIDStoreException ignore) {
		}

		return true;
	}

	public DIDDocument newDid(String storepass, String hint)
			throws DIDStoreException {
		if (privateIdentity == null)
			throw new DIDStoreException("DID Store not contains private identity.");

		HDKey.DerivedKey key = privateIdentity.derive(lastIndex++);
		DID did = new DID(DID.METHOD, key.getAddress());
		PublicKey pk = new PublicKey(new DIDURL(did, "primary"),
				Constants.defaultPublicKeyType, did, key.getPublicKeyBase58());

		DIDDocument doc = new DIDDocument();
		doc.setSubject(did);
		doc.addPublicKey(pk);
		doc.addAuthenticationKey(pk);
		doc.setReadonly(true);

		storeDid(doc, hint);

		String encryptedKey = encryptToBase64(storepass, key.serialize());
		storePrivateKey(did, pk.getId(), encryptedKey);

		key.wipe();

		return doc;
	}

	public DIDDocument newDid(String storepass) throws DIDStoreException {
		return newDid(storepass, null);
	}

	public boolean publishDid(DIDDocument doc, String storepass)
			throws DIDStoreException {
		DIDURL signKey = doc.getDefaultPublicKey();
		return publishDid(doc, signKey, storepass);
	}

	public boolean publishDid(DIDDocument doc, DIDURL signKey, String storepass)
			throws DIDStoreException {
		storeDid(doc);
		return backend.create(doc, signKey, storepass);
	}

	public boolean updateDid(DIDDocument doc, DIDURL signKey, String storepass)
			throws DIDStoreException {
		storeDid(doc);

		return backend.update(doc, signKey, storepass);
	}

	public boolean deactivateDid(DID did, DIDURL signKey, String storepass)
			throws DIDStoreException {
		// TODO: how to handle locally?

		return backend.deactivate(did, signKey, storepass);
	}

	public DIDDocument resolveDid(DID did)
			throws DIDStoreException, MalformedDocumentException {
		return resolveDid(did, false);
	}

	public DIDDocument resolveDid(DID did, boolean force)
			throws DIDStoreException, MalformedDocumentException {
		DIDDocument doc = backend.resolve(did);
		if (doc != null)
			storeDid(doc);

		if (doc == null && !force)
			doc = loadDid(did);

		return doc;
	}

	public abstract void storeDid(DIDDocument doc, String hint)
			throws DIDStoreException;

	public void storeDid(DIDDocument doc) throws DIDStoreException {
		storeDid(doc, null);
	}

	public abstract void setDidHint(DID did, String hint) throws DIDStoreException;

	public abstract String getDidHint(DID did) throws DIDStoreException;

	public abstract DIDDocument loadDid(DID did)
			throws MalformedDocumentException, DIDStoreException;

	public DIDDocument loadDid(String did)
			throws MalformedDIDException, MalformedDocumentException,
			DIDStoreException {
		return loadDid(new DID(did));
	}

	public abstract boolean containsDid(DID did) throws DIDStoreException;

	public boolean containsDid(String did)
			throws MalformedDIDException, DIDStoreException {
		return containsDid(new DID(did));
	}

	public abstract boolean deleteDid(DID did) throws DIDStoreException;

	public boolean deleteDid(String did)
			throws MalformedDIDException, DIDStoreException {
		return deleteDid(new DID(did));
	}

	// Return a <DID, hint> tuples enumeration object
	public abstract List<Entry<DID, String>> listDids(int filter)
			throws DIDStoreException;

	public abstract void storeCredential(VerifiableCredential credential,
			String hint) throws DIDStoreException;

	public void storeCredential(VerifiableCredential credential)
			throws DIDStoreException {
		storeCredential(credential, null);
	}

	public abstract void setCredentialHint(DID did, DIDURL id, String hint)
			throws DIDStoreException;

	public abstract String getCredentialHint(DID did, DIDURL id)
			throws DIDStoreException;

	public abstract VerifiableCredential loadCredential(DID did, DIDURL id)
			throws MalformedCredentialException, DIDStoreException;

	public VerifiableCredential loadCredential(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			MalformedCredentialException, DIDStoreException {
		return loadCredential(new DID(did), new DIDURL(id));
	}

	public abstract boolean containsCredentials(DID did) throws DIDStoreException;

	public boolean containsCredentials(String did)
			throws MalformedDIDException, DIDStoreException {
		return containsCredentials(new DID(did));
	}

	public abstract boolean containsCredential(DID did, DIDURL id)
			throws DIDStoreException;

	public boolean containsCredential(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		return containsCredential(new DID(did), new DIDURL(id));
	}

	public abstract boolean deleteCredential(DID did, DIDURL id)
			throws DIDStoreException;

	public boolean deleteCredential(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		return deleteCredential(new DID(did), new DIDURL(id));
	}

	// Return a <DIDURL, hint> tuples enumeration object
	public abstract List<Entry<DIDURL, String>> listCredentials(DID did)
			throws DIDStoreException;

	public List<Entry<DIDURL, String>> listCredentials(String did)
			throws MalformedDIDException, DIDStoreException {
		return listCredentials(new DID(did));
	}

	public abstract List<Entry<DIDURL, String>> selectCredentials(DID did,
			DIDURL id, String[] type) throws DIDStoreException;

	public List<Entry<DIDURL, String>> selectCredentials(String did, String id,
			String[] type) throws MalformedDIDException,
			MalformedDIDURLException, DIDStoreException {
		return selectCredentials(new DID(did), new DIDURL(id), type);
	}

	public abstract boolean containsPrivateKeys(DID did)
			throws DIDStoreException;

	public boolean containsPrivateKeys(String did)
			throws MalformedDIDException, DIDStoreException {
		return containsPrivateKeys(new DID(did));
	}

	public abstract boolean containsPrivateKey(DID did, DIDURL id)
			throws DIDStoreException;

	public boolean containsPrivateKey(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		return containsPrivateKey(new DID(did), new DIDURL(id));
	}

	public abstract void storePrivateKey(DID did, DIDURL id, String privateKey)
			throws DIDStoreException;

	public void storePrivateKey(String did, String id, String privateKey)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		storePrivateKey(new DID(did), new DIDURL(id), privateKey);
	}

	protected abstract String loadPrivateKey(DID did, DIDURL id)
			throws DIDStoreException;

	public abstract boolean deletePrivateKey(DID did, DIDURL id)
			throws DIDStoreException;

	public void deletePrivateKey(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		deletePrivateKey(new DID(did), new DIDURL(id));
	}

	public String sign(DID did, DIDURL id, String storepass, byte[] ... data)
			throws DIDStoreException {
		byte[] binKey = decryptFromBase64(storepass, loadPrivateKey(did, id));
		HDKey.DerivedKey key = HDKey.DerivedKey.deserialize(binKey);

		byte[] sig = EcdsaSigner.sign(key.getPrivateKeyBytes(), data);

		key.wipe();

		return Base64.encodeToString(sig);
	}
}
