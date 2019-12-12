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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.elastos.credential.MalformedCredentialException;
import org.elastos.credential.VerifiableCredential;

/*
 * FileSystem DID Store: storage layout
 *
 *  + DIDStore root
 *    - .DIDStore						[Store tag file, include magic and version]
 *    + private							[Personal root private key for HD identity]
 *      - key							[HD root private key]
 *      - index							[Last derive index]
 *    + ids
 *      - .ixxxxxxxxxxxxxxx0.meta		[Meta for DID, alias only, OPTIONAL]
 *      + ixxxxxxxxxxxxxxx0 			[DID root, named by id specific string]
 *        - document					[DID document, json format]
 *        + credentials				[Credentials root, OPTIONAL]
 *          - credential-id-0			[Credential, json format, named by id' fragment]
 *          - .credential-id-0.meta	[Meta for credential, alias only, OPTONAL]
 *          - ...
 *          - credential-id-N
 *          - .credential-id-N.meta
 *        + privatekeys				[Private keys root, OPTIONAL]
 *          - privatekey-id-0			[Encrypted private key, named by pk' id]
 *          - ...
 *          - privatekey-id-N
 *
 *      ......
 *
 *      - .ixxxxxxxxxxxxxxxN.meta
 *      + ixxxxxxxxxxxxxxxN
 *
 */
class FileSystemStore extends DIDStore {
	private static final String TAG_FILE = ".DIDStore";
	private static final byte[] TAG_MAGIC = { 0x00, 0x0D, 0x01, 0x0D };
	private static final byte[] TAG_VERSION = { 0x00, 0x00, 0x00, 0x01 };
	private static final int TAG_SIZE = 8;

	private static final String PRIVATE_DIR = "private";
	private static final String HDKEY_FILE = "key";
	private static final String INDEX_FILE = "index";

	private static final String DID_DIR = "ids";
	private static final String DOCUMENT_FILE = "document";
	private static final String CREDENTIALS_DIR = "credentials";
	private static final String PRIVATEKEYS_DIR = "privatekeys";
	private static final String META_EXT = ".meta";

	private static final String DEFAULT_CHARSET = "UTF-8";

	private File storeRoot;

	FileSystemStore(String dir) throws DIDStoreException {
		if (dir == null)
			throw new IllegalArgumentException();

		storeRoot = new File(dir);

		if (storeRoot.exists())
			checkStore();
		else
			initializeStore();
	}

	private void initializeStore() throws DIDStoreException {
		try {
			storeRoot.mkdirs();

			File file = getFile(TAG_FILE);
			file.createNewFile();

			OutputStream out = new FileOutputStream(file);
			out.write(TAG_MAGIC);
			out.write(TAG_VERSION);
			out.close();
		} catch (IOException e) {
			throw new DIDStoreException("Initialize DIDStore \""
					+ storeRoot.getAbsolutePath() + "\" error.", e);
		}
	}

	private void checkStore() throws DIDStoreException {
		if (storeRoot.isFile())
			throw new DIDStoreException("Store root \""
					+ storeRoot.getAbsolutePath() + "\" is a file.");

		File file = getFile(TAG_FILE);
		if (!file.exists() || !file.isFile())
			throw new DIDStoreException("Directory \""
					+ storeRoot.getAbsolutePath() + "\" is not a DIDStore.");

		if (file.length() != TAG_SIZE)
			throw new DIDStoreException("Directory \""
					+ storeRoot.getAbsolutePath() + "\" is not a DIDStore.");

		InputStream in = null;
		byte[] magic = new byte[4];
		byte[] version = new byte[4];

		try {
			in = new FileInputStream(file);
			in.read(magic);
			in.read(version);
			in.close();
		} catch (IOException e) {
			throw new DIDStoreException("Check DIDStore \""
					+ storeRoot.getAbsolutePath() + "\" error.", e);
		}

		if (!Arrays.equals(TAG_MAGIC, magic))
			throw new DIDStoreException("Directory \""
					+ storeRoot.getAbsolutePath() + "\" is not a DIDStore.");

		if (!Arrays.equals(TAG_VERSION, version))
			throw new DIDStoreException("DIDStore \""
					+ storeRoot.getAbsolutePath() + "\", unsupported version.");
	}

	private static void deleteFile(File file) {
		if (file.isDirectory()) {
			File[] children = file.listFiles();
			for (File child : children)
				deleteFile(child);
		}

		file.delete();
	}

