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

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;

import org.elastos.did.backend.ResolverCache;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.MalformedDIDException;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

public class DIDTest {
	private static final String testMethodSpecificID = "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN";
	private static final String testDID = "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN";

	private DID did;

    @BeforeEach
    public void setup() throws MalformedDIDException {
    	did = new DID(testDID);
    }

	@Test
	public void testConstructor() throws MalformedDIDException {
		DID did = new DID(testDID);
		assertEquals(testDID, did.toString());

		did = new DID("did:elastos:1234567890");
		assertEquals("did:elastos:1234567890", did.toString());
	}

	@Test
	public void testConstructorError1() {
		assertThrows(MalformedDIDException.class, () -> {
			new DID("id:elastos:1234567890");
		});
	}

	@Test
	public void testConstructorError2() {
		assertThrows(MalformedDIDException.class, () -> {
			new DID("did:example:1234567890");
		});
	}

	@Test
	public void testConstructorError3() {
		assertThrows(MalformedDIDException.class, () -> {
			new DID("did:elastos:");
		});
	}

	@Test
	public void testGetMethod()  {
		assertEquals(DID.METHOD, did.getMethod());
	}

	@Test
	public void testGetMethodSpecificId() {
		assertEquals(testMethodSpecificID, did.getMethodSpecificId());
	}

	@Test
	public void testToString() {
		assertEquals(testDID, did.toString());
	}

	@Test
	public void testHashCode() throws MalformedDIDException {
		DID other = new DID(testDID);
		assertEquals(did.hashCode(), other.hashCode());

		other = new DID("did:elastos:1234567890");
		assertNotEquals(did.hashCode(), other.hashCode());
	}

	@SuppressWarnings("unlikely-arg-type")
	@Test
	public void testEquals() throws MalformedDIDException {
		DID other = new DID(testDID);
		assertTrue(did.equals(other));
		assertTrue(did.equals(testDID));

		other = new DID("did:elastos:1234567890");
		assertFalse(did.equals(other));
		assertFalse(did.equals("did:elastos:1234567890"));
	}

	@Test
	public void testResolve() throws DIDException, IOException {
		DIDBackend.initialize(TestConfig.resolver, TestData.getResolverCacheDir());
		ResolverCache.reset();

		ArrayList<DID> dids = new ArrayList<DID>(16);

		BufferedReader input = new BufferedReader(new InputStreamReader(
				getClass().getClassLoader().getResourceAsStream("testdata/dids.restore")));
		input.lines().forEach((didstr) -> {
			try {
				DID did = new DID(didstr);
				dids.add(did);
			} catch (MalformedDIDException ignore) {
			}
		});
		input.close();

		long start = System.currentTimeMillis();
		for (DID did : dids) {
			DIDDocument doc = did.resolve();
			assertNotNull(doc);
		}
		long end = System.currentTimeMillis();

		System.out.println(String.format("First time resovle %d DIDs took %d Milliseconds",
				dids.size(), (end-start)));

		start = System.currentTimeMillis();
		for (DID did : dids) {
			DIDDocument doc = did.resolve();
			assertNotNull(doc);
		}
		end = System.currentTimeMillis();

		System.out.println(String.format("Second time resovle %d DIDs took %d Milliseconds",
				dids.size(), (end-start)));
	}

	@Test
	public void testResolveLocal()  throws DIDException, IOException {
		String json = "{\"id\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab\",\"publicKey\":[{\"id\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab#primary\",\"type\":\"ECDSAsecp256r1\",\"controller\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab\",\"publicKeyBase58\":\"21YM84C9hbap4GfFSB3QbjauUfhAN4ETKg2mn4bSqx4Kp\"}],\"authentication\":[\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab#primary\"],\"verifiableCredential\":[{\"id\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab#name\",\"type\":[\"BasicProfileCredential\",\"SelfProclaimedCredential\"],\"issuer\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab\",\"issuanceDate\":\"2020-07-01T00:46:40Z\",\"expirationDate\":\"2025-06-30T00:46:40Z\",\"credentialSubject\":{\"id\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab\",\"name\":\"KP Test\"},\"proof\":{\"type\":\"ECDSAsecp256r1\",\"verificationMethod\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab#primary\",\"signature\":\"jQ1OGwpkYqjxooyaPseqyr_1MncOZDrMS_SvwYzqkCHVrRfjv_b7qfGCjxy7Gbx-LS3bvxZKeMxU1B-k3Ysb3A\"}}],\"expires\":\"2025-07-01T00:46:40Z\",\"proof\":{\"type\":\"ECDSAsecp256r1\",\"created\":\"2020-07-01T00:47:20Z\",\"creator\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab#primary\",\"signatureValue\":\"TOpNt-pWeQDJFaS5EkpMOuCqnZKhPCizf7LYQQDBrNLVIZ_7AR73m-KJk7Aja0wmZWXd7S4n7SC2W4ZQayJlMA\"}}";
		DID did = new DID("did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab");

		DIDBackend.initialize(TestConfig.resolver, TestData.getResolverCacheDir());
		ResolverCache.reset();

		DIDDocument doc = did.resolve();
		assertNull(doc);

		DIDBackend.setResolveHandle((d) -> {
			try {
				if (d.equals(did))
					return DIDDocument.fromJson(json);
			} catch (Exception e) {
			}

			return null;
		});

		doc = did.resolve();
		assertNotNull(doc);
		assertEquals(did, doc.getSubject());

		DIDBackend.setResolveHandle(null);
		doc = did.resolve();
		assertNull(doc);
	}
}
