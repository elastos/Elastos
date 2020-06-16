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
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.elastos.did.adapter.DummyAdapter;
import org.elastos.did.crypto.HDKey;
import org.elastos.did.exception.DIDDeactivatedException;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.WrongPasswordException;
import org.junit.jupiter.api.Test;

public class DIDStoreTest {
	@Test
	public void testCreateEmptyStore() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);

    	File file = new File(TestConfig.storeRoot);
    	assertTrue(file.exists());
    	assertTrue(file.isDirectory());

    	file = new File(TestConfig.storeRoot + File.separator + ".meta");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	assertFalse(store.containsPrivateIdentity());
	}

	@Test
	public void testCreateDidInEmptyStore() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);

    	assertThrows(DIDStoreException.class, () -> {
    		store.newDid(TestConfig.storePass);
    	});
	}

	@Test
	public void testInitPrivateIdentity() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	assertFalse(store.containsPrivateIdentity());

    	String mnemonic = testData.initIdentity();
    	assertTrue(store.containsPrivateIdentity());

    	File file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "key");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "index");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "mnemonic");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	store = DIDStore.open("filesystem", TestConfig.storeRoot, testData.getAdapter());
    	assertTrue(store.containsPrivateIdentity());

    	String exportedMnemonic = store.exportMnemonic(TestConfig.storePass);
    	assertEquals(mnemonic, exportedMnemonic);
	}

	@Test
	public void testInitPrivateIdentityWithMnemonic() throws DIDException {
		String expectedIDString = "iY4Ghz9tCuWvB5rNwvn4ngWvthZMNzEA7U";
		String mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";

		TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	assertFalse(store.containsPrivateIdentity());

    	store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic, "", TestConfig.storePass);
    	assertTrue(store.containsPrivateIdentity());

    	File file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "key");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "index");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "mnemonic");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	store = DIDStore.open("filesystem", TestConfig.storeRoot, testData.getAdapter());
    	assertTrue(store.containsPrivateIdentity());

    	String exportedMnemonic = store.exportMnemonic(TestConfig.storePass);
    	assertEquals(mnemonic, exportedMnemonic);

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertNotNull(doc);
    	assertEquals(expectedIDString, doc.getSubject().getMethodSpecificId());
	}

	@Test
	public void testInitPrivateIdentityWithRootKey() throws DIDException {
		String expectedIDString = "iYbPqEA98rwvDyA5YT6a3mu8UZy87DLEMR";
		String rootKey = "xprv9s21ZrQH143K4biiQbUq8369meTb1R8KnstYFAKtfwk3vF8uvFd1EC2s49bMQsbdbmdJxUWRkuC48CXPutFfynYFVGnoeq8LJZhfd9QjvUt";

		TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	assertFalse(store.containsPrivateIdentity());

    	store.initPrivateIdentity(rootKey, TestConfig.storePass);
    	assertTrue(store.containsPrivateIdentity());

    	File file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "key");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "index");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(TestConfig.storeRoot + File.separator + "private"
    			+ File.separator + "mnemonic");
    	assertFalse(file.exists());

    	store = DIDStore.open("filesystem", TestConfig.storeRoot, testData.getAdapter());
    	assertTrue(store.containsPrivateIdentity());

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertNotNull(doc);
    	assertEquals(expectedIDString, doc.getSubject().getMethodSpecificId());
	}

	@Test
	public void testCreateDIDWithAlias() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	String alias = "my first did";

    	DIDDocument doc = store.newDid(alias, TestConfig.storePass);
    	assertTrue(doc.isValid());

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNull(resolved);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

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

    	// test alias
    	store.storeDid(resolved);
    	assertEquals(alias, resolved.getMetadata().getAlias());
    	assertEquals(doc.getSubject(), resolved.getSubject());
    	assertEquals(doc.getProof().getSignature(),
    			resolved.getProof().getSignature());

    	assertTrue(resolved.isValid());
	}

	@Test
	public void tesCreateDIDWithoutAlias() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNull(resolved);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	File file = new File(TestConfig.storeRoot + File.separator + "ids"
    			+ File.separator + doc.getSubject().getMethodSpecificId()
    			+ File.separator + "document");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.getSubject(), resolved.getSubject());
    	assertEquals(doc.getProof().getSignature(),
    			resolved.getProof().getSignature());

    	assertTrue(resolved.isValid());
    }

	@Test
	public void testCreateDIDByIndex() throws DIDException {
	    TestData testData = new TestData();
	    DIDStore store = testData.setup(true);
	    testData.initIdentity();

	    String alias = "my first did";

	    DID did = store.getDid(0);
	    DIDDocument doc = store.newDid(0, alias, TestConfig.storePass);
	    assertTrue(doc.isValid());
	    assertEquals(did, doc.getSubject());

	    Exception e = assertThrows(DIDStoreException.class, () -> {
	        store.newDid(alias, TestConfig.storePass);
	    });
	    assertEquals("DID already exists.", e.getMessage());

	    boolean success = store.deleteDid(did);
	    assertTrue(success);
	    doc = store.newDid(alias, TestConfig.storePass);
	    assertTrue(doc.isValid());
	    assertEquals(did, doc.getSubject());
	}

	@Test
	public void testGetDid() throws DIDException {
	    TestData testData = new TestData();
	    DIDStore store = testData.setup(true);
	    testData.initIdentity();

	    for (int i = 0; i < 100; i++) {
		    String alias = "did#" + i;

		    DIDDocument doc = store.newDid(i, alias, TestConfig.storePass);
		    assertTrue(doc.isValid());

		    DID did = store.getDid(i);

		    assertEquals(doc.getSubject(), did);
	    }
	}

	@Test
	public void testUpdateDid() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	// Update
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(2, doc.getPublicKeyCount());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	// Update again
    	db = doc.edit();
    	key = TestData.generateKeypair();
    	db.addAuthenticationKey("key2", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(3, doc.getPublicKeyCount());
    	assertEquals(3, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());
	}

	@Test
	public void testUpdateDidWithoutTxid() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	// Update
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(2, doc.getPublicKeyCount());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	doc.getMetadataImpl().setPreviousTransactionId(null);
    	doc.saveMetadata();

    	// Update again
    	db = doc.edit();
    	key = TestData.generateKeypair();
    	db.addAuthenticationKey("key2", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(3, doc.getPublicKeyCount());
    	assertEquals(3, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());
	}


	@Test
	public void testUpdateDidWithoutSignature() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	// Update
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(2, doc.getPublicKeyCount());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	doc.getMetadataImpl().setSignature(null);
    	doc.saveMetadata();

    	// Update again
    	db = doc.edit();
    	key = TestData.generateKeypair();
    	db.addAuthenticationKey("key2", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(3, doc.getPublicKeyCount());
    	assertEquals(3, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());
	}

	@Test
	public void testUpdateDidWithoutTxidAndSignature() throws DIDException {
		TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	doc.getMetadataImpl().setPreviousTransactionId(null);
    	doc.getMetadataImpl().setSignature(null);
    	doc.saveMetadata();

    	// Update
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(2, doc.getPublicKeyCount());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	DID did = doc.getSubject();
    	Exception e = assertThrows(DIDStoreException.class, () -> {
    		store.publishDid(did, TestConfig.storePass);
    	});
    	assertEquals("DID document not up-to-date", e.getMessage());
	}

	@Test
	public void testForceUpdateDidWithoutTxidAndSignature() throws DIDException {
		TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	doc.getMetadataImpl().setPreviousTransactionId(null);
    	doc.getMetadataImpl().setSignature(null);
    	doc.saveMetadata();

    	// Update
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(2, doc.getPublicKeyCount());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), doc.getDefaultPublicKey(),
    			true, TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());
	}

	@Test
	public void testUpdateDidWithWrongTxid() throws DIDException {
		TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	// Update
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(2, doc.getPublicKeyCount());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

		store.publishDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	doc.getMetadataImpl().setPreviousTransactionId("1234567890");
    	doc.saveMetadata();

    	// Update
    	db = doc.edit();
    	key = TestData.generateKeypair();
    	db.addAuthenticationKey("key2", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(3, doc.getPublicKeyCount());
    	assertEquals(3, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

		store.publishDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());
	}

	@Test
	public void testUpdateDidWithWrongSignature() throws DIDException {
		TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	// Update
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(2, doc.getPublicKeyCount());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

   		store.publishDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	doc.getMetadataImpl().setSignature("1234567890");
    	doc.saveMetadata();

    	// Update
    	db = doc.edit();
    	key = TestData.generateKeypair();
    	db.addAuthenticationKey("key2", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(3, doc.getPublicKeyCount());
    	assertEquals(3, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	DID did = doc.getSubject();
    	Exception e = assertThrows(DIDStoreException.class, () -> {
    		store.publishDid(did, TestConfig.storePass);
    	});
    	assertEquals("DID document not up-to-date", e.getMessage());
	}

	@Test
	public void testForceUpdateDidWithWrongTxid() throws DIDException {
		TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	doc.getMetadataImpl().setPreviousTransactionId("1234567890");
    	doc.saveMetadata();

    	// Update
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(2, doc.getPublicKeyCount());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), doc.getDefaultPublicKey(),
    			true, TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());
	}

	@Test
	public void testForceUpdateDidWithWrongSignature() throws DIDException {
		TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	doc.getMetadataImpl().setSignature("1234567890");
    	doc.saveMetadata();

    	// Update
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(2, doc.getPublicKeyCount());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), doc.getDefaultPublicKey(),
    			true, TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());
	}

	@Test
	public void testDeactivateSelfAfterCreate() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	store.deactivateDid(doc.getSubject(), TestConfig.storePass);

    	assertThrows(DIDDeactivatedException.class, () -> {
    		doc.getSubject().resolve(true);
    	});
	}

	@Test
	public void testDeactivateSelfAfterUpdate() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	// Update
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	doc = db.seal(TestConfig.storePass);
    	assertEquals(2, doc.getPublicKeyCount());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	store.deactivateDid(doc.getSubject(), TestConfig.storePass);

    	DID did = doc.getSubject();
    	assertThrows(DIDDeactivatedException.class, () -> {
    		did.resolve(true);
    	});
	}

	@Test
	public void testDeactivateWithAuthorization1() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	assertTrue(doc.isValid());

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	DIDDocument target = store.newDid(TestConfig.storePass);
    	DIDDocument.Builder db = target.edit();
    	db.authorizationDid("recovery", doc.getSubject().toString());
    	target = db.seal(TestConfig.storePass);
    	assertNotNull(target);
    	assertEquals(1, target.getAuthorizationKeyCount());
    	assertEquals(doc.getSubject(), target.getAuthorizationKeys().get(0).getController());
    	store.storeDid(target);

    	store.publishDid(target.getSubject(), TestConfig.storePass);

    	resolved = target.getSubject().resolve();
    	assertNotNull(resolved);
    	assertEquals(target.toString(), resolved.toString());

    	store.deactivateDid(target.getSubject(), doc.getSubject(), TestConfig.storePass);

    	DID did = target.getSubject();
    	assertThrows(DIDDeactivatedException.class, () -> {
    		did.resolve(true);
    	});
	}

	@Test
	public void testDeactivateWithAuthorization2() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	DIDURL id = new DIDURL(doc.getSubject(), "key-2");
    	db.addAuthenticationKey(id, key.getPublicKeyBase58());
    	store.storePrivateKey(doc.getSubject(), id, key.getPrivateKeyBytes(),
    			TestConfig.storePass);
    	doc = db.seal(TestConfig.storePass);
    	assertTrue(doc.isValid());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	DIDDocument target = store.newDid(TestConfig.storePass);
    	db = target.edit();
    	db.addAuthorizationKey("recovery", doc.getSubject().toString(),
    			key.getPublicKeyBase58());
    	target = db.seal(TestConfig.storePass);
    	assertNotNull(target);
    	assertEquals(1, target.getAuthorizationKeyCount());
    	assertEquals(doc.getSubject(), target.getAuthorizationKeys().get(0).getController());
    	store.storeDid(target);

    	store.publishDid(target.getSubject(), TestConfig.storePass);

    	resolved = target.getSubject().resolve();
    	assertNotNull(resolved);
    	assertEquals(target.toString(), resolved.toString());

    	store.deactivateDid(target.getSubject(), doc.getSubject(), id, TestConfig.storePass);

    	DID did = target.getSubject();
    	assertThrows(DIDDeactivatedException.class, () -> {
    		did.resolve(true);
    	});
	}

	@Test
	public void testDeactivateWithAuthorization3() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	DIDDocument.Builder db = doc.edit();
    	HDKey key = TestData.generateKeypair();
    	DIDURL id = new DIDURL(doc.getSubject(), "key-2");
    	db.addAuthenticationKey(id, key.getPublicKeyBase58());
    	store.storePrivateKey(doc.getSubject(), id, key.getPrivateKeyBytes(),
    			TestConfig.storePass);
    	doc = db.seal(TestConfig.storePass);
    	assertTrue(doc.isValid());
    	assertEquals(2, doc.getAuthenticationKeyCount());
    	store.storeDid(doc);

    	store.publishDid(doc.getSubject(), TestConfig.storePass);

    	DIDDocument resolved = doc.getSubject().resolve(true);
    	assertNotNull(resolved);
    	assertEquals(doc.toString(), resolved.toString());

    	DIDDocument target = store.newDid(TestConfig.storePass);
    	db = target.edit();
    	db.addAuthorizationKey("recovery", doc.getSubject().toString(),
    			key.getPublicKeyBase58());
    	target = db.seal(TestConfig.storePass);
    	assertNotNull(target);
    	assertEquals(1, target.getAuthorizationKeyCount());
    	assertEquals(doc.getSubject(), target.getAuthorizationKeys().get(0).getController());
    	store.storeDid(target);

    	store.publishDid(target.getSubject(), TestConfig.storePass);

    	resolved = target.getSubject().resolve();
    	assertNotNull(resolved);
    	assertEquals(target.toString(), resolved.toString());

    	store.deactivateDid(target.getSubject(), doc.getSubject(), TestConfig.storePass);

    	DID did = target.getSubject();
    	assertThrows(DIDDeactivatedException.class, () -> {
    		did.resolve(true);
    	});
	}

	@Test
	public void testBulkCreate() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

		for (int i = 0; i < 100; i++) {
    		String alias = "my did " + i;
        	DIDDocument doc = store.newDid(alias, TestConfig.storePass);
        	assertTrue(doc.isValid());

        	DIDDocument resolved = doc.getSubject().resolve(true);
        	assertNull(resolved);

        	store.publishDid(doc.getSubject(), TestConfig.storePass);

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
        	assertEquals(alias, resolved.getMetadata().getAlias());
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
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	// Create test DIDs
    	LinkedList<DID> dids = new LinkedList<DID>();
		for (int i = 0; i < 100; i++) {
    		String alias = "my did " + i;
        	DIDDocument doc = store.newDid(alias, TestConfig.storePass);
         	store.publishDid(doc.getSubject(), TestConfig.storePass);
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
    	DIDStore store = testData.setup(true);
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
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	// Store test data into current store
    	testData.loadTestIssuer();
    	DIDDocument test = testData.loadTestDocument();
    	VerifiableCredential vc = testData.loadProfileCredential();
    	vc.getMetadata().setAlias("MyProfile");
    	vc = testData.loadEmailCredential();
    	vc.getMetadata().setAlias("Email");
    	vc = testData.loadTwitterCredential();
    	vc.getMetadata().setAlias("Twitter");
    	vc = testData.loadPassportCredential();
    	vc.getMetadata().setAlias("Passport");

    	DIDURL id = new DIDURL(test.getSubject(), "profile");
    	vc = store.loadCredential(test.getSubject(), id);
    	assertNotNull(vc);
    	assertEquals("MyProfile", vc.getMetadata().getAlias());
    	assertEquals(test.getSubject(), vc.getSubject().getId());
    	assertEquals(id, vc.getId());
    	assertTrue(vc.isValid());

    	// try with full id string
    	vc = store.loadCredential(test.getSubject().toString(), id.toString());
    	assertNotNull(vc);
    	assertEquals("MyProfile", vc.getMetadata().getAlias());
    	assertEquals(test.getSubject(), vc.getSubject().getId());
    	assertEquals(id, vc.getId());
    	assertTrue(vc.isValid());

    	id = new DIDURL(test.getSubject(), "twitter");
    	vc = store.loadCredential(test.getSubject().toString(), "twitter");
    	assertNotNull(vc);
    	assertEquals("Twitter", vc.getMetadata().getAlias());
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
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	// Store test data into current store
    	testData.loadTestIssuer();
    	DIDDocument test = testData.loadTestDocument();
    	VerifiableCredential vc = testData.loadProfileCredential();
    	vc.getMetadata().setAlias("MyProfile");
    	vc = testData.loadEmailCredential();
    	vc.getMetadata().setAlias("Email");
    	vc = testData.loadTwitterCredential();
    	vc.getMetadata().setAlias("Twitter");
    	vc = testData.loadPassportCredential();
    	vc.getMetadata().setAlias("Passport");

    	List<DIDURL> vcs = store.listCredentials(test.getSubject());
		assertEquals(4, vcs.size());

		for (DIDURL id : vcs) {
			assertTrue(id.getFragment().equals("profile")
					|| id.getFragment().equals("email")
					|| id.getFragment().equals("twitter")
					|| id.getFragment().equals("passport"));

			assertTrue(id.getMetadata().getAlias().equals("MyProfile")
					|| id.getMetadata().getAlias().equals("Email")
					|| id.getMetadata().getAlias().equals("Twitter")
					|| id.getMetadata().getAlias().equals("Passport"));
		}
	}

	@Test
	public void testDeleteCredential() throws DIDException, IOException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	// Store test data into current store
    	testData.loadTestIssuer();
    	DIDDocument test = testData.loadTestDocument();
    	VerifiableCredential vc = testData.loadProfileCredential();
    	vc.getMetadata().setAlias("MyProfile");
    	vc.saveMetadata();
    	vc = testData.loadEmailCredential();
    	vc.getMetadata().setAlias("Email");
    	vc.saveMetadata();
    	vc = testData.loadTwitterCredential();
    	vc.getMetadata().setAlias("Twitter");
    	vc.saveMetadata();
    	vc = testData.loadPassportCredential();
    	vc.getMetadata().setAlias("Passport");
    	vc.saveMetadata();

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
	public void testChangePassword() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

		for (int i = 0; i < 10; i++) {
    		String alias = "my did " + i;
        	DIDDocument doc = store.newDid(alias, TestConfig.storePass);
        	assertTrue(doc.isValid());

        	DIDDocument resolved = doc.getSubject().resolve(true);
        	assertNull(resolved);

        	store.publishDid(doc.getSubject(), TestConfig.storePass);

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
        	assertEquals(alias, resolved.getMetadata().getAlias());
        	assertEquals(doc.getSubject(), resolved.getSubject());
        	assertEquals(doc.getProof().getSignature(),
        			resolved.getProof().getSignature());

        	assertTrue(resolved.isValid());
    	}

		List<DID> dids = store.listDids(DIDStore.DID_ALL);
		assertEquals(10, dids.size());

		dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		assertEquals(10, dids.size());

		dids = store.listDids(DIDStore.DID_NO_PRIVATEKEY);
		assertEquals(0, dids.size());

		store.changePassword(TestConfig.storePass, "newpasswd");

		dids = store.listDids(DIDStore.DID_ALL);
		assertEquals(10, dids.size());

		dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		assertEquals(10, dids.size());

		dids = store.listDids(DIDStore.DID_NO_PRIVATEKEY);
		assertEquals(0, dids.size());

		DIDDocument doc = store.newDid("newpasswd");
		assertNotNull(doc);
	}

	@Test
	public void testChangePasswordWithWrongPassword() throws DIDException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

		for (int i = 0; i < 10; i++) {
    		String alias = "my did " + i;
        	DIDDocument doc = store.newDid(alias, TestConfig.storePass);
        	assertTrue(doc.isValid());

        	DIDDocument resolved = doc.getSubject().resolve(true);
        	assertNull(resolved);

        	store.publishDid(doc.getSubject(), TestConfig.storePass);

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
        	assertEquals(alias, resolved.getMetadata().getAlias());
        	assertEquals(doc.getSubject(), resolved.getSubject());
        	assertEquals(doc.getProof().getSignature(),
        			resolved.getProof().getSignature());

        	assertTrue(resolved.isValid());
    	}

		List<DID> dids = store.listDids(DIDStore.DID_ALL);
		assertEquals(10, dids.size());

		dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		assertEquals(10, dids.size());

		dids = store.listDids(DIDStore.DID_NO_PRIVATEKEY);
		assertEquals(0, dids.size());

		assertThrows(DIDStoreException.class, () -> {
			store.changePassword("wrongpasswd", "newpasswd");
		});
	}

	@Test
	public void testCompatibility() throws DIDException {
		byte[] data = "Hello World".getBytes();

		URL url = this.getClass().getResource("/teststore");
		File dir = new File(url.getPath());
		System.out.println(url.getPath());

		DummyAdapter adapter = new DummyAdapter();
		DIDBackend.initialize(adapter, TestData.getResolverCacheDir());
		DIDStore store = DIDStore.open("filesystem", dir.getAbsolutePath(), adapter);

       	List<DID> dids = store.listDids(DIDStore.DID_ALL);
       	assertEquals(2, dids.size());

       	for (DID did : dids) {
       		if (did.getMetadata().getAlias().equals("Issuer")) {
       			List<DIDURL> vcs = store.listCredentials(did);
       			assertEquals(1, vcs.size());

       			DIDURL id = vcs.get(0);
       			assertEquals("Profile", id.getMetadata().getAlias());

       			assertNotNull(store.loadCredential(did, id));
       		} else if (did.getMetadata().getAlias().equals("Test")) {
       			List<DIDURL> vcs = store.listCredentials(did);
       			assertEquals(4, vcs.size());

       			for (DIDURL id : vcs) {
       				assertTrue(id.getMetadata().getAlias().equals("Profile")
       						|| id.getMetadata().getAlias().equals("Email")
       						|| id.getMetadata().getAlias().equals("Passport")
       						|| id.getMetadata().getAlias().equals("Twitter"));

       				assertNotNull(store.loadCredential(did, id));
       			}
       		}

       		DIDDocument doc = store.loadDid(did);
       		String sig = doc.sign(TestConfig.storePass, data);
       		assertTrue(doc.verify(sig, data));
       	}
	}

	@Test
	public void testCompatibilityNewDIDWithWrongPass() throws DIDException {
		URL url = this.getClass().getResource("/teststore");
		File dir = new File(url.getPath());

		DummyAdapter adapter = new DummyAdapter();
		DIDBackend.initialize(adapter, TestData.getResolverCacheDir());
		DIDStore store = DIDStore.open("filesystem", dir.getAbsolutePath(), adapter);

		assertThrows(WrongPasswordException.class, () -> {
			store.newDid("wrongpass");
		});
	}

	@Test
	public void testCompatibilityNewDIDandGetDID() throws DIDException {
		URL url = this.getClass().getResource("/teststore");
		File dir = new File(url.getPath());

		DummyAdapter adapter = new DummyAdapter();
		DIDBackend.initialize(adapter, TestData.getResolverCacheDir());
		DIDStore store = DIDStore.open("filesystem", dir.getAbsolutePath(), adapter);

       	DIDDocument doc = store.newDid(TestConfig.storePass);
       	assertNotNull(doc);

       	store.deleteDid(doc.getSubject());

       	DID did = store.getDid(1000);

       	doc = store.newDid(1000, TestConfig.storePass);
       	assertNotNull(doc);
       	assertEquals(doc.getSubject(), did);

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
		DummyAdapter adapter = new DummyAdapter();
		DIDBackend.initialize(adapter, TestData.getResolverCacheDir());

		Utils.deleteFile(new File(TestConfig.storeRoot));
		DIDStore store = null;
    	if (cached)
    		store = DIDStore.open("filesystem", TestConfig.storeRoot, adapter);
    	else
    		store = DIDStore.open("filesystem", TestConfig.storeRoot, 0, 0, adapter);

       	String mnemonic =  Mnemonic.getInstance().generate();
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
			Utils.deleteFile(new File(TestConfig.storeRoot + i));
			stores[i] = DIDStore.open("filesystem", TestConfig.storeRoot + i, new DummyAdapter());
			assertNotNull(stores[i]);
			String mnemonic = Mnemonic.getInstance().generate();
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

	@Test
	public void testExportAndImportDid() throws DIDException, IOException {
		URL url = this.getClass().getResource("/teststore");
		File storeDir = new File(url.getPath());

		DummyAdapter adapter = new DummyAdapter();
		DIDBackend.initialize(adapter, TestData.getResolverCacheDir());
		DIDStore store = DIDStore.open("filesystem", storeDir.getAbsolutePath(), adapter);

		DID did = store.listDids(DIDStore.DID_ALL).get(0);

		File tempDir = new File(TestConfig.tempDir);
		tempDir.mkdirs();
		File exportFile = new File(tempDir, "didexport.json");

		store.exportDid(did, exportFile, "password", TestConfig.storePass);

		File restoreDir = new File(tempDir, "restore");
		Utils.deleteFile(restoreDir);
		DIDStore store2 = DIDStore.open("filesystem", restoreDir.getAbsolutePath(), adapter);
		store2.importDid(exportFile, "password", TestConfig.storePass);

		String path = "ids" + File.separator + did.getMethodSpecificId();
		File didDir = new File(storeDir, path);
		File reDidDir = new File(restoreDir, path);
		assertTrue(didDir.exists());
		assertTrue(reDidDir.exists());
		assertTrue(Utils.equals(reDidDir, didDir));
	}

	@Test
	public void testExportAndImportPrivateIdentity() throws DIDException, IOException {
		URL url = this.getClass().getResource("/teststore");
		File storeDir = new File(url.getPath());

		DummyAdapter adapter = new DummyAdapter();
		DIDBackend.initialize(adapter, TestData.getResolverCacheDir());
		DIDStore store = DIDStore.open("filesystem", storeDir.getAbsolutePath(), adapter);

		File tempDir = new File(TestConfig.tempDir);
		tempDir.mkdirs();
		File exportFile = new File(tempDir, "idexport.json");

		store.exportPrivateIdentity(exportFile, "password", TestConfig.storePass);

		File restoreDir = new File(tempDir, "restore");
		Utils.deleteFile(restoreDir);
		DIDStore store2 = DIDStore.open("filesystem", restoreDir.getAbsolutePath(), adapter);
		store2.importPrivateIdentity(exportFile, "password", TestConfig.storePass);

		File privateDir = new File(storeDir, "private");
		File rePrivateDir = new File(restoreDir, "private");
		assertTrue(privateDir.exists());
		assertTrue(rePrivateDir.exists());
		assertTrue(Utils.equals(rePrivateDir, privateDir));
	}

	@Test
	public void testExportAndImportStore() throws DIDException, IOException {
    	TestData testData = new TestData();
    	DIDStore store = testData.setup(true);
    	testData.initIdentity();

    	// Store test data into current store
    	testData.loadTestIssuer();
    	testData.loadTestDocument();
    	VerifiableCredential vc = testData.loadProfileCredential();
    	vc.getMetadata().setAlias("MyProfile");
    	vc = testData.loadEmailCredential();
    	vc.getMetadata().setAlias("Email");
    	vc = testData.loadTwitterCredential();
    	vc.getMetadata().setAlias("Twitter");
    	vc = testData.loadPassportCredential();
    	vc.getMetadata().setAlias("Passport");

		File tempDir = new File(TestConfig.tempDir);
		tempDir.mkdirs();
		File exportFile = new File(tempDir, "storeexport.zip");

		store.exportStore(exportFile, "password", TestConfig.storePass);

		File restoreDir = new File(tempDir, "restore");
		Utils.deleteFile(restoreDir);
		DIDStore store2 = DIDStore.open("filesystem", restoreDir.getAbsolutePath(), testData.getAdapter());
		store2.importStore(exportFile, "password", TestConfig.storePass);

		File storeDir = new File(TestConfig.storeRoot);

		assertTrue(storeDir.exists());
		assertTrue(restoreDir.exists());
		assertTrue(Utils.equals(restoreDir, storeDir));
	}
}
