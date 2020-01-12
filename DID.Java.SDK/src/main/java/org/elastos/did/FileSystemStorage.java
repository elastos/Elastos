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

import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.MalformedCredentialException;
import org.elastos.did.exception.MalformedDocumentException;
import org.elastos.did.meta.CredentialMeta;
import org.elastos.did.meta.DIDMeta;

/*
 * FileSystem DID Store: storage layout
 *
 *  + DIDStore root
 *    - .meta						    [Store meta file, include magic and version]
 *    + private							[Personal root private key for HD identity]
 *      - key							[HD root private key]
 *      - index							[Last derive index]
 *    + ids
 *      + ixxxxxxxxxxxxxxx0 			[DID root, named by id specific string]
 *        - .meta						[Meta for DID, json format, OPTIONAL]
 *        - document					[DID document, json format]
 *        + credentials				    [Credentials root, OPTIONAL]
 *          + credential-id-0           [Credential root, named by id' fragment]
 *            - .meta					[Meta for credential, json format, OPTONAL]
 *            - credential				[Credential, json format]
 *          + ...
 *          + credential-id-N
 *            - .meta
 *            - credential
 *        + privatekeys				    [Private keys root, OPTIONAL]
 *          - privatekey-id-0			[Encrypted private key, named by pk' id]
 *          - ...
 *          - privatekey-id-N
 *
 *      ......
 *
 *      + ixxxxxxxxxxxxxxxN
 *
 */
class FileSystemStorage implements DIDStorage {
	private static final byte[] STORE_MAGIC = { 0x00, 0x0D, 0x01, 0x0D };
	private static final byte[] STORE_VERSION = { 0x00, 0x00, 0x00, 0x02 };
	private static final int STORE_META_SIZE = 8;

	private static final String PRIVATE_DIR = "private";
	private static final String HDKEY_FILE = "key";
	private static final String INDEX_FILE = "index";
	private static final String MNEMONIC_FILE = "mnemonic";

	private static final String DID_DIR = "ids";
	private static final String DOCUMENT_FILE = "document";
	private static final String CREDENTIALS_DIR = "credentials";
	private static final String CREDENTIAL_FILE = "credential";
	private static final String PRIVATEKEYS_DIR = "privatekeys";

	private static final String META_FILE = ".meta";

	private static final String JOURNAL_SUFFIX = ".journal";
	private static final String DEPRECATED_SUFFIX = ".deprecated";

	private static final String DEFAULT_CHARSET = "UTF-8";

	private File storeRoot;