	private File getFile(String ... path) {
		try {
			return getFile(false, path);
		} catch (IOException ignore) {
			// Dead code
			return null;
		}
	}

	private File getFile(boolean create, String ... path) throws IOException {
		StringBuffer relPath = new StringBuffer(256);
		File file;

		relPath.append(storeRoot.getAbsolutePath());
		int lastIndex = path.length - 1;
		for (int i = 0; i <= lastIndex; i++) {
			relPath.append(File.separator);
			relPath.append(path[i]);

			if (create) {
				boolean isDir = (i < lastIndex);

				file = new File(relPath.toString());
				if (file.exists() && file.isDirectory() != isDir)
					deleteFile(file);
			}
		}

		file = new File(relPath.toString());
		if (create) {
			file.getParentFile().mkdirs();
			file.createNewFile();
		}

		return file;
	}

	private File getDir(String ... path) {
		StringBuffer relPath = new StringBuffer(256);

		relPath.append(storeRoot.getAbsolutePath());
		for (String p : path) {
			relPath.append(File.separator);
			relPath.append(p);
		}

		return new File(relPath.toString());
	}

	private void writeText(File file, String text) throws IOException {
		FileWriter writer = null;
		try {
			writer = new FileWriter(file);
			writer.write(text);
		} finally {
			if (writer != null)
				writer.close();
		}
	}

	private String readText(File file) throws IOException {
		String text = null;

		if (!file.exists()) {
			return null;
		} else {
			BufferedReader reader = null;
			try {
				reader = new BufferedReader(new FileReader(file));
				text = reader.readLine();
			} finally {
				if (reader != null)
					reader.close();
			}
		}

		return text;
	}

	private File getHDPrivateKeyFile(boolean create) throws IOException {
		return getFile(create, PRIVATE_DIR, HDKEY_FILE);
	}

	private File getHDPrivateKeyFile() {
		try {
			return getHDPrivateKeyFile(false);
		} catch (IOException ignore) {
			// createNewFile will throws IOException.
			// but in this case, createNewFile will never be called.
			return null;
		}
	}

	@Override
	public boolean hasPrivateIdentity() {
		File file = getHDPrivateKeyFile();
		return file.exists() && file.length() > 0;
	}

	@Override
	protected void storePrivateIdentity(String key) throws DIDStoreException {
		try {
			File file = getHDPrivateKeyFile(true);
			writeText(file, key);
		} catch (IOException e) {
			throw new DIDStoreException("Store private identity error.", e);
		}
	}

	@Override
	protected String loadPrivateIdentity() throws DIDStoreException {
		try {
			File file = getHDPrivateKeyFile();
			return readText(file);
		} catch (IOException e) {
			throw new DIDStoreException("Load private identity error.", e);
		}
	}

	@Override
	protected void storePrivateIdentityIndex(int index) throws DIDStoreException {
		try {
			File file = getFile(true, PRIVATE_DIR, INDEX_FILE);
			writeText(file, Integer.toString(index));
		} catch (IOException e) {
			throw new DIDStoreException("Store private identity index error.", e);
		}
	}

	@Override
	protected int loadPrivateIdentityIndex() throws DIDStoreException {
		try {
			File file = getFile(PRIVATE_DIR, INDEX_FILE);
			return Integer.valueOf(readText(file));
		} catch (IOException e) {
			throw new DIDStoreException("Load private identity error.", e);
		}
	}

