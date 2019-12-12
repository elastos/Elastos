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
import java.util.LinkedList;
import java.util.List;

import org.elastos.credential.VerifiableCredential;
import org.elastos.did.backend.DummyAdapter;
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

    	file = new File(TestConfig.storeRoot + File.separator + ".DIDStore");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	assertFalse(store.hasPrivateIdentity());
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
    	assertFalse(store.hasPrivateIdentity());

    	testData.initIdentity();
    	assertTrue(store.hasPrivateIdentity());

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
    	assertTrue(store.hasPrivateIdentity());
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
    			+ File.separator + "."
    			+ doc.getSubject().getMethodSpecificId() + ".meta");
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
    			+ File.separator + "."
    			+ doc.getSubject().getMethodSpecificId() + ".meta");
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
        			+ File.separator + "."
        			+ doc.getSubject().getMethodSpecificId() + ".meta");
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

	    	file = new File(TestConfig.storeRoot + File.separator + "ids"
	    			+ File.separator + "."
	    			+ did.getMethodSpecificId() + ".meta");
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
    	testData.loadProfileCredential();
    	testData.loadEmailCredential();
    	testData.loadTwitterCredential();
    	testData.loadPassportCredential();

    	DIDStore store = DIDStore.getInstance();

    	DIDURL id = new DIDURL(test.getSubject(), "profile");
    	VerifiableCredential vc = store.loadCredential(test.getSubject(), id);
    	assertNotNull(vc);
    	vc.setAlias("MyProfile");
    	assertEquals("MyProfile", vc.getAlias());
    	assertEquals(test.getSubject(), vc.getSubject().getId());
    	assertEquals(id, vc.getId());
    	assertTrue(vc.isValid());

    	id = new DIDURL(test.getSubject(), "twitter");
    	vc.setAlias("Twitter");
    	assertEquals("Twitter", vc.getAlias());
    	vc = store.loadCredential(test.getSubject().toString(), "twitter");
    	assertNotNull(vc);
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
    	testData.loadProfileCredential();
    	testData.loadEmailCredential();
    	testData.loadTwitterCredential();
    	testData.loadPassportCredential();

    	DIDStore store = DIDStore.getInstance();

    	boolean deleted = store.deleteCredential(test.getSubject(),
    			new DIDURL(test.getSubject(), "twitter"));
		assertTrue(deleted);

		deleted = store.deleteCredential(test.getSubject().toString(), "passport");
		assertTrue(deleted);

		deleted = store.deleteCredential(test.getSubject().toString(), "notExist");
		assertFalse(deleted);

    	File file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + test.getSubject().getMethodSpecificId()
    			+ File.separator + "credentials" + File.separator + "twitter");
    	assertFalse(file.exists());

    	file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + test.getSubject().getMethodSpecificId()
    			+ File.separator + "credentials" + File.separator + ".twitter.meta");
    	assertFalse(file.exists());

    	file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + test.getSubject().getMethodSpecificId()
    			+ File.separator + "credentials" + File.separator + "passport");
    	assertFalse(file.exists());

    	file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + test.getSubject().getMethodSpecificId()
    			+ File.separator + "credentials" + File.separator + ".passport.meta");
    	assertFalse(file.exists());

		assertTrue(store.containsCredential(test.getSubject().toString(), "email"));
		assertTrue(store.containsCredential(test.getSubject().toString(), "profile"));

		assertFalse(store.containsCredential(test.getSubject().toString(), "twitter"));
		assertFalse(store.containsCredential(test.getSubject().toString(), "passport"));
	}
}
