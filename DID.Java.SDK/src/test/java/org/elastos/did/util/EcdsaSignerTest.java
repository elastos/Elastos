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

package org.elastos.did.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.junit.BeforeClass;
import org.junit.Test;
import org.spongycastle.util.Arrays;

public class EcdsaSignerTest {
	private static final String plain = "The quick brown fox jumps over the lazy dog.";
	private static final String nonce = "testcase";

	private static HDKey.DerivedKey key;
	private static byte[] sig;

	@BeforeClass
	public static void setup() {
		String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);

		HDKey root = HDKey.fromMnemonic(mnemonic, "");
		key = root.derive(0);
	}

	@Test
	public void testSign() {
		sig = EcdsaSigner.sign(key.getPrivateKeyBytes(), plain.getBytes(), nonce.getBytes());

		assertEquals(64, sig.length);
	}

	@Test
	public void testVerify() {
		boolean result = EcdsaSigner.verify(key.getPublicKeyBytes(), plain.getBytes(), nonce.getBytes(), sig);

		assertTrue(result);
	}

	@Test
	public void testVerify1() {
		boolean result = EcdsaSigner.verify(key.getPublicKeyBytes(), (plain + ".").getBytes(), nonce.getBytes(), sig);

		assertFalse(result);
	}

	@Test
	public void testVerify2() {
		byte[] modSig = Arrays.copyOf(sig, sig.length);
		modSig[8] +=1;

		boolean result = EcdsaSigner.verify(key.getPublicKeyBytes(), plain.getBytes(), nonce.getBytes(), modSig);

		assertFalse(result);
	}

	@Test
	public void testVerify3() {
		byte[] modSig = Arrays.copyOf(sig, sig.length);
		modSig[8] +=1;

		boolean result = EcdsaSigner.verify(key.getPublicKeyBytes(), plain.getBytes(), "testcase0".getBytes(), modSig);

		assertFalse(result);
	}

	@Test
	public void testVerify4() {
		byte[] modSig = Arrays.copyOf(sig, sig.length);
		modSig[8] +=1;

		boolean result = EcdsaSigner.verify(key.getPublicKeyBytes(), plain.getBytes(), modSig);

		assertFalse(result);
	}
}
