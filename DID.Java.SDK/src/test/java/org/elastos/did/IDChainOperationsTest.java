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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import org.elastos.did.adapter.SPVAdapter;
import org.elastos.did.exception.DIDException;
import org.elastos.did.util.HDKey;
import org.junit.Test;

public class IDChainOperationsTest {
	@Test
	public void testPublishAndResolve() throws DIDException {
    	TestData testData = new TestData();
    	testData.setupStore(false);
    	testData.initIdentity();

    	DIDStore store = DIDStore.getInstance();
    	assertTrue(store.getAdapter() instanceof SPVAdapter);
    	SPVAdapter adapter = (SPVAdapter)store.getAdapter();

		System.out.print("Waiting for wallet available to create DID");
    	while (true) {
    		if (adapter.isAvailable()) {
    			System.out.println(" OK");
    			break;
    		} else {
    			System.out.print(".");
    		}

    		try {
				Thread.sleep(30000);
			} catch (InterruptedException ignore) {
			}
    	}

    	// Create new DID and publish to ID sidechain.
    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	Boolean success = store.publishDid(doc, TestConfig.storePass);
    	assertTrue(success);
    	System.out.println("Published new DID: " + doc.getSubject());
    	System.out.println("DID transaction id:" + doc.getTransactionId());

    	// Resolve new DID document
    	DIDDocument resolved;
		System.out.print("Try to resolve new published DID");
    	while (true) {
    		try {
	    		resolved = store.resolveDid(doc.getSubject(), true);
	    		if (resolved != null) {
	    			System.out.println(" OK");
	    			break;
	    		} else {
	    			System.out.print(".");
	    		}
    		} catch (Exception ignore) {
    			System.out.print("x");
    		}

    		try {
				Thread.sleep(30000);
			} catch (InterruptedException ignore) {
			}
    	}

    	assertEquals(doc.getSubject(), resolved.getSubject());
    	assertTrue(resolved.isValid());
	}

