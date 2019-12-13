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
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.elastos.did.adapter.DummyAdapter;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
import org.junit.Test;

public class DIDStoreTest {
	@Test
	public void testCreateEmptyStore() throws DIDStoreException {
    	TestData testData = new TestData();
    	testData.setupStore(true);

    	DIDStore store = DIDStore.getInstance();

    	File file = new File(TestConfig.storeRoot);
    	assertTrue(file.exists());
    	assertTrue(file.isDirectory());

    	file = new File(TestConfig.storeRoot + File.separator + ".meta");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	assertFalse(store.containsPrivateIdentity());
	}

	@Test(expected = DIDStoreException.class)
	public void testCreateDidInEmptyStore() throws DIDStoreException {
    	TestData testData = new TestData();
    	testData.setupStore(true);

    	DIDStore store = DIDStore.getInstance();
    	store.newDid(TestConfig.storePass, "this will be fail");
	}

	@Test
	public void testInitPrivateIdentity0() throws DIDStoreException {
    	TestData testData = new TestData();
    	testData.setupStore(true);

    	DIDStore store = DIDStore.getInstance();
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

    	DIDStore.initialize("filesystem", TestConfig.storeRoot,
    			new DummyAdapter());

    	store = DIDStore.getInstance();
    	assertTrue(store.containsPrivateIdentity());
	}

	@Test
	public void testCreateDIDWithAlias() throws DIDException {
    	TestData testData = new TestData();
    	testData.setupStore(true);
    	testData.initIdentity();

    	DIDStore store = DIDStore.getInstance();
		String alias = "my first did";

    	DIDDocument doc = store.newDid(TestConfig.storePass, alias);
    	assertTrue(doc.isValid());

    	DIDDocument resolved = store.resolveDid(doc.getSubject(), true);
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

    	resolved = store.resolveDid(doc.getSubject(), true);
    	assertNotNull(resolved);
    	assertEquals(alias, resolved.getAlias());
    	assertEquals(doc.getSubject(), resolved.getSubject());
    	assertEquals(doc.getProof().getSignature(),
    			resolved.getProof().getSignature());

    	assertTrue(resolved.isValid());
	}

	@Test
	public void tesCreateDIDWithoutAlias() throws DIDException {
    	TestData testData = new TestData();
    	testData.setupStore(true);
    	testData.initIdentity();

    	DIDStore store = DIDStore.getInstance();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	DIDDocument resolved = store.resolveDid(doc.getSubject(), true);
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

    	resolved = store.resolveDid(doc.getSubject(), true);
    	assertNotNull(resolved);
    	assertEquals(doc.getSubject(), resolved.getSubject());
    	assertEquals(doc.getProof().getSignature(),
    			resolved.getProof().getSignature());

    	assertTrue(resolved.isValid());
    }

	@Test
	public void testBulkCreate() throws DIDException {
    	TestData testData = new TestData();
    	testData.setupStore(true);
    	testData.initIdentity();

    	DIDStore store = DIDStore.getInstance();

		for (int i = 0; i < 100; i++) {
    		String alias = "my did " + i;
        	DIDDocument doc = store.newDid(TestConfig.storePass, alias);
        	assertTrue(doc.isValid());

        	DIDDocument resolved = store.resolveDid(doc.getSubject(), true);
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

        	resolved = store.resolveDid(doc.getSubject(), true);
        	assertNotNull(resolved);
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
    	testData.setupStore(true);
    	testData.initIdentity();

    	DIDStore store = DIDStore.getInstance();

    	// Create test DIDs
    	LinkedList<DID> dids = new LinkedList<DID>();
		for (int i = 0; i < 100; i++) {
    		String alias = "my did " + i;
        	DIDDocument doc = store.newDid(TestConfig.storePass, alias);
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
    	testData.setupStore(true);
    	testData.initIdentity();

    	// Store test data into current store
    	DIDDocument issuer = testData.loadTestDocument();
    	DIDDocument test = testData.loadTestIssuer();

    	DIDStore store = DIDStore.getInstance();

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
    	testData.setupStore(true);
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

    	DIDStore store = DIDStore.getInstance();

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
    	testData.setupStore(true);
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

    	DIDStore store = DIDStore.getInstance();

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
    	testData.setupStore(true);
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

    	DIDStore store = DIDStore.getInstance();

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

	private void createDataForPerformanceTest() throws DIDException {
		DIDStore store = DIDStore.getInstance();

		Map<String, String> props= new HashMap<String, String>();
		props.put("name", "John");
		props.put("gender", "Male");
		props.put("nation", "Singapore");
		props.put("language", "English");
		props.put("email", "john@example.com");
		props.put("twitter", "@john");

		for (int i = 0; i < 10; i++) {
    		String alias = "my did " + i;
        	DIDDocument doc = store.newDid(TestConfig.storePass, alias);

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
		DIDAdapter adapter = new DummyAdapter();
    	TestData.deleteFile(new File(TestConfig.storeRoot));
    	if (cached)
    		DIDStore.initialize("filesystem", TestConfig.storeRoot, adapter);
    	else
    		DIDStore.initialize("filesystem", TestConfig.storeRoot, adapter, 0, 0);

       	DIDStore store = DIDStore.getInstance();

       	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
    	store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
    			TestConfig.passphrase, TestConfig.storePass, true);

    	createDataForPerformanceTest();

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
}