	@Override
	public void setDidAlias(DID did, String alias) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		try {
			File file = getFile(true, DID_DIR,
					"." + did.getMethodSpecificId() + META_EXT);

			if (alias == null || alias.isEmpty())
				file.delete();
			else
				writeText(file, alias);
		} catch (IOException e) {
			throw new DIDStoreException("Write alias error.", e);
		}
	}

	@Override
	public String getDidAlias(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		try {
			File file = getFile(DID_DIR, "." + did.getMethodSpecificId()
					+ META_EXT);
			return readText(file);
		} catch (IOException e) {
			throw new DIDStoreException("Read alias error.", e);
		}
	}

	@Override
	public void storeDid(DIDDocument doc, String alias) throws DIDStoreException {
		if (doc == null)
			throw new IllegalArgumentException();

		try {
			File file = getFile(true, DID_DIR,
					doc.getSubject().getMethodSpecificId(), DOCUMENT_FILE);
			boolean exist = file.length() > 0;

			doc.toJson(new FileOutputStream(file), DEFAULT_CHARSET, true);

			if (!exist || (alias != null && !alias.isEmpty()))
				setDidAlias(doc.getSubject(), alias);
		} catch (IOException e) {
			throw new DIDStoreException("Store DIDDocument error.", e);
		}
	}

	@Override
	public DIDDocument loadDid(DID did) throws MalformedDocumentException, DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		try {
			File file = getFile(DID_DIR,
					did.getMethodSpecificId(), DOCUMENT_FILE);
			if (!file.exists())
				return null;

			return DIDDocument.fromJson(new FileInputStream(file));
		} catch (IOException e) {
			throw new DIDStoreException("Load DIDDocument error.", e);
		}
	}

	@Override
	public boolean containsDid(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		File file = getFile(DID_DIR,
					did.getMethodSpecificId(), DOCUMENT_FILE);
		return file.exists();
	}

	@Override
	public boolean deleteDid(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		// Delete .did.meta
		File file = getFile(DID_DIR,
				"." + did.getMethodSpecificId() + META_EXT);
		if (file.exists())
			file.delete();

		// Delete did folder
		File dir = getDir(DID_DIR, did.getMethodSpecificId());
		if (dir.exists()) {
			deleteFile(dir);
			return true;
		} else {
			return false;
		}
	}

	@Override
	public List<DID> listDids(int filter) throws DIDStoreException {
		File dir = getDir(DID_DIR);
		if (!dir.exists())
			return new ArrayList<DID>(0);

		File[] children = dir.listFiles(new FileFilter() {
			@Override
			public boolean accept(File file) {
				if (!file.isDirectory())
					return false;

				boolean hasPrivateKey = false;
				try {
					hasPrivateKey = containsPrivateKeys(file);
				} catch (Exception ignore) {
				}

				if (filter == DID_HAS_PRIVATEKEY) {
					return hasPrivateKey;
				} else if (filter == DID_NO_PRIVATEKEY) {
					return !hasPrivateKey;
				} else if (filter == DID_ALL) {
					return true;
				}

				return false;
			}
		});

		int size = children != null ? children.length : 0;
		ArrayList<DID> dids = new ArrayList<DID>(size);

		for (File didRoot : children) {
			DID did = new DID(DID.METHOD, didRoot.getName());
			try {
				did.setAliasInternal(getDidAlias(did));
			} catch (DIDStoreException ignore) {
			}

			dids.add(did);
		}

		return dids;
	}

	@Override
	public void setCredentialAlias(DID did, DIDURL id, String alias)
			throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		try {
			File file = getFile(true, DID_DIR, did.getMethodSpecificId(),
					CREDENTIALS_DIR, "." + id.getFragment() + META_EXT);

			if (alias == null || alias.isEmpty())
				file.delete();
			else
				writeText(file, alias);
		} catch (IOException e) {
			throw new DIDStoreException("Write alias error.", e);
		}
	}

	@Override
	public String getCredentialAlias(DID did, DIDURL id)
			throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		try {
			File file = getFile(DID_DIR, did.getMethodSpecificId(),
					CREDENTIALS_DIR, "." + id.getFragment() + META_EXT);
			return readText(file);
		} catch (IOException e) {
			throw new DIDStoreException("Read alias error.", e);
		}
	}

	@Override
	public void storeCredential(VerifiableCredential credential, String alias)
			throws DIDStoreException {
		if (credential == null)
			throw new IllegalArgumentException();

		try {
			File file = getFile(true, DID_DIR,
					credential.getSubject().getId().getMethodSpecificId(),
					CREDENTIALS_DIR, credential.getId().getFragment());
			boolean exist = file.length() > 0;

			credential.toJson(new FileOutputStream(file), true);

			if (!exist || (alias != null && !alias.isEmpty()))
				setCredentialAlias(credential.getSubject().getId(),
						credential.getId(), alias);
		} catch (IOException e) {
			throw new DIDStoreException("Store credential error.", e);
		}
	}

	@Override
	public VerifiableCredential loadCredential(DID did, DIDURL id)
			throws MalformedCredentialException, DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		try {
			File file = getFile(DID_DIR, did.getMethodSpecificId(),
					CREDENTIALS_DIR, id.getFragment());
			if (!file.exists())
				return null;

			return VerifiableCredential.fromJson(new FileInputStream(file));
		} catch (IOException e) {
			throw new DIDStoreException("Load VerifiableCredential error.", e);
		}
	}

	@Override
	public boolean containsCredentials(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		File dir = getDir(DID_DIR, did.getMethodSpecificId(), CREDENTIALS_DIR);
		if (!dir.exists())
			return false;

		File[] creds = dir.listFiles(new FileFilter() {
			@Override
			public boolean accept(File file) {
				if (file.getName().startsWith("."))
					return false;
				else
					return true;
			}
		});

		return creds == null ? false : creds.length > 0;
	}

	@Override
	public boolean containsCredential(DID did, DIDURL id) throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		File file = getFile(DID_DIR, did.getMethodSpecificId(),
				CREDENTIALS_DIR, id.getFragment());
		return file.exists();
	}

	@Override
	public boolean deleteCredential(DID did, DIDURL id) throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		File file = getFile(DID_DIR, did.getMethodSpecificId(),
				CREDENTIALS_DIR, "." + id.getFragment() + META_EXT);

		if (file.exists())
			file.delete();

		file = getFile(DID_DIR, did.getMethodSpecificId(),
				CREDENTIALS_DIR, id.getFragment());
		if (file.exists()) {
			file.delete();
			return true;
		} else {
			return false;
		}
	}

	@Override
	public List<DIDURL> listCredentials(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		File dir = getDir(DID_DIR, did.getMethodSpecificId(), CREDENTIALS_DIR);
		if (!dir.exists())
			return new ArrayList<DIDURL>(0);

		File[] children = dir.listFiles(new FileFilter() {
			@Override
			public boolean accept(File file) {
				if (file.isDirectory())
					return false;

				if (file.getName().startsWith("."))
					return false;

				return true;
			}
		});

		int size = children != null ? children.length : 0;
		ArrayList<DIDURL> credentials = new ArrayList<DIDURL>(size);

		for (File credential : children) {
			DIDURL id = new DIDURL(did, credential.getName());
			try {
				id.setAliasInternal(getCredentialAlias(did, id));
			} catch (DIDStoreException ignore) {
			}

			credentials.add(id);
		}

		return credentials;
	}

	@Override
	public List<DIDURL> selectCredentials(DID did, DIDURL id, String[] type)
			throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		if ((id == null) && (type == null || type.length == 0))
			throw new IllegalArgumentException();

		// TODO:
		return null;
	}

	private boolean containsPrivateKeys(File didDir) throws DIDStoreException {
		DID did = new DID(DID.METHOD, didDir.getName());
		return containsPrivateKeys(did);
	}

	@Override
	public boolean containsPrivateKeys(DID did) throws DIDStoreException {
		if (did == null)
			throw new IllegalArgumentException();

		File dir = getDir(DID_DIR, did.getMethodSpecificId(), PRIVATEKEYS_DIR);
		if (!dir.exists())
			return false;

		File[] keys = dir.listFiles(new FileFilter() {
			@Override
			public boolean accept(File file) {
				if (file.getName().startsWith("."))
					return false;
				else
					return true;
			}
		});

		return keys == null ? false : keys.length > 0;
	}

	@Override
	public boolean containsPrivateKey(DID did, DIDURL id) throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		File file = getFile(DID_DIR, did.getMethodSpecificId(),
				PRIVATEKEYS_DIR, id.getFragment());
		return file.exists();
	}

	@Override
	public void storePrivateKey(DID did, DIDURL id, String privateKey) throws DIDStoreException {
		if (did == null || id == null || privateKey == null || privateKey.isEmpty())
			throw new IllegalArgumentException();

		try {
			File file = getFile(true, DID_DIR, did.getMethodSpecificId(),
					PRIVATEKEYS_DIR, id.getFragment());
			writeText(file, privateKey);
		} catch (IOException e) {
			throw new DIDStoreException("Store private key error.", e);
		}
	}

	@Override
	public boolean deletePrivateKey(DID did, DIDURL id) throws DIDStoreException {
		if (did == null || id == null)
			throw new IllegalArgumentException();

		File file = getFile(DID_DIR, did.getMethodSpecificId(),
				PRIVATEKEYS_DIR, id.getFragment());
		if (file.exists()) {
			file.delete();
			return true;
		} else {
			return false;
		}
	}

	@Override
	protected String loadPrivateKey(DID did, DIDURL id) throws DIDStoreException {
		try {
			File file = getFile(DID_DIR, did.getMethodSpecificId(),
					PRIVATEKEYS_DIR, id.getFragment());
			return readText(file);
		} catch (Exception e) {
			throw new DIDStoreException("Load private key error.", e);
		}
	}
}
