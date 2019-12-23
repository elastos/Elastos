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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.net.URL;

import org.elastos.did.exception.DIDException;
import org.elastos.did.wordlists.UserDefinedWordLists;
import org.junit.Test;

public class MnemonicTest {
	@Test
	public void testBuiltinWordList() throws DIDException {
		for (int i = 0; i < 6; i++) {
			String mnemonic = Mnemonic.generate(i);
			assertTrue(Mnemonic.isValid(i, mnemonic));

			// Try to use the mnemonic create root identity.
			TestData testData = new TestData();
			DIDStore store = testData.setupStore(true);
	    	store.initPrivateIdentity(i, mnemonic,
	    			TestConfig.passphrase, TestConfig.storePass, true);

			mnemonic = mnemonic + "z";
			assertFalse(Mnemonic.isValid(i, mnemonic));
		}
	}

	@Test
	public void testUserDefinedWordList() throws DIDException {
		URL url = this.getClass().getResource("/wordlists");
		File dir = new File(url.getPath());

		UserDefinedWordLists.initialize(dir);

		for (int i = 128; i < 131; i++) {
			String mnemonic = Mnemonic.generate(i);
			assertTrue(Mnemonic.isValid(i, mnemonic));

			// Try to use the mnemonic create root identity.
			TestData testData = new TestData();
			DIDStore store = testData.setupStore(true);
	    	store.initPrivateIdentity(i, mnemonic,
	    			TestConfig.passphrase, TestConfig.storePass, true);

			mnemonic = mnemonic + "z";
			assertFalse(Mnemonic.isValid(i, mnemonic));
		}
	}
}
