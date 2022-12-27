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

package org.elastos.did.crypto;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;

import org.junit.jupiter.api.Test;

public class Aes256cbcTest {
	private static final String passwd = "secret";
	private static final String plain = "The quick brown fox jumps over the lazy dog.";
	private static final String cipherBase64 = "TBimuq42IyD6FsoZK0AoCOt75uiL_gEepZTpgu59RYSV-NR-fqxsYfx0cyyzGacX";

	@Test
	public void testEncrypt() throws Exception {
		byte[] cipher = Aes256cbc.encrypt(plain.getBytes(), passwd);
		byte[] expected = Base64.decode(cipherBase64,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);

		assertArrayEquals(expected, cipher);
	}

	@Test
	public void testDecrypt() throws Exception {
		byte[] cipher = Base64.decode(cipherBase64,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		byte[] plainBytes = Aes256cbc.decrypt(cipher, passwd);
		byte[] expected = plain.getBytes();

		assertArrayEquals(expected, plainBytes);
	}

	@Test
	public void testEncryptToBase64() throws Exception {
		String cipher = Aes256cbc.encryptToBase64(plain.getBytes(), passwd);

		assertEquals(cipherBase64, cipher);
	}

	@Test
	public void testDecryptFromBase64() throws Exception {
		byte[] plainBytes = Aes256cbc.decryptFromBase64(cipherBase64, passwd);
		byte[] expected = plain.getBytes();

		assertArrayEquals(expected, plainBytes);
	}

	@Test
	public void testCompatibility() throws Exception {
		String plain = "brown bear what do you see";
		String passwd = "password";
		String base64 = "uK7mHw5JHRD2WS-BmA2b_4mUPD9WhttY9uAC_aw9Tdc";

		String cipher = Aes256cbc.encryptToBase64(plain.getBytes(), passwd);

		assertEquals(base64, cipher);
	}
}
