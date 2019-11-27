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

import java.io.File;

import org.elastos.did.adapter.SPVAdapter;

public final class TestData {
	private static DID primaryDid;

	public static void setup() throws DIDStoreException {
		//DIDAdapter adapter = new FakeConsoleAdapter();

		DIDAdapter adapter = new SPVAdapter(TestConfig.walletDir,
				TestConfig.walletId, TestConfig.networkConfig,
				TestConfig.resolver, new SPVAdapter.PasswordCallback() {
					@Override
					public String getPassword(String walletDir, String walletId) {
						return TestConfig.walletPassword;
					}
				});

    	TestData.deleteFile(new File(TestConfig.storeRoot));

    	DIDStore.initialize("filesystem", TestConfig.storeRoot, adapter);

    	DIDStore store = DIDStore.getInstance();

    	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
    	store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
    			TestConfig.passphrase, TestConfig.storePass, true);


    	DIDDocument doc = store.newDid(TestConfig.storePass, "Primary ID");
    	primaryDid = doc.getSubject();
	}

	public static DID getPrimaryDid() {
		return primaryDid;
	}

	public static void cleanup() {
		primaryDid = null;
		TestData.deleteFile(new File(TestConfig.storeRoot));
	}

	public static void deleteFile(File file) {
		if (file.isDirectory()) {
			File[] children = file.listFiles();
			for (File child : children)
				deleteFile(child);
		}

		file.delete();
	}
}
