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

import java.io.IOException;
import java.util.Arrays;
import java.util.List;

import org.elastos.did.DIDDocument.PublicKey;
import org.elastos.did.DIDDocument.Service;
import org.elastos.did.exception.DIDException;
import org.elastos.did.util.HDKey;
import org.junit.Test;

public class DIDDocumentTest {
	@Test
	public void testGetPublicKey() throws IOException, DIDException {
		TestData testData = new TestData();
		testData.setup(true);

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Count and list.
		assertEquals(4, doc.getPublicKeyCount());

		List<PublicKey> pks = doc.getPublicKeys();
		assertEquals(4, pks.size());

		for (PublicKey pk : pks) {
			assertEquals(doc.getSubject(), pk.getId().getDid());
			assertEquals(Constants.DEFAULT_PUBLICKEY_TYPE, pk.getType());

			if (pk.getId().getFragment().equals("recovery"))
				assertNotEquals(doc.getSubject(), pk.getController());
			else
				assertEquals(doc.getSubject(), pk.getController());

			assertTrue(pk.getId().getFragment().equals("primary")
					|| pk.getId().getFragment().equals("key2")
					|| pk.getId().getFragment().equals("key3")
					|| pk.getId().getFragment().equals("recovery"));
		}

		// PublicKey getter.
		PublicKey pk = doc.getPublicKey("primary");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "primary"), pk.getId());

		DIDURL id = new DIDURL(doc.getSubject(), "key2");
		pk = doc.getPublicKey(id);
		assertNotNull(pk);
		assertEquals(id, pk.getId());

		id = doc.getDefaultPublicKey();
		assertNotNull(id);
		assertEquals(new DIDURL(doc.getSubject(), "primary"), id);

		// Key not exist, should fail.
		pk = doc.getPublicKey("notExist");
		assertNull(pk);

		id = new DIDURL(doc.getSubject(), "notExist");
		pk = doc.getPublicKey(id);
		assertNull(pk);

		// Selector
		id = doc.getDefaultPublicKey();
		pks = doc.selectPublicKeys(id, Constants.DEFAULT_PUBLICKEY_TYPE);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "primary"),
				pks.get(0).getId());

		pks = doc.selectPublicKeys(id, null);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "primary"),
				pks.get(0).getId());

		pks = doc.selectPublicKeys((DIDURL) null,
				Constants.DEFAULT_PUBLICKEY_TYPE);
		assertEquals(4, pks.size());

		pks = doc.selectPublicKeys("key2", Constants.DEFAULT_PUBLICKEY_TYPE);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "key2"), pks.get(0).getId());

		pks = doc.selectPublicKeys("key3", null);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "key3"), pks.get(0).getId());
	}

	@Test
	public void testAddPublicKey() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		DIDDocument.Builder db = doc.edit();

		// Add 2 public keys
		DIDURL id = new DIDURL(db.getSubject(), "test1");
		HDKey.DerivedKey key = TestData.generateKeypair();
		boolean success = db.addPublicKey(id, db.getSubject(),
				key.getPublicKeyBase58());
		assertTrue(success);

		key = TestData.generateKeypair();
		success = db.addPublicKey("test2", doc.getSubject().toString(),
				key.getPublicKeyBase58());
		assertTrue(success);

		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Check existence
		PublicKey pk = doc.getPublicKey("test1");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test1"), pk.getId());

		pk = doc.getPublicKey("test2");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "test2"), pk.getId());

		// Check the final count.
		assertEquals(6, doc.getPublicKeyCount());
		assertEquals(3, doc.getAuthenticationKeyCount());
		assertEquals(1, doc.getAuthorizationKeyCount());
	}

	@Test
	public void testRemovePublicKey() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		DIDDocument.Builder db = doc.edit();

		// recovery used by authorization, should failed.
		DIDURL id = new DIDURL(doc.getSubject(), "recovery");
		boolean success = db.removePublicKey(id);
		assertFalse(success);

		// force remove public key, should success
		success = db.removePublicKey(id, true);
		assertTrue(success);

		success = db.removePublicKey("key2", true);
		assertTrue(success);

		// Key not exist, should fail.
		success = db.removePublicKey("notExistKey", true);
		assertFalse(success);

		// Can not remove default publickey, should fail.
		success = db.removePublicKey(doc.getDefaultPublicKey(), true);
		assertFalse(success);

		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Check existence
		PublicKey pk = doc.getPublicKey("recovery");
		assertNull(pk);

		pk = doc.getPublicKey("key2");
		assertNull(pk);

		// Check the final count.
		assertEquals(2, doc.getPublicKeyCount());
		assertEquals(2, doc.getAuthenticationKeyCount());
		assertEquals(0, doc.getAuthorizationKeyCount());
	}

	@Test
	public void testGetAuthenticationKey() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Count and list.
		assertEquals(3, doc.getAuthenticationKeyCount());

		List<PublicKey> pks = doc.getAuthenticationKeys();
		assertEquals(3, pks.size());

		for (PublicKey pk : pks) {
			assertEquals(doc.getSubject(), pk.getId().getDid());
			assertEquals(Constants.DEFAULT_PUBLICKEY_TYPE, pk.getType());

			assertEquals(doc.getSubject(), pk.getController());

			assertTrue(pk.getId().getFragment().equals("primary")
					|| pk.getId().getFragment().equals("key2")
					|| pk.getId().getFragment().equals("key3"));
		}

		// AuthenticationKey getter
		PublicKey pk = doc.getAuthenticationKey("primary");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "primary"), pk.getId());

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
		pks = doc.selectAuthenticationKeys(id,
				Constants.DEFAULT_PUBLICKEY_TYPE);
		assertEquals(1, pks.size());
		assertEquals(id, pks.get(0).getId());

		pks = doc.selectAuthenticationKeys(id, null);
		assertEquals(1, pks.size());
		assertEquals(id, pks.get(0).getId());

		pks = doc.selectAuthenticationKeys((DIDURL) null,
				Constants.DEFAULT_PUBLICKEY_TYPE);
		assertEquals(3, pks.size());

		pks = doc.selectAuthenticationKeys("key2",
				Constants.DEFAULT_PUBLICKEY_TYPE);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "key2"), pks.get(0).getId());

		pks = doc.selectAuthenticationKeys("key2", null);
		assertEquals(1, pks.size());
		assertEquals(new DIDURL(doc.getSubject(), "key2"), pks.get(0).getId());
	}

	@Test
	public void testAddAuthenticationKey() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		DIDDocument.Builder db = doc.edit();

		// Add 2 public keys for test.
		DIDURL id = new DIDURL(db.getSubject(), "test1");
		HDKey.DerivedKey key = TestData.generateKeypair();
		boolean success = db.addPublicKey(id, db.getSubject(),
				key.getPublicKeyBase58());
		assertTrue(success);

		key = TestData.generateKeypair();
		success = db.addPublicKey("test2", doc.getSubject().toString(),
				key.getPublicKeyBase58());
		assertTrue(success);

		// Add by reference
		success = db
				.addAuthenticationKey(new DIDURL(doc.getSubject(), "test1"));
		assertTrue(success);

		success = db.addAuthenticationKey("test2");
		assertTrue(success);

		// Add new keys
		key = TestData.generateKeypair();
		success = db.addAuthenticationKey(new DIDURL(doc.getSubject(), "test3"),
				key.getPublicKeyBase58());
		assertTrue(success);

		key = TestData.generateKeypair();
		success = db.addAuthenticationKey("test4", key.getPublicKeyBase58());
		assertTrue(success);

		// Try to add a non existing key, should fail.
		success = db.addAuthenticationKey("notExistKey");
		assertFalse(success);

		// Try to add a key not owned by self, should fail.
		success = db.addAuthenticationKey("recovery");
		assertFalse(success);

		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Check existence
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

		// Check the final count.
		assertEquals(8, doc.getPublicKeyCount());
		assertEquals(7, doc.getAuthenticationKeyCount());
		assertEquals(1, doc.getAuthorizationKeyCount());
	}

	@Test
	public void testRemoveAuthenticationKey() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		DIDDocument.Builder db = doc.edit();

		// Add 2 public keys for test
		HDKey.DerivedKey key = TestData.generateKeypair();
		boolean success = db.addAuthenticationKey(
				new DIDURL(doc.getSubject(), "test1"),
				key.getPublicKeyBase58());
		assertTrue(success);

		key = TestData.generateKeypair();
		success = db.addAuthenticationKey("test2", key.getPublicKeyBase58());
		assertTrue(success);

		// Remote keys
		success = db
				.removeAuthenticationKey(new DIDURL(doc.getSubject(), "test1"));
		assertTrue(success);

		success = db.removeAuthenticationKey("test2");
		assertTrue(success);

		success = db.removeAuthenticationKey("key2");
		assertTrue(success);

		// Key not exist, should fail.
		success = db.removeAuthenticationKey("notExistKey");
		assertFalse(success);

		// Default publickey, can not remove, should fail.
		success = db.removeAuthenticationKey(doc.getDefaultPublicKey());
		assertFalse(success);

		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Check existence
		PublicKey pk = doc.getAuthenticationKey("test1");
		assertNull(pk);

		pk = doc.getAuthenticationKey("test2");
		assertNull(pk);

		pk = doc.getAuthenticationKey("key2");
		assertNull(pk);

		// Check the final count.
		assertEquals(6, doc.getPublicKeyCount());
		assertEquals(2, doc.getAuthenticationKeyCount());
		assertEquals(1, doc.getAuthorizationKeyCount());
	}

	@Test
	public void testGetAuthorizationKey() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Count and list.
		assertEquals(1, doc.getAuthorizationKeyCount());

		List<PublicKey> pks = doc.getAuthorizationKeys();
		assertEquals(1, pks.size());

		for (PublicKey pk : pks) {
			assertEquals(doc.getSubject(), pk.getId().getDid());
			assertEquals(Constants.DEFAULT_PUBLICKEY_TYPE, pk.getType());

			assertNotEquals(doc.getSubject(), pk.getController());

			assertTrue(pk.getId().getFragment().equals("recovery"));
		}

		// AuthorizationKey getter
		PublicKey pk = doc.getAuthorizationKey("recovery");
		assertNotNull(pk);
		assertEquals(new DIDURL(doc.getSubject(), "recovery"), pk.getId());

		DIDURL id = new DIDURL(doc.getSubject(), "recovery");
		pk = doc.getAuthorizationKey(id);
		assertNotNull(pk);
		assertEquals(id, pk.getId());

		// Key not exist, should fail.
		pk = doc.getAuthorizationKey("notExistKey");
		assertNull(pk);

		id = new DIDURL(doc.getSubject(), "notExistKey");
		pk = doc.getAuthorizationKey(id);
		assertNull(pk);

		// Selector
		id = new DIDURL(doc.getSubject(), "recovery");
		pks = doc.selectAuthorizationKeys(id, Constants.DEFAULT_PUBLICKEY_TYPE);
		assertEquals(1, pks.size());
		assertEquals(id, pks.get(0).getId());

		pks = doc.selectAuthorizationKeys(id, null);
		assertEquals(1, pks.size());
		assertEquals(id, pks.get(0).getId());

		pks = doc.selectAuthorizationKeys((DIDURL) null,
				Constants.DEFAULT_PUBLICKEY_TYPE);
		assertEquals(1, pks.size());
	}

	@Test
	public void testAddAuthorizationKey() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		DIDDocument.Builder db = doc.edit();

		// Add 2 public keys for test.
		DIDURL id = new DIDURL(db.getSubject(), "test1");
		HDKey.DerivedKey key = TestData.generateKeypair();
		boolean success = db.addPublicKey(id,
				new DID(DID.METHOD, key.getAddress()),
				key.getPublicKeyBase58());
		assertTrue(success);

		key = TestData.generateKeypair();
		success = db.addPublicKey("test2",
				new DID(DID.METHOD, key.getAddress()).toString(),
				key.getPublicKeyBase58());
		assertTrue(success);

		// Add by reference
		success = db.addAuthorizationKey(new DIDURL(doc.getSubject(), "test1"));
		assertTrue(success);

		success = db.addAuthorizationKey("test2");
		assertTrue(success);

		// Add new keys
		key = TestData.generateKeypair();
		success = db.addAuthorizationKey(new DIDURL(doc.getSubject(), "test3"),
				new DID(DID.METHOD, key.getAddress()),
				key.getPublicKeyBase58());
		assertTrue(success);

		key = TestData.generateKeypair();
		success = db.addAuthorizationKey("test4",
				new DID(DID.METHOD, key.getAddress()).toString(),
				key.getPublicKeyBase58());
		assertTrue(success);

		// Try to add a non existing key, should fail.
		success = db.addAuthorizationKey("notExistKey");
		assertFalse(success);

		// Try to add key owned by self, should fail.
		success = db.addAuthorizationKey("key2");
		assertFalse(success);

		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertTrue(doc.isValid());

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

		// Check the final key count.
		assertEquals(8, doc.getPublicKeyCount());
		assertEquals(3, doc.getAuthenticationKeyCount());
		assertEquals(5, doc.getAuthorizationKeyCount());
	}

	@Test
	public void testRemoveAuthorizationKey() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		DIDDocument.Builder db = doc.edit();

		// Add 2 keys for test.
		DIDURL id = new DIDURL(db.getSubject(), "test1");
		HDKey.DerivedKey key = TestData.generateKeypair();
		boolean success = db.addAuthorizationKey(id,
				new DID(DID.METHOD, key.getAddress()),
				key.getPublicKeyBase58());
		assertTrue(success);

		key = TestData.generateKeypair();
		success = db.addAuthorizationKey("test2",
				new DID(DID.METHOD, key.getAddress()).toString(),
				key.getPublicKeyBase58());
		assertTrue(success);

		// Remove keys.
		success = db
				.removeAuthorizationKey(new DIDURL(doc.getSubject(), "test1"));
		assertTrue(success);

		success = db.removeAuthorizationKey("recovery");
		assertTrue(success);

		// Key not exist, should fail.
		success = db.removeAuthorizationKey("notExistKey");
		assertFalse(success);

		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Check existence
		PublicKey pk = doc.getAuthorizationKey("test1");
		assertNull(pk);

		pk = doc.getAuthorizationKey("test2");
		assertNotNull(pk);

		pk = doc.getAuthorizationKey("recovery");
		assertNull(pk);

		// Check the final count.
		assertEquals(6, doc.getPublicKeyCount());
		assertEquals(3, doc.getAuthenticationKeyCount());
		assertEquals(1, doc.getAuthorizationKeyCount());
	}

	@Test
	public void testGetCredential() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Count and list.
		assertEquals(2, doc.getCredentialCount());
		List<VerifiableCredential> vcs = doc.getCredentials();
		assertEquals(2, vcs.size());

		for (VerifiableCredential vc : vcs) {
			assertEquals(doc.getSubject(), vc.getId().getDid());
			assertEquals(doc.getSubject(), vc.getSubject().getId());

			assertTrue(vc.getId().getFragment().equals("profile")
					|| vc.getId().getFragment().equals("email"));
		}

		// Credential getter.
		VerifiableCredential vc = doc.getCredential("profile");
		assertNotNull(vc);
		assertEquals(new DIDURL(doc.getSubject(), "profile"), vc.getId());

		vc = doc.getCredential(new DIDURL(doc.getSubject(), "email"));
		assertNotNull(vc);
		assertEquals(new DIDURL(doc.getSubject(), "email"), vc.getId());

		// Credential not exist.
		vc = doc.getCredential("notExistVc");
		assertNull(vc);

		// Credential selector.
		vcs = doc.selectCredentials(new DIDURL(doc.getSubject(), "profile"),
				"SelfProclaimedCredential");
		assertEquals(1, vcs.size());
		assertEquals(new DIDURL(doc.getSubject(), "profile"),
				vcs.get(0).getId());

		vcs = doc.selectCredentials(new DIDURL(doc.getSubject(), "profile"),
				null);
		assertEquals(1, vcs.size());
		assertEquals(new DIDURL(doc.getSubject(), "profile"),
				vcs.get(0).getId());

		vcs = doc.selectCredentials((DIDURL) null, "SelfProclaimedCredential");
		assertEquals(1, vcs.size());
		assertEquals(new DIDURL(doc.getSubject(), "profile"),
				vcs.get(0).getId());

		vcs = doc.selectCredentials((DIDURL) null, "TestingCredential");
		assertEquals(0, vcs.size());
	}

	@Test
	public void testAddCredential() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		DIDDocument.Builder db = doc.edit();

		// Add credentials.
		VerifiableCredential vc = testData.loadPassportCredential();
		boolean success = db.addCredential(vc);
		assertTrue(success);

		vc = testData.loadTwitterCredential();
		success = db.addCredential(vc);
		assertTrue(success);

		// Credential already exist, should fail.
		success = db.addCredential(vc);
		assertFalse(success);

		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Check new added credential.
		vc = doc.getCredential("passport");
		assertNotNull(vc);
		assertEquals(new DIDURL(doc.getSubject(), "passport"), vc.getId());

		DIDURL id = new DIDURL(doc.getSubject(), "twitter");
		vc = doc.getCredential(id);
		assertNotNull(vc);
		assertEquals(id, vc.getId());

		// Should contains 3 credentials.
		assertEquals(4, doc.getCredentialCount());
	}

	@Test
	public void testRemoveCredential() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		DIDDocument.Builder db = doc.edit();

		// Add test credentials.
		VerifiableCredential vc = testData.loadPassportCredential();
		boolean success = db.addCredential(vc);
		assertTrue(success);

		vc = testData.loadTwitterCredential();
		success = db.addCredential(vc);
		assertTrue(success);

		// Remove credentials
		success = db.removeCredential("profile");
		assertTrue(success);

		success = db.removeCredential(new DIDURL(doc.getSubject(), "twitter"));
		assertTrue(success);

		// Credential not exist, should fail.
		success = db.removeCredential("notExistCredential");
		assertFalse(success);

		success = db.removeCredential(
				new DIDURL(doc.getSubject(), "notExistCredential"));
		assertFalse(success);

		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Check existence
		vc = doc.getCredential("profile");
		assertNull(vc);

		vc = doc.getCredential(new DIDURL(doc.getSubject(), "twitter"));
		assertNull(vc);

		// Check the final count.
		assertEquals(2, doc.getCredentialCount());
	}

	@Test
	public void testGetService() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

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
		assertEquals(new DIDURL(doc.getSubject(), "openid"),
				svcs.get(0).getId());

		svcs = doc.selectServices((DIDURL) null, "CarrierAddress");
		assertEquals(1, svcs.size());
		assertEquals(new DIDURL(doc.getSubject(), "carrier"),
				svcs.get(0).getId());

		// Service not exist, should return a empty list.
		svcs = doc.selectServices("notExistService",
				"CredentialRepositoryService");
		assertEquals(0, svcs.size());

		svcs = doc.selectServices((DIDURL) null, "notExistType");
		assertEquals(0, svcs.size());
	}

	@Test
	public void testAddService() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		DIDDocument.Builder db = doc.edit();

		// Add services
		boolean success = db.addService("test-svc-1", "Service.Testing",
				"https://www.elastos.org/testing1");
		assertTrue(success);

		success = db.addService(new DIDURL(doc.getSubject(), "test-svc-2"),
				"Service.Testing", "https://www.elastos.org/testing2");
		assertTrue(success);

		// Service id already exist, should failed.
		success = db.addService("vcr", "test", "https://www.elastos.org/test");
		assertFalse(success);

		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertTrue(doc.isValid());

		// Check the final count
		assertEquals(5, doc.getServiceCount());

		// Try to select new added 2 services
		List<Service> svcs = doc.selectServices((DIDURL) null,
				"Service.Testing");
		assertEquals(2, svcs.size());
		assertEquals("Service.Testing", svcs.get(0).getType());
		assertEquals("Service.Testing", svcs.get(1).getType());
	}

	@Test
	public void testRemoveService() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		DIDDocument.Builder db = doc.edit();

		// remove services
		boolean success = db.removeService("openid");
		assertTrue(success);

		success = db.removeService(new DIDURL(doc.getSubject(), "vcr"));
		assertTrue(success);

		// Service not exist, should fail.
		success = db.removeService("notExistService");
		assertFalse(success);

		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertTrue(doc.isValid());

		Service svc = doc.getService("openid");
		assertNull(svc);

		svc = doc.getService(new DIDURL(doc.getSubject(), "vcr"));
		assertNull(svc);

		// Check the final count
		assertEquals(1, doc.getServiceCount());
	}

	@Test
	public void testParseAndSerializeDocument()
			throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument compact = DIDDocument
				.fromJson(testData.loadTestCompactJson());
		assertNotNull(compact);
		assertTrue(compact.isValid());

		assertEquals(4, compact.getPublicKeyCount());

		assertEquals(3, compact.getAuthenticationKeyCount());
		assertEquals(1, compact.getAuthorizationKeyCount());
		assertEquals(2, compact.getCredentialCount());
		assertEquals(3, compact.getServiceCount());

		DIDDocument normalized = DIDDocument
				.fromJson(testData.loadTestCompactJson());
		assertNotNull(normalized);
		assertTrue(normalized.isValid());

		assertEquals(4, normalized.getPublicKeyCount());

		assertEquals(3, normalized.getAuthenticationKeyCount());
		assertEquals(1, normalized.getAuthorizationKeyCount());
		assertEquals(2, normalized.getCredentialCount());
		assertEquals(3, normalized.getServiceCount());

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		assertEquals(testData.loadTestNormalizedJson(), compact.toString(true));
		assertEquals(testData.loadTestNormalizedJson(),
				normalized.toString(true));
		assertEquals(testData.loadTestNormalizedJson(), doc.toString(true));

		assertEquals(testData.loadTestCompactJson(), compact.toString(false));
		assertEquals(testData.loadTestCompactJson(),
				normalized.toString(false));
		assertEquals(testData.loadTestCompactJson(), doc.toString(false));
	}

	@Test
	public void testSignAndVerify() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		byte[] data = new byte[1024];
		DIDURL pkid = new DIDURL(doc.getSubject(), "primary");

		for (int i = 0; i < 10; i++) {
			Arrays.fill(data, (byte) i);

			String sig = doc.sign(pkid, TestConfig.storePass, data);
			boolean result = doc.verify(pkid, sig, data);
			assertTrue(result);

			data[0] = 0xF;
			result = doc.verify(pkid, sig, data);
			assertFalse(result);

			sig = doc.sign(TestConfig.storePass, data);
			result = doc.verify(sig, data);
			assertTrue(result);

			data[0] = (byte) i;
			result = doc.verify(sig, data);
			assertFalse(result);
		}
	}
}
