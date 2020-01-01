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

import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.MalformedCredentialException;
import org.elastos.did.exception.MalformedDocumentException;
import org.elastos.did.meta.CredentialMeta;
import org.elastos.did.meta.DIDMeta;

public interface DIDStorage {
	public interface ReEncryptor {
		public String reEncrypt(String data) throws DIDStoreException;
	};

	// Root private identity
	public boolean containsPrivateIdentity() throws DIDStoreException;

	public void storePrivateIdentity(String key) throws DIDStoreException;

	public String loadPrivateIdentity() throws DIDStoreException;

	public void storePrivateIdentityIndex(int index) throws DIDStoreException;

	public int loadPrivateIdentityIndex() throws DIDStoreException;

	public void storeMnemonic(String mnemonic) throws DIDStoreException;

	public String loadMnemonic() throws DIDStoreException;

	// DIDs
	public void storeDidMeta(DID did, DIDMeta meta) throws DIDStoreException;

	public DIDMeta loadDidMeta(DID did) throws DIDStoreException;

	public void storeDid(DIDDocument doc) throws DIDStoreException;

	public DIDDocument loadDid(DID did)
			throws MalformedDocumentException, DIDStoreException;

	public boolean containsDid(DID did) throws DIDStoreException;

	public boolean deleteDid(DID did) throws DIDStoreException;

	public List<DID> listDids(int filter) throws DIDStoreException;

	// Credentials
	public void storeCredentialMeta(DID did, DIDURL id, CredentialMeta meta)
			throws DIDStoreException;

	public CredentialMeta loadCredentialMeta(DID did, DIDURL id)
			throws DIDStoreException;

	public void storeCredential(VerifiableCredential credential)
			throws DIDStoreException;

	public VerifiableCredential loadCredential(DID did, DIDURL id)
			throws MalformedCredentialException, DIDStoreException;

	public boolean containsCredentials(DID did) throws DIDStoreException;

	public boolean containsCredential(DID did, DIDURL id)
			throws DIDStoreException;

	public boolean deleteCredential(DID did, DIDURL id)
			throws DIDStoreException;

	public List<DIDURL> listCredentials(DID did) throws DIDStoreException;

	public List<DIDURL> selectCredentials(DID did, DIDURL id, String[] type)
			throws DIDStoreException;

	// Private keys
	public void storePrivateKey(DID did, DIDURL id, String privateKey)
			throws DIDStoreException;

	public String loadPrivateKey(DID did, DIDURL id)
			throws DIDStoreException;

	public boolean containsPrivateKeys(DID did) throws DIDStoreException;

	public boolean containsPrivateKey(DID did, DIDURL id)
			throws DIDStoreException;

	public boolean deletePrivateKey(DID did, DIDURL id)
			throws DIDStoreException;

	public void changePassword(ReEncryptor reEncryptor)
			throws DIDStoreException;
}
