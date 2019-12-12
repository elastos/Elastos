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

import java.util.List;
import java.util.Map;

import org.elastos.credential.MalformedCredentialException;
import org.elastos.credential.VerifiableCredential;
import org.elastos.did.backend.DIDBackend;
import org.elastos.did.util.Aes256cbc;
import org.elastos.did.util.Base64;
import org.elastos.did.util.EcdsaSigner;
import org.elastos.did.util.HDKey;
import org.elastos.did.util.LRUCache;

public abstract class DIDStore {
	private static final int CACHE_INITIAL_CAPACITY = 16;
	private static final int CACHE_MAX_CAPACITY = 32;

	public static final int DID_HAS_PRIVATEKEY = 0;
	public static final int DID_NO_PRIVATEKEY = 1;
	public static final int DID_ALL	= 2;

	private static DIDStore instance;

	private Map<DID, DIDDocument> didCache;
	private Map<DIDURL, VerifiableCredential> vcCache;

	private DIDBackend backend;

	protected void setupStore(int initialCacheCapacity, int maxCacheCapacity,
			DIDAdapter adapter) {
		if (maxCacheCapacity > 0) {
			this.didCache = LRUCache.createInstance(initialCacheCapacity, maxCacheCapacity);
			this.vcCache = LRUCache.createInstance(initialCacheCapacity, maxCacheCapacity);
		}

		this.backend = new DIDBackend(adapter);
	}

	public static void initialize(String type, String location,
			int initialCacheCapacity, int maxCacheCapacity,
			DIDAdapter adapter) throws DIDStoreException {
		if (type == null || location == null ||
				location.isEmpty() || adapter == null)
			throw new IllegalArgumentException();

		if (!type.equals("filesystem"))
			throw new DIDStoreException("Unsupported store type: " + type);

		instance = new FileSystemStore(location);
		instance.setupStore(initialCacheCapacity, maxCacheCapacity, adapter);
	}

	public static void initialize(String type, String location,
			DIDAdapter adapter) throws DIDStoreException {
		initialize(type, location, CACHE_INITIAL_CAPACITY,
				CACHE_MAX_CAPACITY, adapter);
	}

	public static boolean isInitialized() {
		return instance != null;
	}

	public static DIDStore getInstance() throws DIDStoreException {
		if (instance == null)
			throw new DIDStoreException("Store not initialized.");

		return instance;
	}

	protected DIDAdapter getAdapter() {
		return backend.getAdapter();
	}

	public abstract boolean hasPrivateIdentity() throws DIDStoreException;

	protected abstract void storePrivateIdentity(String key)
			throws DIDStoreException;

	protected abstract String loadPrivateIdentity() throws DIDStoreException;

	protected abstract void storePrivateIdentityIndex(int index)
			throws DIDStoreException;

	protected abstract int loadPrivateIdentityIndex() throws DIDStoreException;

