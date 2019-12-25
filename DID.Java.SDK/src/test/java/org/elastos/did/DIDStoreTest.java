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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.elastos.did.adapter.DummyAdapter;
import org.elastos.did.exception.DIDDeactivatedException;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.meta.DIDMeta;
import org.elastos.did.util.HDKey;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class DIDStoreTest {
	@Rule
	public ExpectedException expectedEx = ExpectedException.none();

	@Test
	public void testCreateEmptyStore() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);

    	File file = new File(TestConfig.storeRoot);
    	assertTrue(file.exists());
    	assertTrue(file.isDirectory());

    	file = new File(TestConfig.storeRoot + File.separator + ".meta");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	assertFalse(store.containsPrivateIdentity());
	}

	@Test(expected = DIDStoreException.class)
	public void testCreateDidInEmptyStore() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	store.newDid("this will be fail", TestConfig.storePass);
	}

	@Test
	public void testInitPrivateIdentity0() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	assertFalse(store.containsPrivateIdentity());

    	testData.initIdentity();
    	assertTrue(store.containsPrivateIdentity());

    	File file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "key");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "index");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	store = DIDStore.open("filesystem", TestConfig.storeRoot);
    	assertTrue(store.containsPrivateIdentity());
	}

	@Test
	public void testCreateDIDWithAlias() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	String alias = "my first did";

    	DIDDocument doc = store.newDid(alias, TestConfig.storePass);
    	assertTrue(doc.isValid());

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNull(resolved);

    	store.publishDid(doc, TestConfig.storePass);

    	File file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + doc.getSubject().getMethodSpecificId()
    			+ File.separator + "document");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + doc.getSubject().getMethodSpecificId()
    			+ File.separator + ".meta");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	store.storeDid(resolved);
    	assertEquals(alias, resolved.getAlias());
    	assertEquals(doc.getSubject(), resolved.getSubject());
    	assertEquals(doc.getProof().getSignature(),
    			resolved.getProof().getSignature());

    	assertTrue(resolved.isValid());
	}

	@Test
	public void tesCreateDIDWithoutAlias() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNull(resolved);

    	store.publishDid(doc, TestConfig.storePass);

    	File file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + doc.getSubject().getMethodSpecificId()
    			+ File.separator + "document");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + doc.getSubject().getMethodSpecificId()
    			+ File.separator + ".meta");
    	assertFalse(file.exists());

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.getSubject(), resolved.getSubject());
    	assertEquals(doc.getProof().getSignature(),
    			resolved.getProof().getSignature());

    	assertTrue(resolved.isValid());
    }

	@Test
	public void testUpdateDid() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	System.out.println(doc.getTransactionId());

    	store.publishDid(doc, TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	store.storeDid(resolved);
    	assertNotNull(resolved);

    	System.out.println(resolved.getTransactionId());

    	// Update
    	DIDDocument.Builder db = resolved.edit();
    	HDKey.DerivedKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	DIDDocument newDoc = db.seal(TestConfig.storePass);
    	assertEquals(2, newDoc.getPublicKeyCount());
    	assertEquals(2, newDoc.getAuthenticationKeyCount());

       	System.out.println(newDoc.getTransactionId());

    	store.updateDid(newDoc, TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	store.storeDid(resolved);
    	assertNotNull(resolved);
    	assertEquals(newDoc.toString(), resolved.toString());

    	// Update again
    	db = resolved.edit();
    	key = TestData.generateKeypair();
    	db.addAuthenticationKey("key2", key.getPublicKeyBase58());
    	newDoc = db.seal(TestConfig.storePass);
    	assertEquals(3, newDoc.getPublicKeyCount());
    	assertEquals(3, newDoc.getAuthenticationKeyCount());

    	store.updateDid(newDoc, TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(newDoc.toString(), resolved.toString());
	}

	@Test
	public void testUpdateNonExistedDid() throws DIDException {
		expectedEx.expect(DIDStoreException.class);
		expectedEx.expectMessage("Create ID transaction error.");

		TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());
    	// fake a txid
    	DIDMeta meta = new DIDMeta();
    	meta.setStore(store);
    	meta.setTransactionId("12345678");
    	store.storeDidMeta(doc.getSubject(), meta);

    	// Update will fail
    	store.updateDid(doc, TestConfig.storePass);
	}

	// TODO: fix later
	@Ignore("Fix later")
	@Test(expected = DIDDeactivatedException.class)
	public void testDeactivateDidAfterCreate() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc, TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);

    	store.deactivateDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	// Dead code
    	assertNull(resolved);
	}

	// TODO: fix later
	@Ignore("Fix later")
	@Test(expected = DIDDeactivatedException.class)
	public void testDeactivateDidAfterUpdate() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc, TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);

    	// Update
    	DIDDocument.Builder db = resolved.edit();
    	HDKey.DerivedKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	DIDDocument newDoc = db.seal(TestConfig.storePass);
    	assertEquals(2, newDoc.getPublicKeyCount());
    	assertEquals(2, newDoc.getAuthenticationKeyCount());

    	store.updateDid(newDoc, TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(newDoc.toString(), resolved.toString());

    	store.deactivateDid(newDoc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	// dead code
    	assertNull(resolved);
	}

	@Test
	public void testBulkCreate() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

		for (int i = 0; i < 100; i++) {
    		String alias = "my did " + i;
        	DIDDocument doc = store.newDid(alias, TestConfig.storePass);
        	assertTrue(doc.isValid());

        	DIDDocument resolved = doc.getSubject().resolve(true);
        	assertNull(resolved);

        	store.publishDid(doc, TestConfig.storePass);

        	File file = new File(TestConfig.storeRoot + File.separator + "ids"
        			+ File.separator + doc.getSubject().getMethodSpecificId()
        			+ File.separator + "document");
        	assertTrue(file.exists());
        	assertTrue(file.isFile());

        	file = new File(TestConfig.storeRoot + File.separator + "ids"
        			+ File.separator + doc.getSubject().getMethodSpecificId()
        			+ File.separator + ".meta");
        	assertTrue(file.exists());
        	assertTrue(file.isFile());

        	resolved = doc.getSubject().resolve(true);
        	assertNotNull(resolved);
        	store.storeDid(resolved);
        	assertEquals(alias, resolved.getAlias());
        	assertEquals(doc.getSubject(), resolved.getSubject());
        	assertEquals(doc.getProof().getSignature(),
        			resolved.getProof().getSignature());

        	assertTrue(resolved.isValid());
    	}

		List<DID> dids = store.listDids(DIDStore.DID_ALL);
		assertEquals(100, dids.size());

		dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		assertEquals(100, dids.size());

		dids = store.listDids(DIDStore.DID_NO_PRIVATEKEY);
		assertEquals(0, dids.size());
	}

	@Test
	public void testDeleteDID() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	// Create test DIDs
    	LinkedList<DID> dids = new LinkedList<DID>();
		for (int i = 0; i < 100; i++) {
    		String alias = "my did " + i;
        	DIDDocument doc = store.newDid(alias, TestConfig.storePass);
         	store.publishDid(doc, TestConfig.storePass);
         	dids.add(doc.getSubject());
    	}

		for (int i = 0; i < 100; i++) {
			if (i % 5 != 0)
				continue;

			DID did = dids.get(i);

    		boolean deleted = store.deleteDid(did);
    		assertTrue(deleted);

	    	File file = new File(TestConfig.storeRoot + File.separator + "ids"
	    			+ File.separator + did.getMethodSpecificId());
	    	assertFalse(file.exists());

    		deleted = store.deleteDid(did);
    		assertFalse(deleted);
    	}

		List<DID> remains = store.listDids(DIDStore.DID_ALL);
		assertEquals(80, remains.size());

		remains = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		assertEquals(80, remains.size());

		remains = store.listDids(DIDStore.DID_NO_PRIVATEKEY);
		assertEquals(0, remains.size());

	}

	@Test
	public void testStoreAndLoadDID() throws DIDException, IOException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	// Store test data into current store
    	DIDDocument issuer = testData.loadTestIssuer();
    	DIDDocument test = testData.loadTestDocument();

    	DIDDocument doc = store.loadDid(issuer.getSubject());
    	assertEquals(issuer.getSubject(), doc.getSubject());
    	assertEquals(issuer.getProof().getSignature(), doc.getProof().getSignature());
    	assertTrue(doc.isValid());

    	doc = store.loadDid(test.getSubject().toString());
    	assertEquals(test.getSubject(), doc.getSubject());
    	assertEquals(test.getProof().getSignature(), doc.getProof().getSignature());
    	assertTrue(doc.isValid());

		List<DID> dids = store.listDids(DIDStore.DID_ALL);
		assertEquals(2, dids.size());

		dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		assertEquals(2, dids.size());

		dids = store.listDids(DIDStore.DID_NO_PRIVATEKEY);
		assertEquals(0, dids.size());
	}

	@Test
	public void testLoadCredentials() throws DIDException, IOException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	// Store test data into current store
    	testData.loadTestIssuer();
    	DIDDocument test = testData.loadTestDocument();
    	VerifiableCredential vc = testData.loadProfileCredential();
    	vc.setAlias("MyProfile");
    	vc = testData.loadEmailCredential();
    	vc.setAlias("Email");
    	vc = testData.loadTwitterCredential();
    	vc.setAlias("Twitter");
    	vc = testData.loadPassportCredential();
    	vc.setAlias("Passport");

    	DIDURL id = new DIDURL(test.getSubject(), "profile");
    	vc = store.loadCredential(test.getSubject(), id);
    	assertNotNull(vc);
    	assertEquals("MyProfile", vc.getAlias());
    	assertEquals(test.getSubject(), vc.getSubject().getId());
    	assertEquals(id, vc.getId());
    	assertTrue(vc.isValid());

    	// try with full id string
    	vc = store.loadCredential(test.getSubject().toString(), id.toString());
    	assertNotNull(vc);
    	assertEquals("MyProfile", vc.getAlias());
    	assertEquals(test.getSubject(), vc.getSubject().getId());
    	assertEquals(id, vc.getId());
    	assertTrue(vc.isValid());

    	id = new DIDURL(test.getSubject(), "twitter");
    	vc = store.loadCredential(test.getSubject().toString(), "twitter");
    	assertNotNull(vc);
    	assertEquals("Twitter", vc.getAlias());
    	assertEquals(test.getSubject(), vc.getSubject().getId());
    	assertEquals(id, vc.getId());
    	assertTrue(vc.isValid());

    	vc = store.loadCredential(test.getSubject().toString(), "notExist");
    	assertNull(vc);

    	id = new DIDURL(test.getSubject(), "twitter");
		assertTrue(store.containsCredential(test.getSubject(), id));
		assertTrue(store.containsCredential(test.getSubject().toString(), "twitter"));
		assertFalse(store.containsCredential(test.getSubject().toString(), "notExist"));
	}

	@Test
	public void testListCredentials() throws DIDException, IOException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	// Store test data into current store
    	testData.loadTestIssuer();
    	DIDDocument test = testData.loadTestDocument();
    	VerifiableCredential vc = testData.loadProfileCredential();
    	vc.setAlias("MyProfile");
    	vc = testData.loadEmailCredential();
    	vc.setAlias("Email");
    	vc = testData.loadTwitterCredential();
    	vc.setAlias("Twitter");
    	vc = testData.loadPassportCredential();
    	vc.setAlias("Passport");

    	List<DIDURL> vcs = store.listCredentials(test.getSubject());
		assertEquals(4, vcs.size());

		for (DIDURL id : vcs) {
			assertTrue(id.getFragment().equals("profile")
					|| id.getFragment().equals("email")
					|| id.getFragment().equals("twitter")
					|| id.getFragment().equals("passport"));

			assertTrue(id.getAlias().equals("MyProfile")
					|| id.getAlias().equals("Email")
					|| id.getAlias().equals("Twitter")
					|| id.getAlias().equals("Passport"));
		}
	}

	@Test
	public void testDeleteCredential() throws DIDException, IOException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setupStore(true);
    	testData.initIdentity();

    	// Store test data into current store
    	testData.loadTestIssuer();
    	DIDDocument test = testData.loadTestDocument();
    	VerifiableCredential vc = testData.loadProfileCredential();
    	vc.setAlias("MyProfile");
    	vc = testData.loadEmailCredential();
    	vc.setAlias("Email");
    	vc = testData.loadTwitterCredential();
    	vc.setAlias("Twitter");
    	vc = testData.loadPassportCredential();
    	vc.setAlias("Passport");

    	File file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + test.getSubject().getMethodSpecificId()
    			+ File.separator + "credentials" + File.separator + "twitter"
    			+ File.separator + "credential");
    	assertTrue(file.exists());

    	file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + test.getSubject().getMethodSpecificId()
    			+ File.separator + "credentials" + File.separator + "twitter"
    			+ File.separator + ".meta");
    	assertTrue(file.exists());

    	file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + test.getSubject().getMethodSpecificId()
    			+ File.separator + "credentials" + File.separator + "passport"
    			+ File.separator + "credential");
    	assertTrue(file.exists());

    	file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + test.getSubject().getMethodSpecificId()
    			+ File.separator + "credentials" + File.separator + "passport"
    			+ File.separator + ".meta");
    	assertTrue(file.exists());

    	boolean deleted = store.deleteCredential(test.getSubject(),
    			new DIDURL(test.getSubject(), "twitter"));
		assertTrue(deleted);

		deleted = store.deleteCredential(test.getSubject().toString(), "passport");
		assertTrue(deleted);

		deleted = store.deleteCredential(test.getSubject().toString(), "notExist");
		assertFalse(deleted);

    	file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + test.getSubject().getMethodSpecificId()
    			+ File.separator + "credentials" + File.separator + "twitter");
    	assertFalse(file.exists());

    	file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + test.getSubject().getMethodSpecificId()
    			+ File.separator + "credentials" + File.separator + "passport");
    	assertFalse(file.exists());

		assertTrue(store.containsCredential(test.getSubject().toString(), "email"));
		assertTrue(store.containsCredential(test.getSubject().toString(), "profile"));

		assertFalse(store.containsCredential(test.getSubject().toString(), "twitter"));
		assertFalse(store.containsCredential(test.getSubject().toString(), "passport"));
	}

	@Test
	public void testCompatibility() throws DIDException {
		URL url = this.getClass().getResource("/teststore");
		File dir = new File(url.getPath());
		System.out.println(dir.getAbsolutePath());

		DIDBackend.initialize(new DummyAdapter());
		DIDStore store = DIDStore.open("filesystem", dir.getAbsolutePath());

       	List<DID> dids = store.listDids(DIDStore.DID_ALL);
       	assertEquals(2, dids.size());

       	for (DID did : dids) {
       		if (did.getAlias().equals("Issuer")) {
       			List<DIDURL> vcs = store.listCredentials(did);
       			assertEquals(1, vcs.size());

       			DIDURL id = vcs.get(0);
       			assertEquals("Profile", id.getAlias());

       			assertNotNull(store.loadCredential(did, id));
       		} else if (did.getAlias().equals("Test")) {
       			List<DIDURL> vcs = store.listCredentials(did);
       			assertEquals(4, vcs.size());

       			for (DIDURL id : vcs) {
       				assertTrue(id.getAlias().equals("Profile")
       						|| id.getAlias().equals("Email")
       						|| id.getAlias().equals("Passport")
       						|| id.getAlias().equals("Twitter"));

       				assertNotNull(store.loadCredential(did, id));
       			}
       		}
       	}
	}

	@Test
	public void testCompatibilityNewDIDWithWrongPass() throws DIDException {
		expectedEx.expect(DIDStoreException.class);
		expectedEx.expectMessage("Decrypt private key error, maybe wrong store password.");

		URL url = this.getClass().getResource("/teststore");
		File dir = new File(url.getPath());

		DIDBackend.initialize(new DummyAdapter());
		DIDStore store = DIDStore.open("filesystem", dir.getAbsolutePath());

       	DIDDocument doc = store.newDid("wrongpass");
       	// Dead code
       	assertNull(doc);
	}

	@Test
	public void testCompatibilityNewDID() throws DIDException {
		URL url = this.getClass().getResource("/teststore");
		File dir = new File(url.getPath());

		DIDBackend.initialize(new DummyAdapter());
		DIDStore store = DIDStore.open("filesystem", dir.getAbsolutePath());

       	DIDDocument doc = store.newDid(TestConfig.storePass);
       	assertNotNull(doc);

       	store.deleteDid(doc.getSubject());
	}

	private void createDataForPerformanceTest(DIDStore store)
			throws DIDException {
		Map<String, String> props= new HashMap<String, String>();
		props.put("name", "John");
		props.put("gender", "Male");
		props.put("nation", "Singapore");
		props.put("language", "English");
		props.put("email", "john@example.com");
		props.put("twitter", "@john");

		for (int i = 0; i < 10; i++) {
    		String alias = "my did " + i;
        	DIDDocument doc = store.newDid(alias, TestConfig.storePass);

        	Issuer issuer = new Issuer(doc);
        	Issuer.CredentialBuilder cb = issuer.issueFor(doc.getSubject());
        	VerifiableCredential vc = cb.id("cred-1")
        			.type("BasicProfileCredential", "SelfProclaimedCredential")
        			.properties(props)
        			.seal(TestConfig.storePass);

        	store.storeCredential(vc);
		}
	}

	private void testStorePerformance(boolean cached) throws DIDException {
		DIDBackend.initialize(new DummyAdapter());

		TestData.deleteFile(new File(TestConfig.storeRoot));
		DIDStore store = null;
    	if (cached)
    		store = DIDStore.open("filesystem", TestConfig.storeRoot);
    	else
    		store = DIDStore.open("filesystem", TestConfig.storeRoot, 0, 0);

       	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
    	store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
    			TestConfig.passphrase, TestConfig.storePass, true);

    	createDataForPerformanceTest(store);

    	List<DID> dids = store.listDids(DIDStore.DID_ALL);
    	assertEquals(10, dids.size());

    	long start = System.currentTimeMillis();

    	for (int i = 0; i < 1000; i++) {
	    	for (DID did : dids) {
	    		DIDDocument doc = store.loadDid(did);
	    		assertEquals(did, doc.getSubject());

	    		DIDURL id = new DIDURL(did, "cred-1");
	    		VerifiableCredential vc = store.loadCredential(did, id);
	    		assertEquals(id, vc.getId());
	    	}
    	}

    	long end = System.currentTimeMillis();

    	System.out.println("Store " + (cached ? "with " : "without ") +
    			"cache took " + (end - start) + " milliseconds.");
	}

	@Test
	public void testStoreWithCache() throws DIDException {
		testStorePerformance(true);
	}

	@Test
	public void testStoreWithoutCache() throws DIDException {
		testStorePerformance(false);
	}

	@Test
	public void testMultipleStore() throws DIDException {
		DIDStore[] stores = new DIDStore[10];
		DIDDocument[] docs = new DIDDocument[10];

		for (int i = 0; i < stores.length; i++) {
			stores[i] = DIDStore.open("filesystem", TestConfig.storeRoot + i);
			assertNotNull(stores[i]);
			String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
			stores[i].initPrivateIdentity(Mnemonic.ENGLISH, mnemonic, "", TestConfig.storePass);
		}

		for (int i = 0; i < stores.length; i++) {
			docs[i] = stores[i].newDid(TestConfig.storePass);
			assertNotNull(docs[i]);
		}

		for (int i = 0; i < stores.length; i++) {
			DIDDocument doc = stores[i].loadDid(docs[i].getSubject());
			assertNotNull(doc);
			assertEquals(docs[i].toString(true), doc.toString(true));
		}
	}
}
