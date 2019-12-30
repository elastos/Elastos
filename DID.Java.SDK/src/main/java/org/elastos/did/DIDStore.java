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

import org.elastos.did.DIDDocument.PublicKey;
import org.elastos.did.exception.DIDDeactivatedException;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDExpiredException;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.MalformedCredentialException;
import org.elastos.did.exception.MalformedDIDException;
import org.elastos.did.exception.MalformedDIDURLException;
import org.elastos.did.exception.MalformedDocumentException;
import org.elastos.did.meta.CredentialMeta;
import org.elastos.did.meta.DIDMeta;
import org.elastos.did.util.Aes256cbc;
import org.elastos.did.util.Base64;
import org.elastos.did.util.EcdsaSigner;
import org.elastos.did.util.HDKey;
import org.elastos.did.util.LRUCache;

public final class DIDStore {
	private static final int CACHE_INITIAL_CAPACITY = 16;
	private static final int CACHE_MAX_CAPACITY = 32;

	public static final int DID_HAS_PRIVATEKEY = 0;
	public static final int DID_NO_PRIVATEKEY = 1;
	public static final int DID_ALL	= 2;

	private Map<DID, DIDDocument> didCache;
	private Map<DIDURL, VerifiableCredential> vcCache;

	private DIDStorage storage;

	private DIDStore(int initialCacheCapacity, int maxCacheCapacity,
			DIDStorage storage) {
		if (maxCacheCapacity > 0) {
			this.didCache = LRUCache.createInstance(initialCacheCapacity, maxCacheCapacity);
			this.vcCache = LRUCache.createInstance(initialCacheCapacity, maxCacheCapacity);
		}

		this.storage = storage;
	}

	public static DIDStore open(String type, String location,
			int initialCacheCapacity, int maxCacheCapacity)
			throws DIDStoreException {
		if (type == null || location == null || location.isEmpty() ||
				maxCacheCapacity < initialCacheCapacity)
			throw new IllegalArgumentException();

		if (!type.equals("filesystem"))
			throw new DIDStoreException("Unsupported store type: " + type);

		DIDStorage storage = new FileSystemStorage(location);
		return new DIDStore(initialCacheCapacity, maxCacheCapacity, storage);
	}

	public static DIDStore open(String type, String location)
			throws DIDStoreException {
		return open(type, location, CACHE_INITIAL_CAPACITY, CACHE_MAX_CAPACITY);
	}

	public boolean containsPrivateIdentity() throws DIDStoreException {
		return storage.containsPrivateIdentity();
	}