	protected static String encryptToBase64(String passwd, byte[] input)
			throws DIDStoreException {
		byte[] cipher;
		try {
			cipher = Aes256cbc.encrypt(passwd, input);
		} catch (Exception e) {
			throw new DIDStoreException("Encrypt key error.", e);
		}

		return Base64.encodeToString(cipher,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
	}

	protected static byte[] decryptFromBase64(String storepass, String input)
			throws DIDStoreException {
		byte[] cipher = Base64.decode(input,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		try {
			return Aes256cbc.decrypt(storepass, cipher);
		} catch (Exception e) {
			throw new DIDStoreException("Decrypt private key error, maybe wrong store password.", e);
		}
	}

	// Initialize & create new private identity and save it to DIDStore.
	public void initPrivateIdentity(int language, String mnemonic,
			String passphrase, String storepass, boolean force)
					throws DIDStoreException {
		if (!Mnemonic.isValid(language, mnemonic))
			throw new IllegalArgumentException("Invalid mnemonic.");

		if (storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException("Invalid password.");

		if (hasPrivateIdentity() && !force)
			throw new DIDStoreException("Already has private indentity.");

		if (passphrase == null)
			passphrase = "";

		HDKey privateIdentity = HDKey.fromMnemonic(mnemonic, passphrase);

		// Save seed instead of root private key,
		// keep compatible with Native SDK
		String encryptedIdentity = encryptToBase64(storepass,
				privateIdentity.getSeed());
		storePrivateIdentity(encryptedIdentity);
		storePrivateIdentityIndex(0);

		privateIdentity.wipe();
	}

	public void initPrivateIdentity(int language, String mnemonic,
			String passphrase, String storepass) throws DIDStoreException {
		initPrivateIdentity(language, mnemonic, passphrase, storepass, false);
	}

	// initialized from saved private identity from DIDStore.
	protected HDKey loadPrivateIdentity(String storepass)
			throws DIDStoreException {
		if (!hasPrivateIdentity())
			return null;

		byte[] seed = decryptFromBase64(storepass, loadPrivateIdentity());
		return HDKey.fromSeed(seed);
	}

	public void synchronize(String storepass) throws DIDStoreException  {
		if (storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException("Invalid password.");

		HDKey privateIdentity = loadPrivateIdentity(storepass);
		if (privateIdentity == null)
			throw new DIDStoreException("DID Store not contains private identity.");

		int nextIndex = loadPrivateIdentityIndex();
		int blanks = 0;
		int i = 0;

		while (i < nextIndex || blanks < 10) {
			HDKey.DerivedKey key = privateIdentity.derive(i++);
			DID did = new DID(DID.METHOD, key.getAddress());

			DIDDocument doc = backend.resolve(did);
			if (doc != null) {
				// TODO: check local conflict
				storeDid(doc);

				// Save private key
				String encryptedKey = encryptToBase64(storepass, key.serialize());
				storePrivateKey(did, doc.getDefaultPublicKey(), encryptedKey);

				if (i >= nextIndex)
					storePrivateIdentityIndex(i);

				blanks = 0;
			} else {
				blanks++;
			}
		}
	}

	public DIDDocument newDid(String storepass, String alias)
			throws DIDStoreException {
		if (storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException("Invalid password.");

		HDKey privateIdentity = loadPrivateIdentity(storepass);
		if (privateIdentity == null)
			throw new DIDStoreException("DID Store not contains private identity.");

		int nextIndex = loadPrivateIdentityIndex();

		HDKey.DerivedKey key = privateIdentity.derive(nextIndex++);
		DID did = new DID(DID.METHOD, key.getAddress());
		DIDURL id = new DIDURL(did, "primary");

		String encryptedKey = encryptToBase64(storepass, key.serialize());
		storePrivateKey(did, id, encryptedKey);

		DIDDocument.Builder db = new DIDDocument.Builder(did);
		db.addAuthenticationKey(id, key.getPublicKeyBase58());
		DIDDocument doc = db.seal(storepass);
		storeDid(doc, alias);

		storePrivateIdentityIndex(nextIndex);
		privateIdentity.wipe();
		key.wipe();

		return doc;
	}

	public DIDDocument newDid(String storepass) throws DIDStoreException {
		return newDid(storepass, null);
	}

	public boolean publishDid(DIDDocument doc, DIDURL signKey, String storepass)
			throws DIDStoreException {
		if (doc == null || storepass == null)
			throw new IllegalArgumentException();

		if (signKey == null)
			signKey = doc.getDefaultPublicKey();

		return backend.create(doc, signKey, storepass);
	}

	public boolean publishDid(DIDDocument doc, String signKey, String storepass)
			throws MalformedDIDURLException, DIDStoreException {
		DIDURL id = signKey == null ? null : new DIDURL(doc.getSubject(), signKey);
		return publishDid(doc, id, storepass);
	}

	public boolean publishDid(DIDDocument doc, String storepass)
			throws DIDStoreException {
		return publishDid(doc, (DIDURL)null, storepass);
	}

	public boolean updateDid(DIDDocument doc, DIDURL signKey, String storepass)
			throws DIDStoreException {
		if (doc == null || storepass == null)
			throw new IllegalArgumentException();

		if (signKey == null)
			signKey = doc.getDefaultPublicKey();

		storeDid(doc);
		return backend.update(doc, signKey, storepass);
	}

	public boolean updateDid(DIDDocument doc, String signKey, String storepass)
			throws MalformedDIDURLException, DIDStoreException {
		DIDURL id = signKey == null ? null : new DIDURL(doc.getSubject(), signKey);
		return updateDid(doc, id, storepass);
	}

	public boolean updateDid(DIDDocument doc, String storepass)
			throws DIDStoreException {
		return updateDid(doc, (DIDURL)null, storepass);
	}

	public boolean deactivateDid(DID did, DIDURL signKey, String storepass)
			throws DIDStoreException {
		if (did == null || storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException();

		if (signKey == null) {
			try {
				DIDDocument doc = resolveDid(did);
				if (doc == null)
					throw new DIDStoreException("Can not resolve DID document.");

				signKey = doc.getDefaultPublicKey();
			} catch (MalformedDocumentException e) {
				throw new DIDStoreException(e);
			}
		}

		return backend.deactivate(did, signKey, storepass);

		// TODO: how to handle locally?
	}

	public boolean deactivateDid(DID did, String signKey, String storepass)
			throws MalformedDIDURLException, DIDStoreException {
		DIDURL id = signKey == null ? null : new DIDURL(did, signKey);
		return deactivateDid(did, id, storepass);
	}

	public boolean deactivateDid(DID did, String storepass)
			throws DIDStoreException {
		return deactivateDid(did, (DIDURL)null, storepass);
	}

	public DIDDocument resolveDid(DID did, boolean force)
			throws DIDStoreException, MalformedDocumentException {
		if (did == null)
			throw new IllegalArgumentException();

		DIDDocument doc = backend.resolve(did);
		if (doc != null)
			storeDid(doc);

		if (doc == null && !force)
			doc = loadDid(did);

		return doc;
	}

	public DIDDocument resolveDid(String did, boolean force)
			throws MalformedDIDException, MalformedDocumentException,
			DIDStoreException  {
		return resolveDid(new DID(did), force);
	}

	public DIDDocument resolveDid(DID did)
			throws DIDStoreException, MalformedDocumentException {
		return resolveDid(did, false);
	}

	public DIDDocument resolveDid(String did)
			throws MalformedDIDException, MalformedDocumentException,
			DIDStoreException  {
		return resolveDid(did, false);
	}

	public abstract void storeDid(DIDDocument doc, String alias)
			throws DIDStoreException;

	public void storeDid(DIDDocument doc) throws DIDStoreException {
		storeDid(doc, null);
	}

	public abstract void setDidAlias(DID did, String alias)
			throws DIDStoreException;

	public void setDidAlias(String did, String alias)
			throws MalformedDIDException, DIDStoreException {
		setDidAlias(new DID(did), alias);
	}

	public abstract String getDidAlias(DID did) throws DIDStoreException;

	public String getDidAlias(String did)
			throws MalformedDIDException, DIDStoreException {
		return getDidAlias(new DID(did));
	}

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

	public abstract List<DID> listDids(int filter)
			throws DIDStoreException;

	public abstract void storeCredential(VerifiableCredential credential,
			String alias) throws DIDStoreException;

	public void storeCredential(VerifiableCredential credential)
			throws DIDStoreException {
		storeCredential(credential, null);
	}

	public abstract void setCredentialAlias(DID did, DIDURL id, String alias)
			throws DIDStoreException;

	public void setCredentialAlias(String did, String id, String alias)
			throws  MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		DID _did = new DID(did);
		setCredentialAlias(_did, new DIDURL(_did, id), alias);
	}

	public abstract String getCredentialAlias(DID did, DIDURL id)
			throws DIDStoreException;

	public String getCredentialAlias(String did, String id)
			throws  MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		DID _did = new DID(did);
		return getCredentialAlias(_did, new DIDURL(_did, id));
	}

	public abstract VerifiableCredential loadCredential(DID did, DIDURL id)
			throws MalformedCredentialException, DIDStoreException;

	public VerifiableCredential loadCredential(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			MalformedCredentialException, DIDStoreException {
		DID _did = new DID(did);
		return loadCredential(_did, new DIDURL(_did, id));
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
		DID _did = new DID(did);
		return containsCredential(_did, new DIDURL(_did, id));
	}

	public abstract boolean deleteCredential(DID did, DIDURL id)
			throws DIDStoreException;

	public boolean deleteCredential(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		DID _did = new DID(did);
		return deleteCredential(_did, new DIDURL(_did, id));
	}

	public abstract List<DIDURL> listCredentials(DID did)
			throws DIDStoreException;

	public List<DIDURL> listCredentials(String did)
			throws MalformedDIDException, DIDStoreException {
		return listCredentials(new DID(did));
	}

	public abstract List<DIDURL> selectCredentials(DID did,
			DIDURL id, String[] type) throws DIDStoreException;

	public List<DIDURL> selectCredentials(String did, String id,
			String[] type) throws MalformedDIDException,
			MalformedDIDURLException, DIDStoreException {
		DID _did = new DID(did);
		return selectCredentials(_did, new DIDURL(_did, id), type);
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
		DID _did = new DID(did);
		return containsPrivateKey(_did, new DIDURL(_did, id));
	}

	public abstract void storePrivateKey(DID did, DIDURL id, String privateKey)
			throws DIDStoreException;

	public void storePrivateKey(String did, String id, String privateKey)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		DID _did = new DID(did);
		storePrivateKey(_did, new DIDURL(_did, id), privateKey);
	}

	protected abstract String loadPrivateKey(DID did, DIDURL id)
			throws DIDStoreException;

	public abstract boolean deletePrivateKey(DID did, DIDURL id)
			throws DIDStoreException;

	public boolean deletePrivateKey(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		DID _did = new DID(did);
		return deletePrivateKey(_did, new DIDURL(_did, id));
	}

	public String sign(DID did, DIDURL id, String storepass, byte[] ... data)
			throws DIDStoreException {
		if (did == null || storepass == null || storepass.isEmpty() || data == null)
			throw new IllegalArgumentException();

		if (id == null) {
			try {
				DIDDocument doc = resolveDid(did);
				if (doc == null)
					throw new DIDStoreException("Can not resolve DID document.");

				id = doc.getDefaultPublicKey();
			} catch (MalformedDocumentException e) {
				throw new DIDStoreException(e);
			}
		}

		byte[] binKey = decryptFromBase64(storepass, loadPrivateKey(did, id));
		HDKey.DerivedKey key = HDKey.DerivedKey.deserialize(binKey);

		byte[] sig = EcdsaSigner.sign(key.getPrivateKeyBytes(), data);

		key.wipe();

		return Base64.encodeToString(sig,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
	}

	public String sign(DID did, String storepass, byte[] ... data)
			throws DIDStoreException {
		return sign(did, null, storepass, data);
	}
}