	FileSystemStorage(String dir) throws DIDStoreException {
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

			File file = getFile(META_FILE);
			file.createNewFile();

			OutputStream out = new FileOutputStream(file);
			out.write(STORE_MAGIC);
			out.write(STORE_VERSION);
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

		File file = getFile(META_FILE);
		if (!file.exists() || !file.isFile())
			throw new DIDStoreException("Directory \""
					+ storeRoot.getAbsolutePath() + "\" is not a DIDStore.");

		if (file.length() != STORE_META_SIZE)
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

		if (!Arrays.equals(STORE_MAGIC, magic))
			throw new DIDStoreException("Directory \""
					+ storeRoot.getAbsolutePath() + "\" is not a DIDStore.");

		if (!Arrays.equals(STORE_VERSION, version))
			throw new DIDStoreException("DIDStore \""
					+ storeRoot.getAbsolutePath() + "\", unsupported version.");

		postChangePassword();
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
	public boolean containsPrivateIdentity() {
		File file = getHDPrivateKeyFile();
		return file.exists() && file.length() > 0;
	}

	@Override
	public void storePrivateIdentity(String key) throws DIDStoreException {
		try {
			File file = getHDPrivateKeyFile(true);
			writeText(file, key);
		} catch (IOException e) {
			throw new DIDStoreException("Store private identity error.", e);
		}
	}

	@Override
	public String loadPrivateIdentity() throws DIDStoreException {
		try {
			File file = getHDPrivateKeyFile();
			return readText(file);
		} catch (IOException e) {
			throw new DIDStoreException("Load private identity error.", e);
		}
	}

	@Override
	public void storePrivateIdentityIndex(int index) throws DIDStoreException {
		try {
			File file = getFile(true, PRIVATE_DIR, INDEX_FILE);
			writeText(file, Integer.toString(index));
		} catch (IOException e) {
			throw new DIDStoreException("Store private identity index error.", e);
		}
	}

	@Override
	public int loadPrivateIdentityIndex() throws DIDStoreException {
		try {
			File file = getFile(PRIVATE_DIR, INDEX_FILE);
			return Integer.valueOf(readText(file));
		} catch (IOException e) {
			throw new DIDStoreException("Load private identity error.", e);
		}
	}

	@Override
	public void storeMnemonic(String mnemonic) throws DIDStoreException {
		try {
			File file = getFile(true, PRIVATE_DIR, MNEMONIC_FILE);
			writeText(file, mnemonic);
		} catch (IOException e) {
			throw new DIDStoreException("Store mnemonic error.", e);
		}
	}

	@Override
	public String loadMnemonic() throws DIDStoreException {
		try {
			File file = getFile(PRIVATE_DIR, MNEMONIC_FILE);
			return readText(file);
		} catch (IOException e) {
			throw new DIDStoreException("Load mnemonic error.", e);
		}
	}


	@Override
	public void storeDidMeta(DID did, DIDMeta meta) throws DIDStoreException {
		try {
			File file = getFile(true, DID_DIR, did.getMethodSpecificId(), META_FILE);
			String metadata = (meta != null && !meta.isEmpty()) ?
					meta.toString() : null;

			if (metadata == null || metadata.isEmpty())
				file.delete();
			else
				writeText(file, metadata);
		} catch (IOException e) {
			throw new DIDStoreException("Write alias error.", e);
		}
	}

	@Override
	public DIDMeta loadDidMeta(DID did) throws DIDStoreException {
		try {
			File file = getFile(DID_DIR, did.getMethodSpecificId(), META_FILE);
			return DIDMeta.fromJson(readText(file));
		} catch (IOException e) {
			throw new DIDStoreException("Read alias error.", e);
		}
	}

	@Override
	public void storeDid(DIDDocument doc) throws DIDStoreException {
		try {
			File file = getFile(true, DID_DIR,
					doc.getSubject().getMethodSpecificId(), DOCUMENT_FILE);

			doc.toJson(new FileOutputStream(file), DEFAULT_CHARSET, true);
		} catch (IOException e) {
			throw new DIDStoreException("Store DIDDocument error.", e);
		}
	}

	@Override
	public DIDDocument loadDid(DID did) throws MalformedDocumentException, DIDStoreException {
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
	public boolean containsDid(DID did) {
		File file = getFile(DID_DIR,
					did.getMethodSpecificId(), DOCUMENT_FILE);
		return file.exists();
	}

	@Override
	public boolean deleteDid(DID did) throws DIDStoreException {
		File dir = getDir(DID_DIR, did.getMethodSpecificId());
		if (dir.exists()) {
			deleteFile(dir);
			return true;
		} else {
			return false;
		}
	}

	@Override
	public List<DID> listDids(int filter) {
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

				if (filter == DIDStore.DID_HAS_PRIVATEKEY) {
					return hasPrivateKey;
				} else if (filter == DIDStore.DID_NO_PRIVATEKEY) {
					return !hasPrivateKey;
				} else if (filter == DIDStore.DID_ALL) {
					return true;
				}

				return false;
			}
		});

		int size = children != null ? children.length : 0;
		ArrayList<DID> dids = new ArrayList<DID>(size);

		for (File didRoot : children) {
			DID did = new DID(DID.METHOD, didRoot.getName());
			dids.add(did);
		}

