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
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.Signature;

import org.bitcoinj.core.Sha256Hash;
import org.elastos.did.TestData;
import org.elastos.did.exception.DIDException;
import org.junit.jupiter.api.Test;

import io.jsonwebtoken.Claims;
import io.jsonwebtoken.Jws;
import io.jsonwebtoken.Jwts;

public class HDKeyTest {
	// Test HD key algorithm, keep compatible with SPV.

	@Test
	public void test0() {
		String expectedIDString = "iY4Ghz9tCuWvB5rNwvn4ngWvthZMNzEA7U";
		String mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";

		HDKey root = new HDKey(mnemonic, "");
		HDKey key = root.derive(HDKey.DERIVE_PATH_PREFIX + '0');

		assertEquals(expectedIDString, key.getAddress());

		byte[] sk = HDKey.paddingToExtendedPrivateKey(key.getPrivateKeyBytes());
		HDKey rk = HDKey.deserialize(sk);

		assertEquals(key.getPrivateKeyBase58(), rk.getPrivateKeyBase58());
		assertEquals(key.getPublicKeyBase58(), rk.getPublicKeyBase58());
	}

	@Test
	public void test1() {
		String expectedIDString = "iW3HU8fTmwkENeVT9UCEvvg3ddUD5oCxYA";
		String mnemonic = "service illegal blossom voice three eagle grace agent service average knock round";

		HDKey root = new HDKey(mnemonic, "");
		HDKey key = root.derive(HDKey.DERIVE_PATH_PREFIX + '0');

		assertEquals(expectedIDString, key.getAddress());

		byte[] sk = HDKey.paddingToExtendedPrivateKey(key.getPrivateKeyBytes());
		HDKey rk = HDKey.deserialize(sk);

		assertEquals(key.getPrivateKeyBase58(), rk.getPrivateKeyBase58());
		assertEquals(key.getPublicKeyBase58(), rk.getPublicKeyBase58());
	}

	@Test
	public void test2() {
		String mnemonic = "pact reject sick voyage foster fence warm luggage cabbage any subject carbon";
		String passphrase = "helloworld";
		// String seed = "98b9dde6ea5edba3c7808b8a342377bc8b08e8004667bb4be2a229f3bcda8e73b750101f3993272d971a4f50914a91d7221c9bbed964e4778081d9f4523b4525";
		String key = "xprv9s21ZrQH143K4biiQbUq8369meTb1R8KnstYFAKtfwk3vF8uvFd1EC2s49bMQsbdbmdJxUWRkuC48CXPutFfynYFVGnoeq8LJZhfd9QjvUt";

		HDKey root = new HDKey(mnemonic, passphrase);
		assertEquals(key, root.serializeBase58());
		byte[] keyBytes = root.serialize();

		root = HDKey.deserialize(Base58.decode(key));
		assertEquals(key, root.serializeBase58());
		assertArrayEquals(keyBytes, root.serialize());
	}

	@Test
	public void testDerivePublicOnly() {
		String mnemonic = "pact reject sick voyage foster fence warm luggage cabbage any subject carbon";
		String passphrase = "helloworld";

		HDKey root = new HDKey(mnemonic, passphrase);

		HDKey preDerivedKey = root.derive(HDKey.PRE_DERIVED_PUBLICKEY_PATH);
		String preDerivedPubBase58 = preDerivedKey.serializePublicKeyBase58();
		HDKey preDerivedPub = HDKey.deserializeBase58(preDerivedPubBase58);

		for (int i = 0; i < 1000; i++) {
			HDKey key = root.derive(HDKey.DERIVE_PATH_PREFIX + i);

			HDKey keyPubOnly = preDerivedPub.derive("0/" + i);

			assertEquals(key.getPublicKeyBase58(), keyPubOnly.getPublicKeyBase58());
			assertEquals(key.getAddress(), keyPubOnly.getAddress());
		}
	}

	@Test
	public void testJWTCompatible() throws DIDException, GeneralSecurityException {
		byte[] input = "The quick brown fox jumps over the lazy dog.".getBytes();

		for (int i = 0; i < 1000; i++) {
			HDKey key = TestData.generateKeypair();

			// to JCE KeyPair
			KeyPair jceKeyPair = key.getJCEKeyPair();
			Signature jceSigner = Signature.getInstance("SHA256withECDSA");

			byte[] didSig = key.sign(Sha256Hash.hash(input));

			jceSigner.initSign(jceKeyPair.getPrivate());
			jceSigner.update(input);
	        byte[] jceSig = jceSigner.sign();

	        assertTrue(key.verify(Sha256Hash.hash(input), jceSig));

	        jceSigner.initVerify(jceKeyPair.getPublic());
	        jceSigner.update(input);
	        assertTrue(jceSigner.verify(didSig));
		}
	}

	@Test
	public void testJwtES256() throws DIDException {
		for (int i = 0; i < 1000; i++) {
			HDKey key = TestData.generateKeypair();
			KeyPair keypair = key.getJCEKeyPair();

	    	// Build and sign
	    	String token = Jwts.builder()
	    			.setSubject("Hello JWT!")
	    			.claim("name", "abc")
	    			.signWith(keypair.getPrivate())
	    			.compact();

	    	// Parse and verify
	    	@SuppressWarnings("unused")
			Jws<Claims> jws = Jwts.parserBuilder()
	    			.setSigningKey(keypair.getPublic())
	    			.build()
	    			.parseClaimsJws(token);

	    	String[] parts = token.split("\\.");
	    	Boolean success = EcdsaSigner.verifyData(key.getPublicKeyBytes(),
	    			Base64.decode(parts[2], Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP),
	    			parts[0].getBytes(), ".".getBytes(), parts[1].getBytes());
	    	assertTrue(success);
		}
	}
}
