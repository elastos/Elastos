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

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;

import org.junit.jupiter.api.Test;

public class HDKeyTest {
	/*
	private static void dumpHex(String prompt, byte[] bytes) {
		System.out.print(prompt + "[" + bytes.length + "]: ");
		for (byte b : bytes)
			System.out.print(String.format("%02x", b));
		System.out.println();
	}
	*/

	// Test HD key algorithm, keep compatible with SPV.

	@Test
	public void test0() {
		String expectedIDString = "iY4Ghz9tCuWvB5rNwvn4ngWvthZMNzEA7U";
		String mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";

		HDKey root = HDKey.fromMnemonic(mnemonic, "");
		HDKey.DerivedKey key = root.derive(0);

		assertEquals(expectedIDString, key.getAddress());
	}


	@Test
	public void test1() {
		String expectedIDString = "iW3HU8fTmwkENeVT9UCEvvg3ddUD5oCxYA";
		String mnemonic = "service illegal blossom voice three eagle grace agent service average knock round";

		HDKey root = HDKey.fromMnemonic(mnemonic, "");
		HDKey.DerivedKey key = root.derive(0);

		assertEquals(expectedIDString, key.getAddress());
	}

	@Test
	public void test2() {
		String mnemonic = "pact reject sick voyage foster fence warm luggage cabbage any subject carbon";
		String passphrase = "helloworld";
		// String seed = "98b9dde6ea5edba3c7808b8a342377bc8b08e8004667bb4be2a229f3bcda8e73b750101f3993272d971a4f50914a91d7221c9bbed964e4778081d9f4523b4525";
		String key = "xprv9s21ZrQH143K4biiQbUq8369meTb1R8KnstYFAKtfwk3vF8uvFd1EC2s49bMQsbdbmdJxUWRkuC48CXPutFfynYFVGnoeq8LJZhfd9QjvUt";

		HDKey root = HDKey.fromMnemonic(mnemonic, passphrase);
		assertEquals(key, Base58.encode(root.serialize()));
		byte[] keyBytes = root.getKeyBytes();

		root = HDKey.deserialize(Base58.decode(key));
		assertEquals(key, Base58.encode(root.serialize()));
		assertArrayEquals(keyBytes, root.getKeyBytes());
	}
}