		return dids;
	}

	@Override
	public void storeCredentialMeta(DID did, DIDURL id, CredentialMeta meta)
			throws DIDStoreException {
		try {
			File file = getFile(true, DID_DIR, did.getMethodSpecificId(),
					CREDENTIALS_DIR, id.getFragment(), META_FILE);
			String metadata = (meta != null && !meta.isEmpty()) ?
					meta.toString() : null;

			if (metadata == null || metadata.isEmpty())
				file.delete();
			else
				writeText(file, metadata);
		} catch (IOException e) {
			throw new DIDStoreException("Write alias error.", e);
		}
	}

	@Override
	public CredentialMeta loadCredentialMeta(DID did, DIDURL id)
			throws DIDStoreException {
		try {
			File file = getFile(DID_DIR, did.getMethodSpecificId(),
					CREDENTIALS_DIR, id.getFragment(), META_FILE);
			return CredentialMeta.fromJson(readText(file));
		} catch (IOException e) {
			throw new DIDStoreException("Read alias error.", e);
		}
	}

	@Override
	public void storeCredential(VerifiableCredential credential)
			throws DIDStoreException {
		try {
			File file = getFile(true, DID_DIR,
					credential.getSubject().getId().getMethodSpecificId(),
					CREDENTIALS_DIR, credential.getId().getFragment(),
					CREDENTIAL_FILE);

			credential.toJson(new FileOutputStream(file), true);
		} catch (IOException e) {
			throw new DIDStoreException("Store credential error.", e);
		}
	}

	@Override
	public VerifiableCredential loadCredential(DID did, DIDURL id)
			throws MalformedCredentialException, DIDStoreException {
		try {
			File file = getFile(DID_DIR, did.getMethodSpecificId(),
					CREDENTIALS_DIR, id.getFragment(), CREDENTIAL_FILE);
			if (!file.exists())
				return null;

			return VerifiableCredential.fromJson(new FileInputStream(file));
		} catch (IOException e) {
			throw new DIDStoreException("Load VerifiableCredential error.", e);
		}
	}

	@Override
	public boolean containsCredentials(DID did) throws DIDStoreException {
		File dir = getDir(DID_DIR, did.getMethodSpecificId(), CREDENTIALS_DIR);
		if (!dir.exists())
			return false;

		File[] creds = dir.listFiles(new FileFilter() {
			@Override
			public boolean accept(File file) {
				if (file.isDirectory())
					return true;
				else
					return false;
			}
		});

		return creds == null ? false : creds.length > 0;
	}

	@Override
	public boolean containsCredential(DID did, DIDURL id) throws DIDStoreException {
		File file = getFile(DID_DIR, did.getMethodSpecificId(),
				CREDENTIALS_DIR, id.getFragment(), CREDENTIAL_FILE);
		return file.exists();
	}

	@Override
	public boolean deleteCredential(DID did, DIDURL id) throws DIDStoreException {
		File dir = getDir(DID_DIR, did.getMethodSpecificId(),
				CREDENTIALS_DIR, id.getFragment());
		if (dir.exists()) {
			deleteFile(dir);
			return true;
		} else {
			return false;
		}
	}

	@Override
	public List<DIDURL> listCredentials(DID did) throws DIDStoreException {
		File dir = getDir(DID_DIR, did.getMethodSpecificId(), CREDENTIALS_DIR);
		if (!dir.exists())
			return new ArrayList<DIDURL>(0);

		File[] children = dir.listFiles(new FileFilter() {
			@Override
			public boolean accept(File file) {
				if (file.isDirectory())
					return true;
				else
					return false;
			}
		});

		int size = children != null ? children.length : 0;
		ArrayList<DIDURL> credentials = new ArrayList<DIDURL>(size);

		for (File credential : children) {
			DIDURL id = new DIDURL(did, credential.getName());
			credentials.add(id);
		}

		return credentials;
	}

	@Override
	public List<DIDURL> selectCredentials(DID did, DIDURL id, String[] type)
			throws DIDStoreException {
		// TODO:
		return null;
	}

	@Override
	public void storePrivateKey(DID did, DIDURL id, String privateKey) throws DIDStoreException {
		try {
			File file = getFile(true, DID_DIR, did.getMethodSpecificId(),
					PRIVATEKEYS_DIR, id.getFragment());
			writeText(file, privateKey);
		} catch (IOException e) {
			throw new DIDStoreException("Store private key error.", e);
		}
	}

	@Override
	public String loadPrivateKey(DID did, DIDURL id) throws DIDStoreException {
		try {
			File file = getFile(DID_DIR, did.getMethodSpecificId(),
					PRIVATEKEYS_DIR, id.getFragment());
			return readText(file);
		} catch (Exception e) {
			throw new DIDStoreException("Load private key error.", e);
		}
	}

	private boolean containsPrivateKeys(File didDir) throws DIDStoreException {
		DID did = new DID(DID.METHOD, didDir.getName());
		return containsPrivateKeys(did);
	}

	@Override
	public boolean containsPrivateKeys(DID did) throws DIDStoreException {
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
		File file = getFile(DID_DIR, did.getMethodSpecificId(),
				PRIVATEKEYS_DIR, id.getFragment());
		return file.exists();
	}

	@Override
	public boolean deletePrivateKey(DID did, DIDURL id) throws DIDStoreException {
		File file = getFile(DID_DIR, did.getMethodSpecificId(),
				PRIVATEKEYS_DIR, id.getFragment());
		if (file.exists()) {
			file.delete();
			return true;
		} else {
			return false;
		}
	}

	private boolean needReencrypt(File file) {
		String[] patterns = {
				"(.+)\\" + File.separator + PRIVATE_DIR + "\\" +
				File.separator + HDKEY_FILE,
				"(.+)\\" + File.separator + PRIVATE_DIR + "\\" +
				File.separator + MNEMONIC_FILE,
				"(.+)\\" + File.separator + DID_DIR + "\\" +
				File.separator + "(.+)" + "\\" + File.separator +
				PRIVATEKEYS_DIR + "\\" + File.separator + "(.+)"
		};

		String path = file.getAbsolutePath();
		for (String pattern : patterns) {
			if (path.matches(pattern))
				return true;
		}

		return false;
	}

	private void copy(File src, File dest, ReEncryptor reEncryptor)
			throws IOException, DIDStoreException {
		if (src.isDirectory()) {
			if (!dest.exists()) {
				dest.mkdir();
			}

			String files[] = src.list();
			for (String file : files) {
				File srcFile = new File(src, file);
				File destFile = new File(dest, file);
				copy(srcFile, destFile, reEncryptor);
			}
		} else {
			if (needReencrypt(src)) {
				String org = readText(src);
				writeText(dest, reEncryptor.reEncrypt(org));
			} else {
			    FileInputStream in = null;
			    FileOutputStream out = null;
			    try {
			        in = new FileInputStream(src);
			        out = new FileOutputStream(dest);
			        out.getChannel().transferFrom(in.getChannel(), 0, in.getChannel().size());
				} finally {
					if (in != null)
						in.close();

					if (out != null)
						out.close();
				}
			}
		}
	}

	private void postChangePassword() {
		File privateDir = getDir(PRIVATE_DIR);
		File privateDeprecated = getDir(PRIVATE_DIR + DEPRECATED_SUFFIX);
		File privateJournal = getDir(PRIVATE_DIR + JOURNAL_SUFFIX);

		File didDir = getDir(DID_DIR);
		File didDeprecated = getDir(DID_DIR + DEPRECATED_SUFFIX);
		File didJournal = getDir(DID_DIR + JOURNAL_SUFFIX);

		File stageFile = getFile("postChangePassword");

		if (stageFile.exists()) {
			if (privateJournal.exists()) {
				if (privateDir.exists())
					privateDir.renameTo(privateDeprecated);

				privateJournal.renameTo(privateDir);
			}

			if (didJournal.exists()) {
				if (didDir.exists())
					didDir.renameTo(didDeprecated);

				didJournal.renameTo(didDir);
			}

			deleteFile(privateDeprecated);
			deleteFile(didDeprecated);
			stageFile.delete();
		} else {
			if (privateJournal.exists())
				deleteFile(privateJournal);

			if (didJournal.exists())
				deleteFile(didJournal);
		}
	}

	@Override
	public void changePassword(ReEncryptor reEncryptor)
			throws DIDStoreException {
		try {
			File privateDir = getDir(PRIVATE_DIR);
			File privateJournal = getDir(PRIVATE_DIR + JOURNAL_SUFFIX);

			File didDir = getDir(DID_DIR);
			File didJournal = getDir(DID_DIR + JOURNAL_SUFFIX);

			copy(privateDir, privateJournal, reEncryptor);
			copy(didDir, didJournal, reEncryptor);

			@SuppressWarnings("unused")
			File stageFile = getFile(true, "postChangePassword");
		} catch (Exception e) {
			throw new DIDStoreException("Change store password failed.");
		} finally {
			postChangePassword();
		}
	}
}