	@Test
	public void testUpdateAndResolve() throws DIDException {
    	TestData testData = new TestData();
    	testData.setupStore(false);
    	testData.initIdentity();

    	DIDStore store = DIDStore.getInstance();
    	assertTrue(store.getAdapter() instanceof SPVAdapter);
    	SPVAdapter adapter = (SPVAdapter)store.getAdapter();

		System.out.print("Waiting for wallet available to create DID");
    	while (true) {
    		if (adapter.isAvailable()) {
    			System.out.println(" OK");
    			break;
    		} else {
    			System.out.print(".");
    		}

    		try {
				Thread.sleep(30000);
			} catch (InterruptedException ignore) {
			}
    	}

    	// Create new DID and publish to ID sidechain.
    	DIDDocument doc = store.newDid(TestConfig.storePass);
    	Boolean success = store.publishDid(doc, TestConfig.storePass);
    	assertTrue(success);
    	System.out.println("Published new DID: " + doc.getSubject());

    	// Resolve new DID document
		System.out.print("Waiting for create transaction confirm");
    	while (true) {
      		try {
    			Thread.sleep(30000);
    		} catch (InterruptedException ignore) {
    		}

    		if (adapter.isAvailable()) {
    			System.out.println(" OK");
    			break;
    		} else {
    			System.out.print(".");
    		}
    	}

    	DIDDocument resolved;
    	System.out.print("Try to resolve new published DID");
    	while (true) {
    		try {
	    		resolved = store.resolveDid(doc.getSubject(), true);
	    		if (resolved != null) {
	    			System.out.println(" OK");
	    			break;
	    		} else {
	    			System.out.print(".");
	    		}
    		} catch (Exception ignore) {
    			System.out.print("x");
    		}

    		try {
				Thread.sleep(30000);
			} catch (InterruptedException ignore) {
			}
    	}

    	assertEquals(doc.getSubject(), resolved.getSubject());
    	assertTrue(resolved.isValid());

    	// Update
		System.out.print("Waiting for wallet available to update DID");
    	while (true) {
    		if (adapter.isAvailable()) {
    			System.out.println(" OK");
    			break;
    		} else {
    			System.out.print(".");
    		}

    		try {
				Thread.sleep(30000);
			} catch (InterruptedException ignore) {
			}
    	}

    	DIDDocument.Builder db = resolved.edit();
    	HDKey.DerivedKey key = TestData.generateKeypair();
    	db.addAuthenticationKey("key1", key.getPublicKeyBase58());
    	DIDDocument newDoc = db.seal(TestConfig.storePass);
    	assertEquals(2, newDoc.getPublicKeyCount());
    	assertEquals(2, newDoc.getAuthenticationKeyCount());

    	store.updateDid(newDoc, TestConfig.storePass);

		System.out.print("Waiting for update transaction confirm");
    	while (true) {
    		try {
				Thread.sleep(30000);
			} catch (InterruptedException ignore) {
			}

    		if (adapter.isAvailable()) {
    			System.out.println(" OK");
    			break;
    		} else {
    			System.out.print(".");
    		}
    	}

		System.out.print("Try to resolve updated DID.");
    	while (true) {
    		try {
	    		resolved = store.resolveDid(doc.getSubject(), true);
	    		if (resolved != null) {
	    			System.out.println(" OK");
	    			break;
	    		} else {
	    			System.out.print(".");
	    		}
    		} catch (Exception ignore) {
    			System.out.print("x");
    		}

    		try {
				Thread.sleep(30000);
			} catch (InterruptedException ignore) {
			}
    	}

    	assertEquals(doc.getSubject(), resolved.getSubject());
    	assertTrue(resolved.isValid());

    	resolved = store.resolveDid(doc.getSubject(), true);
    	assertNotNull(resolved);
    	assertEquals(newDoc.toString(), resolved.toString());

    	// Update
		System.out.print("Waiting for wallet available to update DID");
    	while (true) {
    		if (adapter.isAvailable()) {
    			System.out.println(" OK");
    			break;
    		} else {
    			System.out.print(".");
    		}

    		try {
				Thread.sleep(30000);
			} catch (InterruptedException ignore) {
			}
    	}

    	db = resolved.edit();
    	key = TestData.generateKeypair();
    	db.addAuthenticationKey("key2", key.getPublicKeyBase58());
    	newDoc = db.seal(TestConfig.storePass);
    	assertEquals(3, newDoc.getPublicKeyCount());
    	assertEquals(3, newDoc.getAuthenticationKeyCount());

    	store.updateDid(newDoc, TestConfig.storePass);

		System.out.print("Waiting for update transaction confirm");
    	while (true) {
    		try {
				Thread.sleep(30000);
			} catch (InterruptedException ignore) {
			}

    		if (adapter.isAvailable()) {
    			System.out.println(" OK");
    			break;
    		} else {
    			System.out.print(".");
    		}
    	}

		System.out.print("Try to resolve updated DID.");
    	while (true) {
    		try {
	    		resolved = store.resolveDid(doc.getSubject(), true);
	    		if (resolved != null) {
	    			System.out.println(" OK");
	    			break;
	    		} else {
	    			System.out.print(".");
	    		}
    		} catch (Exception ignore) {
    			System.out.print("x");
    		}

    		try {
				Thread.sleep(30000);
			} catch (InterruptedException ignore) {
			}
    	}

    	assertEquals(doc.getSubject(), resolved.getSubject());
    	assertTrue(resolved.isValid());

    	resolved = store.resolveDid(doc.getSubject(), true);
    	assertNotNull(resolved);
    	assertEquals(newDoc.toString(), resolved.toString());
	}

	@Test(timeout = 900000)
	public void testRestore() throws DIDException, IOException {
    	TestData testData = new TestData();
    	testData.setupStore(false);

    	DIDStore store = DIDStore.getInstance();

    	String mnemonic = testData.loadRestoreMnemonic();

    	store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
    			TestConfig.passphrase, TestConfig.storePass, true);

    	store.synchronize(TestConfig.storePass);

    	List<DID> dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
    	assertEquals(5, dids.size());

    	ArrayList<String> didStrings = new ArrayList<String>(dids.size());
    	for (DID id : dids)
    		didStrings.add(id.toString());

    	BufferedReader input = new BufferedReader(new InputStreamReader(
				getClass().getClassLoader().getResourceAsStream("testdata/dids.restore")));

    	String did;
    	while ((did = input.readLine()) != null) {
    		assertTrue(didStrings.contains(did));
    	}

		input.close();
	}
}
