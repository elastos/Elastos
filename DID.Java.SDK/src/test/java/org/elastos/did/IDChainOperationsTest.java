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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.elastos.did.adapter.SPVAdapter;
import org.elastos.did.exception.DIDException;
import org.elastos.did.util.HDKey;
import org.junit.Test;

//@RunWith(Parameterized.class)
public class IDChainOperationsTest {
	private static final boolean DUMMY_TEST = false;
	/*
	@Parameterized.Parameters
	public static Object[][] data() {
		return new Object[50][0];
	}
	*/

	@Test
	public void testPublishAndResolve() throws DIDException {
		TestData testData = new TestData();
		DIDStore store = testData.setupStore(DUMMY_TEST);
		testData.initIdentity();

		SPVAdapter adapter = null;

		// need synchronize?
		if (DIDBackend.getInstance().getAdapter() instanceof SPVAdapter)
			adapter = (SPVAdapter)DIDBackend.getInstance().getAdapter();

		if (adapter != null) {
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
		}

		// Create new DID and publish to ID sidechain.
		DIDDocument doc = store.newDid(TestConfig.storePass);
		DID did = doc.getSubject();

		String txid = store.publishDid(doc, TestConfig.storePass);
		assertNotNull(txid);
		System.out.println("Published new DID: " + did);

		// Resolve new DID document
		if (adapter != null) {
			System.out.print("Try to resolve new published DID");
			while (true) {
				try {
					Thread.sleep(30000);
				} catch (InterruptedException ignore) {
				}

				try {
					DIDDocument rdoc = did.resolve(true);
					if (rdoc != null) {
						System.out.println(" OK");
						break;
					} else {
						System.out.print(".");
					}
				} catch (Exception ignore) {
					System.out.print("x");
				}
			}
		}

		DIDDocument resolved = did.resolve(true);
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));
	}

	@Test
	public void testUpdateAndResolve() throws DIDException {
		TestData testData = new TestData();
		DIDStore store = testData.setupStore(DUMMY_TEST);
		testData.initIdentity();

		SPVAdapter adapter = null;

		// need synchronize?
		if (DIDBackend.getInstance().getAdapter() instanceof SPVAdapter)
			adapter = (SPVAdapter)DIDBackend.getInstance().getAdapter();

		if (adapter != null) {
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
		}

		// Create new DID and publish to ID sidechain.
		DIDDocument doc = store.newDid(TestConfig.storePass);
		DID did = doc.getSubject();

		String txid = store.publishDid(doc, TestConfig.storePass);
		assertNotNull(txid);
		System.out.println("Published new DID: " + did);

		// Resolve new DID document
		if (adapter != null) {
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

			System.out.print("Try to resolve new published DID");
			while (true) {
				try {
					DIDDocument rdoc = did.resolve(true);
					if (rdoc != null) {
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
		}

		DIDDocument resolved = did.resolve(true);
		store.storeDid(resolved);
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		String lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update
		DIDDocument.Builder db = resolved.edit();
		HDKey.DerivedKey key = TestData.generateKeypair();
		db.addAuthenticationKey("key1", key.getPublicKeyBase58());
		doc = db.seal(TestConfig.storePass);
		assertEquals(2, doc.getPublicKeyCount());
		assertEquals(2, doc.getAuthenticationKeyCount());

		txid = store.updateDid(doc, TestConfig.storePass);
		assertNotNull(txid);
		System.out.println("Updated DID: " + did);

		if (adapter != null) {
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
					DIDDocument rdoc = did.resolve(true);
					if (rdoc != null && rdoc.getTransactionId() != lastTxid) {
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
		}

		resolved = did.resolve(true);
		store.storeDid(resolved);
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update
		db = resolved.edit();
		key = TestData.generateKeypair();
		db.addAuthenticationKey("key2", key.getPublicKeyBase58());
		doc = db.seal(TestConfig.storePass);
		assertEquals(3, doc.getPublicKeyCount());
		assertEquals(3, doc.getAuthenticationKeyCount());

		txid = store.updateDid(doc, TestConfig.storePass);
		assertNotNull(txid);
		System.out.println("Updated DID: " + did);

		if (adapter != null) {
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
					DIDDocument rdoc = did.resolve(true);
					if (rdoc != null && rdoc.getTransactionId() != lastTxid) {
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
		}

		resolved = did.resolve(true);
		store.storeDid(resolved);
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);
	}

	@Test
	public void testUpdateAndResolveWithCredentials() throws DIDException {
		TestData testData = new TestData();
		DIDStore store = testData.setupStore(DUMMY_TEST);
		String mnemonic = testData.initIdentity();
		System.out.println("Mnemonic: " + mnemonic);

		SPVAdapter adapter = null;

		// need synchronize?
		if (DIDBackend.getInstance().getAdapter() instanceof SPVAdapter)
			adapter = (SPVAdapter)DIDBackend.getInstance().getAdapter();

		if (adapter != null) {
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
		}

		// Create new DID and publish to ID sidechain.
		DIDDocument doc = store.newDid(TestConfig.storePass);
		DID did = doc.getSubject();

		Issuer selfIssuer = new Issuer(doc);
		Issuer.CredentialBuilder cb = selfIssuer.issueFor(did);

		Map<String, String> props= new HashMap<String, String>();
		props.put("name", "John");
		props.put("gender", "Male");
		props.put("nation", "Singapore");
		props.put("language", "English");
		props.put("email", "john@example.com");
		props.put("twitter", "@john");

		VerifiableCredential vc = cb.id("profile")
				.type("BasicProfileCredential", "SelfProclaimedCredential")
				.properties(props)
				.seal(TestConfig.storePass);
		assertNotNull(vc);

		DIDDocument.Builder db = doc.edit();
		db.addCredential(vc);
		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertEquals(1, doc.getCredentialCount());

		String txid = store.publishDid(doc, TestConfig.storePass);
		assertNotNull(txid);
		System.out.println("Published new DID: " + did);

		// Resolve new DID document
		if (adapter != null) {
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

			System.out.print("Try to resolve new published DID");
			while (true) {
				try {
					DIDDocument rdoc = did.resolve(true);
					if (rdoc != null) {
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
		}

		DIDDocument resolved = did.resolve(true);
		store.storeDid(resolved);
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		String lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update
		selfIssuer = new Issuer(resolved);
		cb = selfIssuer.issueFor(did);

		props.clear();
		props.put("nation", "Singapore");
		props.put("passport", "S653258Z07");

		vc = cb.id("passport")
				.type("BasicProfileCredential", "SelfProclaimedCredential")
				.properties(props)
				.seal(TestConfig.storePass);
		assertNotNull(vc);

		db = resolved.edit();
		db.addCredential(vc);
		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertEquals(2, doc.getCredentialCount());

		txid = store.updateDid(doc, TestConfig.storePass);
		assertNotNull(txid);
		System.out.println("Updated DID: " + did);

		if (adapter != null) {
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
					DIDDocument rdoc = did.resolve(true);
					if (rdoc != null && rdoc.getTransactionId() != lastTxid) {
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
		}

		resolved = did.resolve(true);
		store.storeDid(resolved);
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update
		selfIssuer = new Issuer(resolved);
		cb = selfIssuer.issueFor(did);

		props.clear();
		props.put("Abc", "Abc");
		props.put("abc", "abc");
		props.put("Foobar", "Foobar");
		props.put("foobar", "foobar");
		props.put("zoo", "zoo");
		props.put("Zoo", "Zoo");

		vc = cb.id("test")
				.type("TestCredential", "SelfProclaimedCredential")
				.properties(props)
				.seal(TestConfig.storePass);
		assertNotNull(vc);

		db = resolved.edit();
		db.addCredential(vc);
		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertEquals(3, doc.getCredentialCount());

		txid = store.updateDid(doc, TestConfig.storePass);
		assertNotNull(txid);
		System.out.println("Updated DID: " + did);

		if (adapter != null) {
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
					DIDDocument rdoc = did.resolve(true);
					if (rdoc != null && rdoc.getTransactionId() != lastTxid) {
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
		}

		resolved = did.resolve(true);
		store.storeDid(resolved);
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);
	}

	@Test(timeout = 900000)
	public void testRestore() throws DIDException, IOException {
		TestData testData = new TestData();
		DIDStore store = testData.setupStore(false);

		String mnemonic = testData.loadRestoreMnemonic();

		store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
				TestConfig.passphrase, TestConfig.storePass, true);

		System.out.println("Synchronizing from IDChain...");
		store.synchronize(TestConfig.storePass);
		System.out.println("OK");

		List<DID> dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		assertEquals(5, dids.size());

		ArrayList<String> didStrings = new ArrayList<String>(dids.size());
		for (DID id : dids)
			didStrings.add(id.toString());

		BufferedReader input = new BufferedReader(new InputStreamReader(
				getClass().getClassLoader().getResourceAsStream("testdata/dids.restore")));

		String didstr;
		while ((didstr = input.readLine()) != null) {
			assertTrue(didStrings.contains(didstr));

			DID did = new DID(didstr);
			DIDDocument doc = store.loadDid(did);
			assertNotNull(doc);
			assertEquals(did, doc.getSubject());
			assertEquals(4, doc.getCredentialCount());

			List<DIDURL> vcs = store.listCredentials(did);
			assertEquals(4, vcs.size());

			for (DIDURL id : vcs) {
				VerifiableCredential vc = store.loadCredential(did, id);
				assertNotNull(vc);
				assertEquals(id, vc.getId());
			}
		}

		input.close();
	}
}
