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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.Arrays;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;

import org.elastos.credential.VerifiableCredential;
import org.elastos.did.util.Base58;
import org.junit.Test;

public class DIDDocumentTest {
	private static DIDStore store;

	private DIDDocument loadTestDocument() throws MalformedDocumentException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream("testdiddoc.json"));
		return DIDDocument.fromJson(input);
	}

	private DIDDocument updateForTesting(DIDDocument doc) throws DIDException {
		doc.modify();

		byte[] keyBytes = new byte[33];
		DIDURL id;
		String keyBase58;
		boolean success;

		for (int i = 0; i < 5; i++) {
			id = new DIDURL(doc.getSubject(), "test-pk-" + i);
			Arrays.fill(keyBytes, (byte)i);
			keyBase58 = Base58.encode(keyBytes);
			success = doc.addPublicKey(id, doc.getSubject(), keyBase58);
			assertTrue(success);
		}

		for (int i = 0; i < 5; i++) {
			id = new DIDURL(doc.getSubject(), "test-auth-" + i);
			Arrays.fill(keyBytes, (byte)(i + 5));
			keyBase58 = Base58.encode(keyBytes);
			success = doc.addAuthenticationKey(id, keyBase58);
			assertTrue(success);
		}

		DID controller = new DID("did:elastos:ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh");
		for (int i = 0; i < 5; i++) {
			id = new DIDURL(doc.getSubject(), "test-autho-" + i);
			Arrays.fill(keyBytes, (byte)(i + 10));
			keyBase58 = Base58.encode(keyBytes);
			success = doc.addAuthorizationKey(id, controller, keyBase58);
			assertTrue(success);
		}

		doc.setReadonly(true);
		return doc;
	}

	@Test
	public void testGetPublicKey() throws DIDException {
		DIDDocument doc = loadTestDocument();
		assertNotNull(doc);

		// Count and list.
		assertEquals(4, doc.getPublicKeyCount());

		List<PublicKey> pks = doc.getPublicKeys();
		assertEquals(4, pks.size());

		for (PublicKey pk : pks) {
			assertEquals(doc.getSubject(), pk.getId().getDid());
			assertEquals(Constants.defaultPublicKeyType, pk.getType());

			if (pk.getId().getFragment().equals("recovery"))
				assertNotEquals(doc.getSubject(), pk.getController());
			else
				assertEquals(doc.getSubject(), pk.getController());

			assertTrue(pk.getId().getFragment().equals("default")
					|| pk.getId().getFragment().equals("key2")
					|| pk.getId().getFragment().equals("key3")
					|| pk.getId().getFragment().equals("recovery"));
		}

		// PublicKey getter.
		PublicKey pk = doc.getPublicKey("default");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "default"), pk.getId());

		DIDURL id = new DIDURL(doc.getSubject(), "key3");
		pk = doc.getPublicKey(id);
		assertNotNull(pk);
		assertEquals(id, pk.getId());

		id = doc.getDefaultPublicKey();
		assertNotNull(id);
		assertEquals(new DIDURL(doc.getSubject(), "default"), id);

		// Key not exist, should fail.
		pk = doc.getPublicKey("notExist");
		assertNull(pk);

		id = new DIDURL(doc.getSubject(), "notExist");
		pk = doc.getPublicKey(id);
		assertNull(pk);

		// Selector
		id = doc.getDefaultPublicKey();
		pks = doc.selectPublicKeys(id, Constants.defaultPublicKeyType);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "default"), pks.get(0).getId());

		pks = doc.selectPublicKeys(id, null);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "default"), pks.get(0).getId());

		pks = doc.selectPublicKeys((DIDURL)null, Constants.defaultPublicKeyType);
		assertEquals(4, pks.size());

		pks = doc.selectPublicKeys("key2", Constants.defaultPublicKeyType);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "key2"), pks.get(0).getId());

		pks = doc.selectPublicKeys("key2", null);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "key2"), pks.get(0).getId());
	}

	@Test
	public void testAddPublicKey() throws DIDException {
		DIDDocument doc = loadTestDocument();
		assertNotNull(doc);

		byte[] keyBytes = new byte[33];
		Arrays.fill(keyBytes, (byte)1);
		String keyBase58 = Base58.encode(keyBytes);

		// Read only mode, should fail.
		DIDURL id = new DIDURL(doc.getSubject(), "test0");
		boolean success = doc.addPublicKey(id, doc.getSubject(), keyBase58);
		assertFalse(success);

		success = doc.addPublicKey("test0", doc.getSubject().toString(), keyBase58);
		assertFalse(success);

		doc.modify();

		// Modification mode, should success.
		id = new DIDURL(doc.getSubject(), "test0");
		success = doc.addPublicKey(id, doc.getSubject(), keyBase58);
		assertTrue(success);

		Arrays.fill(keyBytes, (byte)2);
		keyBase58 = Base58.encode(keyBytes);

		success = doc.addPublicKey("test1", doc.getSubject().toString(), keyBase58);
		assertTrue(success);

		PublicKey pk = doc.getPublicKey("test0");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test0"), pk.getId());

		pk = doc.getPublicKey("test1");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test1"), pk.getId());

		// Check the final count.
		assertEquals(6, doc.getPublicKeyCount());
	}

	@Test
	public void testRemovePublicKey() throws DIDException {
		DIDDocument doc = loadTestDocument();
		assertNotNull(doc);

		updateForTesting(doc);

		// Read only mode, should fail.
		DIDURL id = new DIDURL(doc.getSubject(), "test-pk-0");
		boolean success = doc.removePublicKey(id, true);
		assertFalse(success);

		success = doc.removePublicKey("test-pk-1", true);
		assertFalse(success);

		doc.modify();

		// Modification mode, should success.
		id = new DIDURL(doc.getSubject(), "test-auth-0");
		success = doc.removePublicKey(id, true);
		assertTrue(success);

		success = doc.removePublicKey("test-pk-0");
		assertTrue(success);

		success = doc.removePublicKey("test-autho-0", true);
		assertTrue(success);

		PublicKey pk = doc.getPublicKey("test-auth-0");
		assertNull(pk);

		pk = doc.getPublicKey("test-pk-0");
		assertNull(pk);

		pk = doc.getPublicKey("test-autho-0");
		assertNull(pk);

		// PublicKey used by authentication, can not remove directly, should fail.
		id = new DIDURL(doc.getSubject(), "test-auth-0");
		success = doc.removePublicKey(id);
		assertFalse(success);

		// Key not exist, should fail.
		success = doc.removePublicKey("notExistKey", true);
		assertFalse(success);

		// Can not remove default publickey, should fail.
		success = doc.removePublicKey(doc.getDefaultPublicKey(), true);
		assertFalse(success);

		// Check the final count.
		assertEquals(16, doc.getPublicKeyCount());
		assertEquals(7, doc.getAuthenticationKeyCount());
		assertEquals(5, doc.getAuthorizationKeyCount());
	}

	@Test
	public void testGetAuthenticationKey() throws DIDException {
		DIDDocument doc = loadTestDocument();
		assertNotNull(doc);

		// Count and list.
		assertEquals(3, doc.getAuthenticationKeyCount());

		List<PublicKey> pks = doc.getAuthenticationKeys();
		assertEquals(3, pks.size());

		for (PublicKey pk : pks) {
			assertEquals(doc.getSubject(), pk.getId().getDid());
			assertEquals(Constants.defaultPublicKeyType, pk.getType());

			assertEquals(doc.getSubject(), pk.getController());

			assertTrue(pk.getId().getFragment().equals("default")
					|| pk.getId().getFragment().equals("key2")
					|| pk.getId().getFragment().equals("key3"));
		}

		// AuthenticationKey getter
		PublicKey pk = doc.getAuthenticationKey("default");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "default"), pk.getId());

		DIDURL id = new DIDURL(doc.getSubject(), "key3");
		pk = doc.getAuthenticationKey(id);
		assertNotNull(pk);
		assertEquals(id, pk.getId());

		// Key not exist, should fail.
		pk = doc.getAuthenticationKey("notExist");
		assertNull(pk);

		id = new DIDURL(doc.getSubject(), "notExist");
		pk = doc.getAuthenticationKey(id);
		assertNull(pk);

		// selector
		id = new DIDURL(doc.getSubject(), "key3");
		pks = doc.selectAuthenticationKeys(id, Constants.defaultPublicKeyType);
		assertEquals(1, pks.size());
		assertEquals(id, pks.get(0).getId());

		pks = doc.selectAuthenticationKeys(id, null);
		assertEquals(1, pks.size());
		assertEquals(id, pks.get(0).getId());

		pks = doc.selectAuthenticationKeys((DIDURL)null, Constants.defaultPublicKeyType);
		assertEquals(3, pks.size());

		pks = doc.selectAuthenticationKeys("key2", Constants.defaultPublicKeyType);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "key2"), pks.get(0).getId());

		pks = doc.selectAuthenticationKeys("key2", null);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "key2"), pks.get(0).getId());
	}

	@Test
	public void testAddAuthenticationKey() throws DIDException {
		DIDDocument doc = loadTestDocument();
		assertNotNull(doc);

		// Add the keys for testing.
		doc.modify();

		DIDURL id = new DIDURL(doc.getSubject(), "test1");
		byte[] keyBytes = new byte[33];
		Arrays.fill(keyBytes, (byte)1);
		String keyBase58 = Base58.encode(keyBytes);
		boolean success = doc.addPublicKey(id, doc.getSubject(), keyBase58);
		assertTrue(success);

		id = new DIDURL(doc.getSubject(), "test2");
		Arrays.fill(keyBytes, (byte)2);
		keyBase58 = Base58.encode(keyBytes);
		success = doc.addPublicKey(id, doc.getSubject(), keyBase58);
		assertTrue(success);

		doc.setReadonly(true);

		// Read only mode, shoud fail.
		id = new DIDURL(doc.getSubject(), "test1");
		success = doc.addAuthenticationKey(id);
		assertFalse(success);

		success = doc.addAuthenticationKey("test2");
		assertFalse(success);

		Arrays.fill(keyBytes, (byte)3);
		keyBase58 = Base58.encode(keyBytes);
		success = doc.addAuthenticationKey("test3", keyBase58);
		assertFalse(success);

		Arrays.fill(keyBytes, (byte)4);
		keyBase58 = Base58.encode(keyBytes);
		success = doc.addAuthenticationKey(new DIDURL(doc.getSubject(), "test4"), keyBase58);
		assertFalse(success);

		doc.modify();

		// Modification mode, should success.
		success = doc.addAuthenticationKey(new DIDURL(doc.getSubject(), "test1"));
		assertTrue(success);

		success = doc.addAuthenticationKey("test2");
		assertTrue(success);

		Arrays.fill(keyBytes, (byte)3);
		keyBase58 = Base58.encode(keyBytes);

		success = doc.addAuthenticationKey(new DIDURL(doc.getSubject(), "test3"), keyBase58);
		assertTrue(success);

		Arrays.fill(keyBytes, (byte)4);
		keyBase58 = Base58.encode(keyBytes);

		success = doc.addAuthenticationKey("test4", keyBase58);
		assertTrue(success);

		PublicKey pk = doc.getAuthenticationKey("test1");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test1"), pk.getId());

		pk = doc.getAuthenticationKey("test2");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test2"), pk.getId());

		pk = doc.getAuthenticationKey("test3");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test3"), pk.getId());

		pk = doc.getAuthenticationKey("test4");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test4"), pk.getId());

		// Try to add a non existing key, should fail.
		success = doc.addAuthenticationKey("test0");
		assertFalse(success);

		// Try to add a key not owned by self, should fail.
		success = doc.addAuthenticationKey("recovery");
		assertFalse(success);

		// Check the final count.
		assertEquals(8, doc.getPublicKeyCount());
		assertEquals(7, doc.getAuthenticationKeyCount());

	}

	@Test
	public void testRemoveAuthenticationKey() throws DIDException {
		DIDDocument doc = loadTestDocument();
		assertNotNull(doc);

		updateForTesting(doc);

		// Read only mode, should fail.
		boolean success = doc.removeAuthenticationKey(new DIDURL(doc.getSubject(), "test-auth-0"));
		assertFalse(success);

		success = doc.removeAuthenticationKey("test-auth-1");
		assertFalse(success);

		doc.modify();

		// Modification mode, should success.
		success = doc.removeAuthenticationKey(new DIDURL(doc.getSubject(), "test-auth-0"));
		assertTrue(success);

		success = doc.removeAuthenticationKey("test-auth-1");
		assertTrue(success);

		success = doc.removeAuthenticationKey("key2");
		assertTrue(success);

		PublicKey pk = doc.getAuthenticationKey("test-auth-0");
		assertNull(pk);

		pk = doc.getAuthenticationKey("test-auth-1");
		assertNull(pk);

		pk = doc.getAuthenticationKey("key2");
		assertNull(pk);

		// Key not exist, should fail.
		success = doc.removeAuthenticationKey("test-auth-10");
		assertFalse(success);

		// Default publickey, can not remove, should fail.
		success = doc.removeAuthenticationKey(doc.getDefaultPublicKey());
		assertFalse(success);

		// Check the final count.
		assertEquals(19, doc.getPublicKeyCount());
		assertEquals(5, doc.getAuthenticationKeyCount());
		assertEquals(6, doc.getAuthorizationKeyCount());

		assertEquals(19, doc.getPublicKeyCount());
		assertEquals(5, doc.getAuthenticationKeyCount());
		assertEquals(6, doc.getAuthorizationKeyCount());
	}

	@Test
	public void testGetAuthorizationKey() throws DIDException {
		DIDDocument doc = loadTestDocument();
		assertNotNull(doc);

		updateForTesting(doc);

		// Count and list.
		assertEquals(6, doc.getAuthorizationKeyCount());

		List<PublicKey> pks = doc.getAuthorizationKeys();
		assertEquals(6, pks.size());

		for (PublicKey pk : pks) {
			assertEquals(doc.getSubject(), pk.getId().getDid());
			assertEquals(Constants.defaultPublicKeyType, pk.getType());

			assertNotEquals(doc.getSubject(), pk.getController());

			assertTrue(pk.getId().getFragment().equals("recovery")
					|| pk.getId().getFragment().startsWith("test-autho-"));
		}

		// AuthorizationKey getter
		PublicKey pk = doc.getAuthorizationKey("recovery");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "recovery"), pk.getId());


		DIDURL id = new DIDURL(doc.getSubject(), "test-autho-0");
		pk = doc.getAuthorizationKey(id);
		assertNotNull(pk);
		assertEquals(id, pk.getId());

		// Key not exist, should fail.
		pk = doc.getAuthorizationKey("notExist");
		assertNull(pk);

		id = new DIDURL(doc.getSubject(), "notExist");
		pk = doc.getAuthorizationKey(id);
		assertNull(pk);

		// Selector
		id = new DIDURL(doc.getSubject(), "test-autho-1");
		pks = doc.selectAuthorizationKeys(id, Constants.defaultPublicKeyType);
		assertEquals(1, pks.size());
		assertEquals(id, pks.get(0).getId());

		pks = doc.selectAuthorizationKeys(id, null);
		assertEquals(1, pks.size());
		assertEquals(id, pks.get(0).getId());

		pks = doc.selectAuthorizationKeys((DIDURL)null, Constants.defaultPublicKeyType);
		assertEquals(6, pks.size());

		pks = doc.selectAuthorizationKeys("test-autho-2", Constants.defaultPublicKeyType);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "test-autho-2"), pks.get(0).getId());

		pks = doc.selectAuthorizationKeys("test-autho-2", null);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "test-autho-2"), pks.get(0).getId());
	}

	@Test
	public void testAddAuthorizationKey() throws DIDException {
		DIDDocument doc = loadTestDocument();
		assertNotNull(doc);

		// Add the testing keys
		doc.modify();

		DID controller = new DID("did:elastos:ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh");

		DIDURL id = new DIDURL(doc.getSubject(), "test1");
		byte[] keyBytes = new byte[33];
		Arrays.fill(keyBytes, (byte)1);
		String keyBase58 = Base58.encode(keyBytes);
		boolean success = doc.addPublicKey(id, controller, keyBase58);
		assertTrue(success);

		id = new DIDURL(doc.getSubject(), "test2");
		Arrays.fill(keyBytes, (byte)2);
		keyBase58 = Base58.encode(keyBytes);
		success = doc.addPublicKey(id, controller, keyBase58);
		assertTrue(success);

		doc.setReadonly(true);

		// Read only mode, should fail.
		id = new DIDURL(doc.getSubject(), "test1");
		success = doc.addAuthorizationKey(id);
		assertFalse(success);

		success = doc.addAuthorizationKey("test2");
		assertFalse(success);

		Arrays.fill(keyBytes, (byte)3);
		keyBase58 = Base58.encode(keyBytes);
		success = doc.addAuthorizationKey("test3", controller.toString(), keyBase58);
		assertFalse(success);

		Arrays.fill(keyBytes, (byte)4);
		keyBase58 = Base58.encode(keyBytes);
		success = doc.addAuthorizationKey(new DIDURL(doc.getSubject(), "test4"),
				controller, keyBase58);
		assertFalse(success);

		doc.modify();

		// Modification mode, should success.
		success = doc.addAuthorizationKey(new DIDURL(doc.getSubject(), "test1"));
		assertTrue(success);

		success = doc.addAuthorizationKey("test2");
		assertTrue(success);

		Arrays.fill(keyBytes, (byte)3);
		keyBase58 = Base58.encode(keyBytes);

		success = doc.addAuthorizationKey(new DIDURL(doc.getSubject(), "test3"),
				controller, keyBase58);
		assertTrue(success);

		Arrays.fill(keyBytes, (byte)4);
		keyBase58 = Base58.encode(keyBytes);

		success = doc.addAuthorizationKey("test4", controller.toString(), keyBase58);
		assertTrue(success);

		PublicKey pk = doc.getAuthorizationKey("test1");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test1"), pk.getId());

		pk = doc.getAuthorizationKey("test2");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test2"), pk.getId());

		pk = doc.getAuthorizationKey("test3");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test3"), pk.getId());

		pk = doc.getAuthorizationKey("test4");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test4"), pk.getId());

		// Try to add a non existing key, should fail.
		success = doc.addAuthorizationKey("test0");
		assertFalse(success);

		// Try to add key owned by self, should fail.
		success = doc.addAuthorizationKey("key2");
		assertFalse(success);

		// Check the final key count.
		assertEquals(8, doc.getPublicKeyCount());
		assertEquals(5, doc.getAuthorizationKeyCount());
	}

	@Test
	public void testRemoveAuthorizationKey() throws DIDException {
        DIDDocument doc = loadTestDocument();
        assertNotNull(doc);

        updateForTesting(doc);

        // Read only mode, should fail.
        boolean success = doc.removeAuthorizationKey(new DIDURL(doc.getSubject(), "test-autho-0"));
        assertFalse(success);

        success = doc.removeAuthorizationKey("test-autho-1");
        assertFalse(success);

        doc.modify();

        // Modification mode, should success.
        success = doc.removeAuthorizationKey(new DIDURL(doc.getSubject(), "test-autho-0"));
        assertTrue(success);

        success = doc.removeAuthorizationKey("test-autho-1");
        assertTrue(success);

        success = doc.removeAuthorizationKey("recovery");
        assertTrue(success);

        PublicKey pk = doc.getAuthorizationKey("test-autho-0");
        assertNull(pk);

        pk = doc.getAuthorizationKey("test-autho-1");
        assertNull(pk);

        pk = doc.getAuthorizationKey("recovery");
        assertNull(pk);

        // Key not exist, should fail.
        success = doc.removeAuthorizationKey("test-autho-10");
        assertFalse(success);

        // Check the final count.
        assertEquals(19, doc.getPublicKeyCount());
        assertEquals(8, doc.getAuthenticationKeyCount());
        assertEquals(3, doc.getAuthorizationKeyCount());
	}

	@Test
	public void testGetCredential() throws DIDException {
        DIDDocument doc = loadTestDocument();
        assertNotNull(doc);

        // Count and list.
        assertEquals(2, doc.getCredentialCount());
        List<VerifiableCredential> vcs = doc.getCredentials();
        assertEquals(2, vcs.size());

        for (VerifiableCredential vc : vcs) {
			assertEquals(doc.getSubject(), vc.getId().getDid());
			assertEquals(doc.getSubject(), vc.getSubject().getId());

			assertTrue(vc.getId().getFragment().startsWith("credential-"));
		}

        // Credential getter.
        VerifiableCredential vc = doc.getCredential("credential-1");
        assertNotNull(vc);
		assertEquals(new DIDURL(doc.getSubject(), "credential-1"), vc.getId());

        vc = doc.getCredential(new DIDURL(doc.getSubject(), "credential-2"));
        assertNotNull(vc);
		assertEquals(new DIDURL(doc.getSubject(), "credential-2"), vc.getId());

		// Credential not exist.
        vc = doc.getCredential("credential-3");
        assertNull(vc);

        // TODO: Credential selector.
        vcs = doc.selectCredentials(new DIDURL(doc.getSubject(), "credential-1"), "SelfProclaimedCredential");
        assertEquals(1, vcs.size());
		assertEquals(new DIDURL(doc.getSubject(), "credential-1"), vcs.get(0).getId());

        vcs = doc.selectCredentials(new DIDURL(doc.getSubject(), "credential-1"), null);
        assertEquals(1, vcs.size());
		assertEquals(new DIDURL(doc.getSubject(), "credential-1"), vcs.get(0).getId());

        vcs = doc.selectCredentials((DIDURL)null, "SelfProclaimedCredential");
        assertEquals(1, vcs.size());
		assertEquals(new DIDURL(doc.getSubject(), "credential-1"), vcs.get(0).getId());

        vcs = doc.selectCredentials((DIDURL)null, "TestingCredential");
        assertEquals(0, vcs.size());
	}

	@Test
	public void testAddCredential() throws DIDException {
		String json = "{\"id\":\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#test-cred\",\"type\":[\"SelfProclaimedCredential\",\"BasicProfileCredential\"],\"issuanceDate\":\"2019-01-01T19:20:18Z\",\"credentialSubject\":{\"id\":\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\",\"purpose\": \"Testing\"},\"proof\":{\"type\":\"ECDSAsecp256r1\",\"verificationMethod\":\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#master-key\",\"signature\":\"pYw8XNi1..Cky6Ed=\"}}";

		DIDDocument doc = loadTestDocument();
        assertNotNull(doc);

        // Read only mode, should fail.
        VerifiableCredential vc = VerifiableCredential.fromJson(json);
        boolean success = doc.addCredential(vc);
        assertFalse(success);

        doc.modify();

        // Modification mode, should success.
        success = doc.addCredential(vc);
        assertTrue(success);

        // Credential already exist, should fail.
        success = doc.addCredential(vc);
        assertFalse(success);

        // Check new added credential.
        vc = doc.getCredential("test-cred");
        assertNotNull(vc);
        assertEquals(new DIDURL(doc.getSubject(), "test-cred"), vc.getId());

        // Should contains 3 credentials.
        assertEquals(3, doc.getCredentialCount());
	}

	@Test
	public void testRemoveCredential() throws DIDException {
		DIDDocument doc = loadTestDocument();
        assertNotNull(doc);

        // Read only mode, should fail.
        boolean success = doc.removeCredential("credential-1");
        assertFalse(success);

        success = doc.removeCredential(new DIDURL(doc.getSubject(), "credential-2"));
        assertFalse(success);

        doc.modify();

        // Modification mode, should success.
        success = doc.removeCredential("credential-1");
        assertTrue(success);

        success = doc.removeCredential(new DIDURL(doc.getSubject(), "credential-2"));
        assertTrue(success);

        VerifiableCredential vc = doc.getCredential("credential-1");
        assertNull(vc);

        vc = doc.getCredential(new DIDURL(doc.getSubject(), "credential-2"));
        assertNull(vc);

        // Credential not exist, should fail.
        success = doc.removeCredential("notExistCredential-1");
        assertFalse(success);

        success = doc.removeCredential(new DIDURL(doc.getSubject(), "notExistCredential-2"));
        assertFalse(success);

        // Should no credentials in the document.
        assertEquals(0, doc.getCredentialCount());
	}

	@Test
	public void testGetService() throws DIDException {
        DIDDocument doc = loadTestDocument();
        assertNotNull(doc);

        // Count and list
        assertEquals(3, doc.getServiceCount());
        List<Service> svcs = doc.getServices();
        assertEquals(3, svcs.size());

        for (Service svc : svcs) {
            assertEquals(doc.getSubject(), svc.getId().getDid());

			assertTrue(svc.getId().getFragment().equals("openid")
					|| svc.getId().getFragment().equals("vcr")
					|| svc.getId().getFragment().equals("carrier"));
        }

        // Service getter, should success.
        Service svc = doc.getService("openid");
        assertNotNull(svc);
        assertEquals(new DIDURL(doc.getSubject(), "openid"), svc.getId());
        assertEquals("OpenIdConnectVersion1.0Service", svc.getType());
        assertEquals("https://openid.example.com/", svc.getServiceEndpoint());

        svc = doc.getService(new DIDURL(doc.getSubject(), "vcr"));
        assertNotNull(svc);
        assertEquals(new DIDURL(doc.getSubject(), "vcr"), svc.getId());

        // Service not exist, should fail.
        svc = doc.getService("notExistService");
        assertNull(svc);

        // Service selector.
        svcs = doc.selectServices("vcr", "CredentialRepositoryService");
        assertEquals(1, svcs.size());
        assertEquals(new DIDURL(doc.getSubject(), "vcr"), svcs.get(0).getId());

        svcs = doc.selectServices(new DIDURL(doc.getSubject(), "openid"), null);
        assertEquals(1, svcs.size());
        assertEquals(new DIDURL(doc.getSubject(), "openid"), svcs.get(0).getId());

        svcs = doc.selectServices((DIDURL)null, "CarrierAddress");
        assertEquals(1, svcs.size());
        assertEquals(new DIDURL(doc.getSubject(), "carrier"), svcs.get(0).getId());

        // Service not exist, should return a empty list.
        svcs = doc.selectServices("notExistService", "CredentialRepositoryService");
        assertEquals(0, svcs.size());

        svcs = doc.selectServices((DIDURL)null, "notExistType");
        assertEquals(0, svcs.size());
	}

	@Test
	public void testAddService() throws DIDException {
        DIDDocument doc = loadTestDocument();
        assertNotNull(doc);

        // Read only mode. should fail.
        boolean success = doc.addService("test-svc-0",
        		"Service.Testing", "https://www.elastos.org/testing0");
        assertFalse(success);

        success = doc.addService(new DIDURL(doc.getSubject(), "test-svc-1"),
        		"Service.Testing", "https://www.elastos.org/testing1");
        assertFalse(success);

        doc.modify();

        // Modification mode. should success.
        success = doc.addService("test-svc-0",
        		"Service.Testing", "https://www.elastos.org/testing0");
        assertTrue(success);

        success = doc.addService(new DIDURL(doc.getSubject(), "test-svc-1"),
        		"Service.Testing", "https://www.elastos.org/testing1");
        assertTrue(success);

        // Service id already exist, should failed.
        success = doc.addService("vcr", "test", "https://www.elastos.org/test");
        assertFalse(success);

        assertEquals(5, doc.getServiceCount());

        // Try to select new added 2 services
        List<Service> svcs = doc.selectServices((DIDURL)null, "Service.Testing");
        assertEquals(2, svcs.size());
        assertEquals("Service.Testing", svcs.get(0).getType());
	}

	@Test
	public void testRemoveService() throws DIDException {
		DIDDocument doc = loadTestDocument();
        assertNotNull(doc);

        // Read only mode, should fail.
        boolean success = doc.removeService("openid");
        assertFalse(success);

        success = doc.removeService(new DIDURL(doc.getSubject(), "vcr"));
        assertFalse(success);

        doc.modify();

        // Modification mode, should success.
        success = doc.removeService("openid");
        assertTrue(success);

        success = doc.removeService(new DIDURL(doc.getSubject(), "vcr"));
        assertTrue(success);

        Service svc = doc.getService("openid");
        assertNull(svc);

        svc = doc.getService(new DIDURL(doc.getSubject(), "vcr"));
        assertNull(svc);

        // Service not exist, should fail.
        success = doc.removeService("notExistService");
        assertFalse(success);

        assertEquals(1, doc.getServiceCount());
	}

	@Test
	public void testParseDocument() throws DIDException {
		DIDDocument doc = loadTestDocument();
		assertNotNull(doc);

		assertEquals(4, doc.getPublicKeyCount());

		assertEquals(3, doc.getAuthenticationKeyCount());
		assertEquals(1, doc.getAuthorizationKeyCount());
		assertEquals(2, doc.getCredentialCount());
		assertEquals(3, doc.getServiceCount());
	}

	@Test
	public void testCompactJson() throws DIDException, IOException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream("testdiddoc.json"));
		DIDDocument doc = DIDDocument.fromJson(input);
		input.close();

		String json = doc.toExternalForm(true);

		File file = new File(getClass().getClassLoader().getResource("compact.json").getFile());
		char[] chars = new char[(int)file.length()];
		input = new InputStreamReader(new FileInputStream(file));
		input.read(chars);
		input.close();

		String expected = new String(chars);
		assertEquals(expected, json);
	}

	@Test
	public void testNormalizedJson() throws DIDException, IOException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream("testdiddoc.json"));
		DIDDocument doc = DIDDocument.fromJson(input);
		input.close();

		String json = doc.toExternalForm(false);

		File file = new File(getClass().getClassLoader().getResource("normalized.json").getFile());
		char[] chars = new char[(int)file.length()];
		input = new InputStreamReader(new FileInputStream(file));
		input.read(chars);
		input.close();

		String expected = new String(chars);
		assertEquals(expected, json);
	}

	@Test
	public void test31SignAndVerify() throws DIDException {
		TestData.deleteFile(new File(TestConfig.storeRoot));
    	DIDStore.initialize("filesystem", TestConfig.storeRoot,
    			new FakeConsoleAdapter());
    	store = DIDStore.getInstance();
    	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
    	store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
    			TestConfig.passphrase, TestConfig.storePass, true);

    	LinkedHashMap<DID, String> ids = new LinkedHashMap<DID, String>(128);
    	for (int i = 0; i < 10; i++) {
    		String hint = "my did " + i;
    		DIDDocument doc = store.newDid(TestConfig.storePass, hint);

	    	File file = new File(TestConfig.storeRoot + File.separator + "ids"
	    			+ File.separator + doc.getSubject().getMethodSpecificId()
	    			+ File.separator + "document");
	    	assertTrue(file.exists());
	    	assertTrue(file.isFile());

	    	file = new File(TestConfig.storeRoot + File.separator + "ids"
	    			+ File.separator + "."
	    			+ doc.getSubject().getMethodSpecificId() + ".meta");
	    	assertTrue(file.exists());
	    	assertTrue(file.isFile());

	    	ids.put(doc.getSubject(), hint);
    	}

    	Iterator<DID> dids = ids.keySet().iterator();
		while (dids.hasNext()) {
			DID did = dids.next();

			DIDDocument doc = DIDStore.getInstance().loadDid(did);
			String json = doc.toExternalForm(false);
			DIDURL pkid = new DIDURL(did, "primary");

			String sig = doc.sign(pkid, TestConfig.storePass, json.getBytes());
			boolean result = doc.verify(pkid, sig, json.getBytes());
			assertTrue(result);

			result = doc.verify(pkid, sig, json.substring(1).getBytes());
			assertFalse(result);

			sig = doc.sign(TestConfig.storePass, json.getBytes());
			result = doc.verify(sig, json.getBytes());
			assertTrue(result);

			result = doc.verify(sig, json.substring(1).getBytes());
			assertFalse(result);
		}
	}
}
