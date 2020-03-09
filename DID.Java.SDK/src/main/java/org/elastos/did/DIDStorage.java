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

import org.elastos.did.exception.DIDStorageException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.meta.CredentialMeta;
import org.elastos.did.meta.DIDMeta;

public interface DIDStorage {
	public interface ReEncryptor {
		public String reEncrypt(String data) throws DIDStoreException;
	};

	// Root private identity
	public boolean containsPrivateIdentity() throws DIDStorageException;

	public void storePrivateIdentity(String key) throws DIDStorageException;

	public String loadPrivateIdentity() throws DIDStorageException;

	public void storePublicIdentity(String key) throws DIDStorageException;

	public String loadPublicIdentity() throws DIDStorageException;

	public void storePrivateIdentityIndex(int index) throws DIDStorageException;

	public int loadPrivateIdentityIndex() throws DIDStorageException;

	public void storeMnemonic(String mnemonic) throws DIDStorageException;

	public String loadMnemonic() throws DIDStorageException;

	// DIDs
	public void storeDidMeta(DID did, DIDMeta meta) throws DIDStorageException;

	public DIDMeta loadDidMeta(DID did) throws DIDStorageException;

	public void storeDid(DIDDocument doc) throws DIDStorageException;

	public DIDDocument loadDid(DID did) throws DIDStorageException;

	public boolean containsDid(DID did) throws DIDStorageException;

	public boolean deleteDid(DID did) throws DIDStorageException;

	public List<DID> listDids(int filter) throws DIDStorageException;

	// Credentials
	public void storeCredentialMeta(DID did, DIDURL id, CredentialMeta meta)
			throws DIDStorageException;

	public CredentialMeta loadCredentialMeta(DID did, DIDURL id)
			throws DIDStorageException;

	public void storeCredential(VerifiableCredential credential)
			throws DIDStorageException;

	public VerifiableCredential loadCredential(DID did, DIDURL id)
			throws DIDStorageException;

	public boolean containsCredentials(DID did) throws DIDStorageException;

	public boolean containsCredential(DID did, DIDURL id)
			throws DIDStorageException;

	public boolean deleteCredential(DID did, DIDURL id)
			throws DIDStorageException;

	public List<DIDURL> listCredentials(DID did) throws DIDStorageException;

	public List<DIDURL> selectCredentials(DID did, DIDURL id, String[] type)
			throws DIDStorageException;

	// Private keys
	public void storePrivateKey(DID did, DIDURL id, String privateKey)
			throws DIDStorageException;

	public String loadPrivateKey(DID did, DIDURL id)
			throws DIDStorageException;

	public boolean containsPrivateKeys(DID did) throws DIDStorageException;

	public boolean containsPrivateKey(DID did, DIDURL id)
			throws DIDStorageException;

	public boolean deletePrivateKey(DID did, DIDURL id)
			throws DIDStorageException;

	public void changePassword(ReEncryptor reEncryptor)
			throws DIDStorageException;
}
