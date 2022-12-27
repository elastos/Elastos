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

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.util.Arrays;

import org.elastos.did.Mnemonic;
import org.elastos.did.exception.DIDException;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;

public class EcdsaSignerTest {
	private static final String plain = "The quick brown fox jumps over the lazy dog.";
	private static final String nonce = "testcase";

	private static HDKey key;
	private static byte[] sig;

	@BeforeAll
	public static void setup() throws DIDException {
		String mnemonic = Mnemonic.getInstance().generate();

		HDKey root = new HDKey(mnemonic, "");
		key = root.derive(HDKey.DERIVE_PATH_PREFIX + 0);

		sig = EcdsaSigner.signData(key.getPrivateKeyBytes(), plain.getBytes(), nonce.getBytes());

		assertEquals(64, sig.length);
	}

	@Test
	public void testVerify() {
		boolean result = EcdsaSigner.verifyData(key.getPublicKeyBytes(), sig, plain.getBytes(), nonce.getBytes());

		assertTrue(result);
	}

	@Test
	public void testVerify1() {
		boolean result = EcdsaSigner.verifyData(key.getPublicKeyBytes(), sig, (plain + ".").getBytes(), nonce.getBytes());

		assertFalse(result);
	}

	@Test
	public void testVerify2() {
		byte[] modSig = Arrays.copyOf(sig, sig.length);
		modSig[8] +=1;

		boolean result = EcdsaSigner.verifyData(key.getPublicKeyBytes(), modSig, plain.getBytes(), nonce.getBytes());

		assertFalse(result);
	}

	@Test
	public void testVerify3() {
		byte[] modSig = Arrays.copyOf(sig, sig.length);
		modSig[8] +=1;

		boolean result = EcdsaSigner.verifyData(key.getPublicKeyBytes(), modSig, plain.getBytes(), "testcase0".getBytes());

		assertFalse(result);
	}

	@Test
	public void testVerify4() {
		byte[] modSig = Arrays.copyOf(sig, sig.length);
		modSig[8] +=1;

		boolean result = EcdsaSigner.verifyData(key.getPublicKeyBytes(), modSig, plain.getBytes());

		assertFalse(result);
	}

	@Test
	public void testCompatibility() {
		String input = "abcdefghijklmnopqrstuvwxyz";
		String pkBase58 = "voHKsUjoPSJSQKWLJHWYzUfEv3NEaRUyJReoZVS6XCYM";
		String expectedSig1 = "SlDq9rsEQJgS83ydi2cPMiwXm6SgJCuwYwx_NqpOwf5IQcbfUM574GHThnvJ5lgTeyeOwVcxbWyQxehlK3MO-A";
		String expectedSig2 = "gm4Bx8ijQjBEFsf1Cm1mHcqSzFHquoQe235uzL3OUDJiIuFnJ49lEWn0RueIfgCZbrDEhLdxKSaNYqnBpjiR6A";

		byte[] pk = Base58.decode(pkBase58);

		byte[] sig = Base64.decode(expectedSig1,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		boolean result = EcdsaSigner.verifyData(pk, sig, input.getBytes());
		assertTrue(result);

		sig = Base64.decode(expectedSig2,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		result = EcdsaSigner.verifyData(pk, sig, input.getBytes());
		assertTrue(result);
	}
}
