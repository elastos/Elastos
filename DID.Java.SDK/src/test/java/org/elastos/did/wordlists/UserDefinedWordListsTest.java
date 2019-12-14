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

package org.elastos.did.wordlists;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import java.io.File;
import java.io.IOException;
import java.net.URL;

import org.junit.Test;

import io.github.novacrypto.bip39.WordList;

public class UserDefinedWordListsTest {
	@Test
	public void testLoadWordLists() throws IOException {
		URL url = this.getClass().getResource("/wordlists");
		File dir = new File(url.getPath());

		UserDefinedWordLists.initialize(dir);

		UserDefinedWordLists wls = UserDefinedWordLists.getInstance();
		assertNotNull(wls);

		int language = 128;
		WordList wl = wls.getWordList(language);
		assertNotNull(wl);
		FileWordList fwl = (FileWordList)wl;
		assertEquals(language, fwl.ordinal());
		assertEquals("Czech", fwl.name());
		assertEquals("abdikace", fwl.getWord(0));
		assertEquals("zvyk", fwl.getWord(2047));

		language = 129;
		wl = wls.getWordList(language);
		assertNotNull(wl);
		fwl = (FileWordList)wl;
		assertEquals(language, fwl.ordinal());
		assertEquals("Italian", fwl.name());
		assertEquals("abaco", fwl.getWord(0));
		assertEquals("zuppa", fwl.getWord(2047));

		language = 130;
		wl = wls.getWordList(language);
		assertNotNull(wl);
		fwl = (FileWordList)wl;
		assertEquals(language, fwl.ordinal());
		assertEquals("Korean", fwl.name());
		assertEquals("가격", fwl.getWord(0));
		assertEquals("힘껏", fwl.getWord(2047));

		wl = wls.getWordList(200);
		assertNull(wl);
	}

	@Test(expected = IllegalArgumentException.class)
	public void testLoadFromNonExistDir() throws IOException {
		UserDefinedWordLists.initialize("/tmp/not-exist-dir");
	}

	@Test(expected = IllegalArgumentException.class)
	public void testLoadFromFile() throws IOException {
		URL url = this.getClass().getResource("/testdata/document.json");
		File dir = new File(url.getPath());

		UserDefinedWordLists.initialize(dir);
	}
}