	protected static String encryptToBase64(byte[] input, String passwd)
			throws DIDStoreException {
		byte[] cipher;
		try {
			cipher = Aes256cbc.encrypt(input, passwd);
		} catch (Exception e) {
			throw new DIDStoreException("Encrypt key error.", e);
		}

		return Base64.encodeToString(cipher,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
	}

	protected static byte[] decryptFromBase64(String input, String storepass)
			throws DIDStoreException {
		byte[] cipher = Base64.decode(input,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		try {
			return Aes256cbc.decrypt(cipher, storepass);
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

		if (containsPrivateIdentity() && !force)
			throw new DIDStoreException("Already has private indentity.");

		if (passphrase == null)
			passphrase = "";

		HDKey privateIdentity = HDKey.fromMnemonic(mnemonic, passphrase);

		// Save seed instead of root private key,
		// keep compatible with Native SDK
		String encryptedIdentity = encryptToBase64(
				privateIdentity.getSeed(), storepass);
		storage.storePrivateIdentity(encryptedIdentity);
		storage.storePrivateIdentityIndex(0);

		// Save mnemonic
		String encryptedMnemonic = encryptToBase64(
				mnemonic.getBytes(), storepass);
		storage.storeMnemonic(encryptedMnemonic);

		privateIdentity.wipe();
	}

	public void initPrivateIdentity(int language, String mnemonic,
			String passphrase, String storepass) throws DIDStoreException {
		initPrivateIdentity(language, mnemonic, passphrase, storepass, false);
	}

	public String exportMnemonic(String storepass) throws DIDStoreException {
		if (storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException("Invalid password.");

		String encryptedMnemonic = storage.loadMnemonic();
		return new String(decryptFromBase64(encryptedMnemonic, storepass));
	}

	// initialized from saved private identity from DIDStore.
	protected HDKey loadPrivateIdentity(String storepass)
			throws DIDStoreException {
		if (!containsPrivateIdentity())
			return null;

		byte[] seed = decryptFromBase64(storage.loadPrivateIdentity(), storepass);
		return HDKey.fromSeed(seed);
	}

	public void synchronize(String storepass)
			throws DIDStoreException, DIDException  {
		if (storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException("Invalid password.");

		HDKey privateIdentity = loadPrivateIdentity(storepass);
		if (privateIdentity == null)
			throw new DIDStoreException("DID Store does not contains private identity.");

		int nextIndex = storage.loadPrivateIdentityIndex();
		int blanks = 0;
		int i = 0;

		while (i < nextIndex || blanks < 20) {
			HDKey.DerivedKey key = privateIdentity.derive(i++);
			DID did = new DID(DID.METHOD, key.getAddress());

			DIDDocument doc = null;
			try {
				doc = DIDBackend.getInstance().resolve(did, true);
			} catch (DIDExpiredException e) {
				continue;
			} catch (DIDDeactivatedException e) {
				continue;
			} catch (DIDResolveException e) {
				// TODO: throws exception if necessary
				blanks++;
				continue;
			}

			if (doc != null) {
				// Save private key
				storePrivateKey(did, doc.getDefaultPublicKey(),
						key.serialize(),storepass);

				storeDid(doc);

				if (i >= nextIndex)
					storage.storePrivateIdentityIndex(i);

				blanks = 0;
			} else {
				blanks++;
			}
		}
	}

	public DIDDocument newDid(String alias, String storepass)
			throws DIDStoreException {
		if (storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException("Invalid password.");

		HDKey privateIdentity = loadPrivateIdentity(storepass);
		if (privateIdentity == null)
			throw new DIDStoreException("DID Store not contains private identity.");

		int nextIndex = storage.loadPrivateIdentityIndex();

		HDKey.DerivedKey key = privateIdentity.derive(nextIndex++);
		DID did = new DID(DID.METHOD, key.getAddress());
		DIDURL id = new DIDURL(did, "primary");

		storePrivateKey(did, id, key.serialize(), storepass);

		DIDDocument.Builder db = new DIDDocument.Builder(did, this);
		db.addAuthenticationKey(id, key.getPublicKeyBase58());
		DIDDocument doc = db.seal(storepass);
		doc.getMeta().setAlias(alias);
		storeDid(doc);

		storage.storePrivateIdentityIndex(nextIndex);
		privateIdentity.wipe();
		key.wipe();

		return doc;
	}

	public DIDDocument newDid(String storepass) throws DIDStoreException {
		return newDid(null, storepass);
	}

	public String publishDid(DID did, DIDURL signKey, String storepass)
			throws DIDStoreException, DIDException {
		if (did == null || storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException();

		DIDDocument doc = loadDid(did);
		if (doc == null)
			throw new DIDStoreException("Can not find the document for " + did);

		if (signKey == null)
			signKey = doc.getDefaultPublicKey();

		return DIDBackend.getInstance().create(doc, signKey, storepass);
	}

	public String publishDid(String did, String signKey, String storepass)
			throws MalformedDIDURLException, DIDStoreException, DIDException {
		DID _did = new DID(did);
		DIDURL _signKey = signKey == null ? null : new DIDURL(_did, signKey);
		return publishDid(_did, _signKey, storepass);
	}

	public String publishDid(DID did, String storepass)
			throws DIDStoreException, DIDException {
		return publishDid(did, (DIDURL)null, storepass);
	}

	public String publishDid(String did, String storepass)
			throws DIDStoreException, DIDException {
		return publishDid(did, (String)null, storepass);
	}

	public String updateDid(DID did, DIDURL signKey, String storepass)
			throws DIDException, DIDStoreException, DIDException {
		if (did == null || storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException();

		DIDDocument doc = loadDid(did);
		if (doc == null)
			throw new DIDStoreException("Can not find the document for " + did);

		if (signKey == null)
			signKey = doc.getDefaultPublicKey();

		return DIDBackend.getInstance().update(doc,
				doc.getTransactionId(), signKey, storepass);
	}

	public String updateDid(String did, String signKey, String storepass)
			throws MalformedDIDURLException, DIDException, DIDStoreException {
		DID _did = new DID(did);
		DIDURL _signKey = signKey == null ? null : new DIDURL(_did, signKey);
		return updateDid(_did, _signKey, storepass);
	}

	public String updateDid(DID did, String storepass)
			throws DIDException, DIDStoreException {
		return updateDid(did, (DIDURL)null, storepass);
	}

	public String updateDid(String did, String storepass)
			throws DIDException, DIDStoreException {
		return updateDid(did, (String)null, storepass);
	}

	// Deactivate self use authentication keys
	public String deactivateDid(DID did, DIDURL signKey, String storepass)
			throws DIDStoreException, DIDException {
		if (did == null || storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException();

		// Document should use the IDChain's copy
		boolean localCopy = false;
		DIDDocument doc;
		try {
			doc = DIDBackend.getInstance().resolve(did);
		} catch (DIDException e) {
			throw e;
		}

		if (doc == null) {
			// Fail-back: try to load document from local store
			doc = loadDid(did);
			if (doc == null)
				throw new DIDStoreException("Can not resolve DID document.");
			else
				localCopy = true;
		} else {
			doc.getMeta().setStore(this);
		}

		if (signKey == null)
			signKey = doc.getDefaultPublicKey();

		String txid = DIDBackend.getInstance().deactivate(doc, signKey, storepass);

		// Save deactivated status to DID metadata
		if (localCopy) {
			doc.getMeta().setDeactivated(true);
			storeDidMeta(did, doc.getMeta());
		}

		return txid;
	}

	public String deactivateDid(String did, String signKey, String storepass)
			throws MalformedDIDURLException, DIDStoreException, DIDException {
		DID _did = new DID(did);
		DIDURL _signKey = signKey == null ? null : new DIDURL(_did, signKey);
		return deactivateDid(_did, _signKey, storepass);
	}

	public String deactivateDid(DID did, String storepass)
			throws DIDStoreException, DIDException {
		return deactivateDid(did, (DIDURL)null, storepass);
	}

	public String deactivateDid(String did, String storepass)
			throws DIDStoreException, DIDException {
		return deactivateDid(did, (String)null, storepass);
	}

	// Deactivate target DID with authorization
	public String deactivateDid(DID target, DID did, DIDURL signKey,
			String storepass) throws DIDStoreException, DIDException {
		if (target == null || did == null ||
				storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException();

		// All documents should use the IDChain's copy
		DIDDocument doc;
		try {
			doc = DIDBackend.getInstance().resolve(did);
		} catch (DIDException e) {
			throw e;
		}

		if (doc == null) {
			// Fail-back: try to load document from local store
			doc = loadDid(did);
			if (doc == null)
				throw new DIDStoreException("Can not resolve DID document.");
		} else {
			doc.getMeta().setStore(this);
		}

		PublicKey signPk = null;
		if (signKey != null) {
			signPk = doc.getAuthenticationKey(signKey);
			if (signPk == null)
				throw new DIDException("Not authentication key.");
		}

		DIDDocument targetDoc = DIDBackend.getInstance().resolve(target);
		if (targetDoc == null)
			throw new DIDResolveException("DID " + target + " not exist.");

		if (targetDoc.getAuthorizationKeyCount() == 0)
			throw new DIDException("No authorization.");

		// The authorization key id in the target doc
		DIDURL targetSignKey = null;

		List<PublicKey> authorizationKeys = targetDoc.getAuthorizationKeys();
		matchloop: for (PublicKey targetPk : authorizationKeys) {
			if (!targetPk.getController().equals(did))
				continue;

			if (signPk != null) {
				if (!targetPk.getPublicKeyBase58().equals(signPk.getPublicKeyBase58()))
					continue;

				targetSignKey = targetPk.getId();
				break;
			} else {
				List<PublicKey> pks = doc.getAuthenticationKeys();
				for (PublicKey pk : pks) {
					if (pk.getPublicKeyBase58().equals(targetPk.getPublicKeyBase58())) {
						signPk = pk;
						signKey = signPk.getId();
						targetSignKey = targetPk.getId();
						break matchloop;
					}
				}
			}
		}

		if (targetSignKey == null)
			throw new DIDException("No matched authorization key.");

		return DIDBackend.getInstance().deactivate(target, targetSignKey,
				doc, signKey, storepass);
	}

	public String deactivateDid(String target, String did, String signKey,
			String storepass) throws DIDStoreException, DIDException {
		DID _did = new DID(did);
		DIDURL _signKey = signKey == null ? null : new DIDURL(_did, signKey);
		return deactivateDid(new DID(target), _did, _signKey, storepass);
	}

	public String deactivateDid(DID target, DID did, String storepass)
			throws DIDStoreException, DIDException {
		return deactivateDid(target, did, null, storepass);
	}

	public void storeDid(DIDDocument doc, String alias)
			throws DIDStoreException {
		storeDid(doc);
		doc.setAlias(alias);
	}

	public void storeDid(DIDDocument doc) throws DIDStoreException {
		if (doc == null)
			throw new IllegalArgumentException();

		storage.storeDid(doc);

		// TODO: Check me!!!
		DIDMeta meta = loadDidMeta(doc.getSubject());
		meta.merge(doc.getMeta());
		meta.setStore(this);
		doc.setMeta(meta);

		storage.storeDidMeta(doc.getSubject(), meta);

		for (VerifiableCredential vc : doc.getCredentials())
			storeCredential(vc);

		if (didCache != null)
			didCache.put(doc.getSubject(), doc);
	}

	protected void storeDidMeta(DID did, DIDMeta meta)
			throws DIDStoreException {
		storage.storeDidMeta(did, meta);

		if (didCache != null) {
			DIDDocument doc = didCache.get(did);
			if (doc != null)
				doc.setMeta(meta);
		}
	}

	protected void storeDidMeta(String did, DIDMeta meta)
			throws MalformedDIDException, DIDStoreException {
		storeDidMeta(new DID(did), meta);
	}

	protected DIDMeta loadDidMeta(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		DIDMeta meta = null;
		DIDDocument doc = null;

		if (didCache != null) {
			doc = didCache.get(did);
			if (doc != null) {
				meta = doc.getMeta();
				if (meta != null)
					return meta;
			}
		}

		meta = storage.loadDidMeta(did);
		if (doc != null)
			doc.setMeta(meta);

		return meta;
	}

	protected DIDMeta loadDidMeta(String did)
			throws MalformedDIDException, DIDStoreException {
		return loadDidMeta(new DID(did));
	}

	public DIDDocument loadDid(DID did)
			throws MalformedDocumentException, DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		DIDDocument doc;

		if (didCache != null) {
			doc = didCache.get(did);
			if (doc != null)
				return doc;
		}

		doc = storage.loadDid(did);
		if (doc != null) {
			doc.setMeta(storage.loadDidMeta(did));
			doc.getMeta().setStore(this);
		}

		if (doc != null && didCache != null)
			didCache.put(doc.getSubject(), doc);

		return doc;
	}

	public DIDDocument loadDid(String did)
			throws MalformedDIDException, MalformedDocumentException,
			DIDStoreException {
		return loadDid(new DID(did));
	}

	public boolean containsDid(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		return storage.containsDid(did);
	}

	public boolean containsDid(String did)
			throws MalformedDIDException, DIDStoreException {
		return containsDid(new DID(did));
	}

	public boolean deleteDid(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		return storage.deleteDid(did);
	}

	public boolean deleteDid(String did)
			throws MalformedDIDException, DIDStoreException {
		return deleteDid(new DID(did));
	}

	public List<DID> listDids(int filter) throws DIDStoreException {
		List<DID> dids = storage.listDids(filter);

		for (DID did : dids) {
			DIDMeta meta = loadDidMeta(did);
			meta.setStore(this);
			did.setMeta(meta);
		}

		return dids;
	}

	public void storeCredential(VerifiableCredential credential, String alias)
			throws DIDStoreException {
		credential.getMeta().setAlias(alias);
		storeCredential(credential);
	}

	public void storeCredential(VerifiableCredential credential)
			throws DIDStoreException {
		if (credential == null)
			throw new IllegalArgumentException();

		storage.storeCredential(credential);

		// TODO: Check me!!!
		CredentialMeta meta = loadCredentialMeta(
				credential.getSubject().getId(), credential.getId());
		meta.merge(credential.getMeta());
		meta.setStore(this);
		credential.setMeta(meta);

		credential.getMeta().setStore(this);
		storage.storeCredentialMeta(credential.getSubject().getId(),
				credential.getId(), meta);

		if (vcCache != null)
			vcCache.put(credential.getId(), credential);
	}

	protected void storeCredentialMeta(DID did, DIDURL id, CredentialMeta meta)
			throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		storage.storeCredentialMeta(did, id, meta);

		if (vcCache != null) {
			VerifiableCredential vc = vcCache.get(id);
			if (vc != null) {
				vc.setMeta(meta);
			}
		}
	}

	protected void storeCredentialMeta(String did, String id, CredentialMeta meta)
			throws  MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		DID _did = new DID(did);
		storeCredentialMeta(_did, new DIDURL(_did, id), meta);
	}

	protected CredentialMeta loadCredentialMeta(DID did, DIDURL id)
			throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		CredentialMeta meta = null;
		VerifiableCredential vc = null;

		if (vcCache != null) {
			vc = vcCache.get(id);
			if (vc != null) {
				meta = vc.getMeta();
				if (meta != null)
					return meta;
			}
		}

		meta = storage.loadCredentialMeta(did, id);
		if (vc != null)
			vc.setMeta(meta);

		return meta;
	}

	protected CredentialMeta loadCredentialMeta(String did, String id)
			throws  MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		DID _did = new DID(did);
		return loadCredentialMeta(_did, new DIDURL(_did, id));
	}

	public VerifiableCredential loadCredential(DID did, DIDURL id)
			throws MalformedCredentialException, DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		VerifiableCredential vc;

		if (vcCache != null) {
			vc = vcCache.get(id);
			if (vc != null)
				return vc;
		}

		vc = storage.loadCredential(did, id);
		if (vc != null && vcCache != null)
			vcCache.put(vc.getId(), vc);

		return vc;
	}

	public VerifiableCredential loadCredential(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			MalformedCredentialException, DIDStoreException {
		DID _did = new DID(did);
		return loadCredential(_did, new DIDURL(_did, id));
	}

	public boolean containsCredentials(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		return storage.containsCredentials(did);
	}

	public boolean containsCredentials(String did)
			throws MalformedDIDException, DIDStoreException {
		return containsCredentials(new DID(did));
	}

	public boolean containsCredential(DID did, DIDURL id)
			throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		return storage.containsCredential(did, id);
	}

	public boolean containsCredential(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		DID _did = new DID(did);
		return containsCredential(_did, new DIDURL(_did, id));
	}

	public boolean deleteCredential(DID did, DIDURL id)
			throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		return storage.deleteCredential(did, id);
	}

	public boolean deleteCredential(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		DID _did = new DID(did);
		return deleteCredential(_did, new DIDURL(_did, id));
	}

	public List<DIDURL> listCredentials(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		List<DIDURL> ids = storage.listCredentials(did);

		for (DIDURL id : ids) {
			CredentialMeta meta = loadCredentialMeta(did, id);
			meta.setStore(this);
			id.setMeta(meta);
		}

		return ids;
	}

	public List<DIDURL> listCredentials(String did)
			throws MalformedDIDException, DIDStoreException {
		return listCredentials(new DID(did));
	}

	public List<DIDURL> selectCredentials(DID did, DIDURL id, String[] type)
			throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		if ((id == null) && (type == null || type.length == 0))
			throw new IllegalArgumentException();

		return storage.selectCredentials(did, id, type);
	}

	public List<DIDURL> selectCredentials(String did, String id, String[] type)
			throws MalformedDIDException, MalformedDIDURLException, DIDStoreException {
		DID _did = new DID(did);
		return selectCredentials(_did, new DIDURL(_did, id), type);
	}

	public void storePrivateKey(DID did, DIDURL id, byte[] privateKey,
			String storepass) throws DIDStoreException {
		if (did == null || id == null ||
				privateKey == null || privateKey.length == 0 ||
				storepass == null || storepass.isEmpty())
			throw new IllegalArgumentException();

		String encryptedKey = encryptToBase64(privateKey, storepass);
		storage.storePrivateKey(did, id, encryptedKey);
	}

	public void storePrivateKey(String did, String id, byte[] privateKey,
			String storepass) throws MalformedDIDException,
			MalformedDIDURLException, DIDStoreException {
		DID _did = new DID(did);
		storePrivateKey(_did, new DIDURL(_did, id), privateKey, storepass);
	}

	protected String loadPrivateKey(DID did, DIDURL id)
			throws DIDStoreException {
		return storage.loadPrivateKey(did, id);
	}

	public boolean containsPrivateKeys(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		return storage.containsPrivateKeys(did);
	}

	public boolean containsPrivateKeys(String did)
			throws MalformedDIDException, DIDStoreException {
		return containsPrivateKeys(new DID(did));
	}

	public boolean containsPrivateKey(DID did, DIDURL id)
			throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		return storage.containsPrivateKey(did, id);
	}

	public boolean containsPrivateKey(String did, String id)
			throws MalformedDIDException, MalformedDIDURLException,
			DIDStoreException {
		DID _did = new DID(did);
		return containsPrivateKey(_did, new DIDURL(_did, id));
	}

	public boolean deletePrivateKey(DID did, DIDURL id)
			throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		return storage.deletePrivateKey(did, id);
	}

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
				DIDDocument doc = loadDid(did);
				if (doc == null)
					throw new DIDStoreException("Can not resolve DID document.");

				id = doc.getDefaultPublicKey();
			} catch (MalformedDocumentException e) {
				throw new DIDStoreException(e);
			}
		}

		byte[] binKey = decryptFromBase64(loadPrivateKey(did, id), storepass);
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
