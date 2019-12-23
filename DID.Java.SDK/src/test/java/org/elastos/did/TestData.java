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
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;

import org.elastos.did.adapter.DummyAdapter;
import org.elastos.did.adapter.SPVAdapter;
import org.elastos.did.backend.ResolverCache;
import org.elastos.did.exception.DIDException;
import org.elastos.did.util.Base58;
import org.elastos.did.util.HDKey;

public final class TestData {
	private static DummyAdapter dummyAdapter;
	private static DIDAdapter spvAdapter;
	private static HDKey rootKey;
	private static int index;

	private DIDDocument testIssuer;
	private String issuerCompactJson;
	private String issuerNormalizedJson;

	private DIDDocument testDocument;
	private String testCompactJson;
	private String testNormalizedJson;

	private VerifiableCredential profileVc;
	private String profileVcCompactJson;
	private String profileVcNormalizedJson;

	private VerifiableCredential emailVc;
	private String emailVcCompactJson;
	private String emailVcNormalizedJson;

	private VerifiableCredential passportVc;
	private String passportVcCompactJson;
	private String passportVcNormalizedJson;

	private VerifiableCredential twitterVc;
	private String twitterVcCompactJson;
	private String twitterVcNormalizedJson;

	private VerifiablePresentation testVp;
	private String testVpNormalizedJson;

	private String restoreMnemonic;

	private DIDStore store;

	public DIDStore setupStore(boolean dummyBackend) throws DIDException {
		DIDAdapter adapter;

		ResolverCache.reset();

		if (dummyBackend) {
			if (TestData.dummyAdapter == null)
				TestData.dummyAdapter = new DummyAdapter(TestConfig.verbose);
			else
				TestData.dummyAdapter.reset();

			adapter = TestData.dummyAdapter;
		} else {
			if (TestData.spvAdapter == null)
				TestData.spvAdapter = new SPVAdapter(TestConfig.walletDir,
						TestConfig.walletId, TestConfig.networkConfig,
						TestConfig.resolver, new SPVAdapter.PasswordCallback() {
							@Override
							public String getPassword(String walletDir, String walletId) {
								return TestConfig.walletPassword;
							}
						});
			adapter = TestData.spvAdapter;
		}

		DIDBackend.initialize(adapter);
    	deleteFile(new File(TestConfig.storeRoot));
    	store = DIDStore.open("filesystem", TestConfig.storeRoot);
    	return store;
	}

	public String initIdentity() throws DIDException {
    	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
    	store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
    			TestConfig.passphrase, TestConfig.storePass, true);

    	return mnemonic;
	}

	private DIDDocument loadDIDDocument(String fileName)
			throws DIDException, IOException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream("testdata/" + fileName));
		DIDDocument doc = DIDDocument.fromJson(input);
		input.close();

		if (store != null) {
			store.storeDid(doc);
		}

		return doc;
	}

	private void importPrivateKey(DIDURL id, String fileName)
			throws IOException, DIDException {
		String skBase58 = loadText(fileName);
		byte[] sk = Base58.decode(skBase58);

		store.storePrivateKey(id.getDid(), id, sk, TestConfig.storePass);
	}

	public DIDDocument loadTestIssuer() throws DIDException, IOException {
		if (testIssuer == null) {
			testIssuer = loadDIDDocument("issuer.json");

			importPrivateKey(testIssuer.getDefaultPublicKey(), "issuer.primary.sk");

			store.publishDid(testIssuer, TestConfig.storePass);
		}

		return testIssuer;
	}

	public DIDDocument loadTestDocument() throws DIDException, IOException {
		loadTestIssuer();

		if (testDocument == null) {
			testDocument = loadDIDDocument("document.json");

			importPrivateKey(testDocument.getDefaultPublicKey(), "document.primary.sk");
			importPrivateKey(testDocument.getPublicKey("key2").getId(), "document.key2.sk");
			importPrivateKey(testDocument.getPublicKey("key3").getId(), "document.key3.sk");

			store.publishDid(testDocument, TestConfig.storePass);
		}

		return testDocument;
	}

	private VerifiableCredential loadCredential(String fileName)
			throws DIDException, IOException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream("testdata/" + fileName));
		VerifiableCredential vc = VerifiableCredential.fromJson(input);
		input.close();

		if (store != null)
			store.storeCredential(vc);

		return vc;
	}

	public VerifiableCredential loadProfileCredential()
			throws DIDException, IOException {
		if (profileVc == null)
			profileVc = loadCredential("vc-profile.json");

		return profileVc;
	}

	public VerifiableCredential loadEmailCredential()
			throws DIDException, IOException {
		if (emailVc == null)
			emailVc = loadCredential("vc-email.json");

		return emailVc;
	}

	public VerifiableCredential loadPassportCredential()
			throws DIDException, IOException {
		if (passportVc == null)
			passportVc = loadCredential("vc-passport.json");

		return passportVc;
	}

	public VerifiableCredential loadTwitterCredential()
			throws DIDException, IOException {
		if (twitterVc == null)
			twitterVc = loadCredential("vc-twitter.json");

		return twitterVc;
	}

	public VerifiablePresentation loadPresentation()
			throws DIDException, IOException {
		if (testVp == null) {
			Reader input = new InputStreamReader(getClass()
					.getClassLoader().getResourceAsStream("testdata/vp.json"));
			testVp = VerifiablePresentation.fromJson(input);
			input.close();
		}

		return testVp;
	}

	private String loadText(String fileName) throws IOException {
		BufferedReader input = new BufferedReader(new InputStreamReader(
				getClass().getClassLoader().getResourceAsStream("testdata/" + fileName)));
		String text = input.readLine();
		input.close();

		return text;
	}

	public String loadIssuerCompactJson() throws IOException {
		if (issuerCompactJson == null)
			issuerCompactJson = loadText("issuer.compact.json");

		return issuerCompactJson;
	}

	public String loadIssuerNormalizedJson() throws IOException {
		if (issuerNormalizedJson == null)
			issuerNormalizedJson = loadText("issuer.normalized.json");

		return issuerNormalizedJson;
	}

	public String loadTestCompactJson() throws IOException {
		if (testCompactJson == null)
			testCompactJson = loadText("document.compact.json");

		return testCompactJson;
	}

	public String loadTestNormalizedJson() throws IOException {
		if (testNormalizedJson == null)
			testNormalizedJson = loadText("document.normalized.json");

		return testNormalizedJson;
	}

	public String loadProfileVcCompactJson() throws IOException {
		if (profileVcCompactJson == null)
			profileVcCompactJson = loadText("vc-profile.compact.json");

		return profileVcCompactJson;
	}

	public String loadProfileVcNormalizedJson() throws IOException {
		if (profileVcNormalizedJson == null)
			profileVcNormalizedJson = loadText("vc-profile.normalized.json");

		return profileVcNormalizedJson;
	}

	public String loadEmailVcCompactJson() throws IOException {
		if (emailVcCompactJson == null)
			emailVcCompactJson = loadText("vc-email.compact.json");

		return emailVcCompactJson;
	}

	public String loadEmailVcNormalizedJson() throws IOException {
		if (emailVcNormalizedJson == null)
			emailVcNormalizedJson = loadText("vc-email.normalized.json");

		return emailVcNormalizedJson;
	}

	public String loadPassportVcCompactJson() throws IOException {
		if (passportVcCompactJson == null)
			passportVcCompactJson = loadText("vc-passport.compact.json");

		return passportVcCompactJson;
	}

	public String loadPassportVcNormalizedJson() throws IOException {
		if (passportVcNormalizedJson == null)
			passportVcNormalizedJson = loadText("vc-passport.normalized.json");

		return passportVcNormalizedJson;
	}

	public String loadTwitterVcCompactJson() throws IOException {
		if (twitterVcCompactJson == null)
			twitterVcCompactJson = loadText("vc-twitter.compact.json");

		return twitterVcCompactJson;
	}

	public String loadTwitterVcNormalizedJson() throws IOException {
		if (twitterVcNormalizedJson == null)
			twitterVcNormalizedJson = loadText("vc-twitter.normalized.json");

		return twitterVcNormalizedJson;
	}

	public String loadPresentationNormalizedJson() throws IOException {
		if (testVpNormalizedJson == null)
			testVpNormalizedJson = loadText("vp.normalized.json");

		return testVpNormalizedJson;
	}

	public String loadRestoreMnemonic() throws IOException {
		if (restoreMnemonic == null)
			restoreMnemonic = loadText("mnemonic.restore");

		return restoreMnemonic;
	}

	public static void deleteFile(File file) {
		if (file.isDirectory()) {
			File[] children = file.listFiles();
			for (File child : children)
				deleteFile(child);
		}

		file.delete();
	}

	public static synchronized HDKey.DerivedKey generateKeypair()
			throws DIDException {
		if (rootKey == null) {
	    	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
	    	rootKey = HDKey.fromMnemonic(mnemonic, "");
	    	index = 0;
		}

		return rootKey.derive(index++);
	}
}
