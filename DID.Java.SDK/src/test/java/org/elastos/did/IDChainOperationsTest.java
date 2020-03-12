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
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;

import org.elastos.did.crypto.HDKey;
import org.elastos.did.exception.DIDException;
import org.junit.jupiter.api.Test;

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
		DIDStore store = testData.setup(DUMMY_TEST);
		testData.initIdentity();
		testData.waitForWalletAvaliable();

		// Create new DID and publish to ID sidechain.
		DIDDocument doc = store.newDid(TestConfig.storePass);
		DID did = doc.getSubject();

		System.out.print("Publishing new DID: " + did + "...");
		long start = System.currentTimeMillis();
		String txid = store.publishDid(did, 1, TestConfig.storePass);
		long duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");
		assertNotNull(txid);

		testData.waitForWalletAvaliable();
		DIDDocument resolved = did.resolve(true);
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));
	}

	@Test
	public void testPublishAndResolveAsync() throws DIDException {
		TestData testData = new TestData();
		DIDStore store = testData.setup(DUMMY_TEST);
		testData.initIdentity();
		testData.waitForWalletAvaliable();

		// Create new DID and publish to ID sidechain.
		DIDDocument doc = store.newDid(TestConfig.storePass);
		DID did = doc.getSubject();

		System.out.print("Publishing new DID: " + did + "...");
		long start = System.currentTimeMillis();
		CompletableFuture<String> tf = store.publishDidAsync(did, 1, TestConfig.storePass)
				.thenApply((tx) -> {
					long duration = (System.currentTimeMillis() - start + 500) / 1000;
					System.out.println("OK(" + duration + "s)");
					return tx;
				});
		String txid = tf.join();
		assertNotNull(txid);

		testData.waitForWalletAvaliable();
		CompletableFuture<DIDDocument> rf = did.resolveAsync(true);
		DIDDocument resolved = rf.join();
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));
	}

	@Test
	public void testPublishAndResolveAsync2() throws DIDException {
		TestData testData = new TestData();
		DIDStore store = testData.setup(DUMMY_TEST);
		testData.initIdentity();
		testData.waitForWalletAvaliable();

		// Create new DID and publish to ID sidechain.
		DIDDocument doc = store.newDid(TestConfig.storePass);
		DID did = doc.getSubject();

		System.out.print("Publishing new DID and resolve: " + did + "...");
		long start = System.currentTimeMillis();
		CompletableFuture<DIDDocument> tf = store.publishDidAsync(did, 1, TestConfig.storePass)
				.thenCompose((tx) -> did.resolveAsync(true));
		DIDDocument resolved = tf.join();
		long duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");

		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));
	}

	@Test
	public void testUpdateAndResolve() throws DIDException {
		String[] txids = new String[3];
		String[] sigs = new String[3];

		TestData testData = new TestData();
		DIDStore store = testData.setup(DUMMY_TEST);
		testData.initIdentity();
		testData.waitForWalletAvaliable();

		// Create new DID and publish to ID sidechain.
		DIDDocument doc = store.newDid(TestConfig.storePass);
		DID did = doc.getSubject();

		System.out.print("Publishing new DID: " + did + "...");
		long start = System.currentTimeMillis();
		String txid = store.publishDid(did, 1, TestConfig.storePass);
		long duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");
		assertNotNull(txid);

		txids[2] = txid;
		sigs[2] = doc.getProof().getSignature();

		testData.waitForWalletAvaliable();
		DIDDocument resolved = did.resolve(true);
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		String lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update
		DIDDocument.Builder db = doc.edit();
		HDKey.DerivedKey key = TestData.generateKeypair();
		db.addAuthenticationKey("key1", key.getPublicKeyBase58());
		doc = db.seal(TestConfig.storePass);
		assertEquals(2, doc.getPublicKeyCount());
		assertEquals(2, doc.getAuthenticationKeyCount());
		store.storeDid(doc);

		System.out.print("Updating DID: " + did + "...");
		start = System.currentTimeMillis();
		txid = store.publishDid(did, 1, TestConfig.storePass);
		duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");
		assertNotNull(txid);

		txids[1] = txid;
		sigs[1] = doc.getProof().getSignature();

		testData.waitForWalletAvaliable();
		resolved = did.resolve(true);
		assertNotEquals(lastTxid, resolved.getTransactionId());
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update again
		db = doc.edit();
		key = TestData.generateKeypair();
		db.addAuthenticationKey("key2", key.getPublicKeyBase58());
		doc = db.seal(TestConfig.storePass);
		assertEquals(3, doc.getPublicKeyCount());
		assertEquals(3, doc.getAuthenticationKeyCount());
		store.storeDid(doc);

		System.out.print("Updating DID: " + did + "...");
		start = System.currentTimeMillis();
		txid = store.publishDid(did, 1, TestConfig.storePass);
		duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");
		assertNotNull(txid);

		txids[0] = txid;
		sigs[0] = doc.getProof().getSignature();

		testData.waitForWalletAvaliable();
		resolved = did.resolve(true);
		assertNotEquals(lastTxid, resolved.getTransactionId());
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		DIDHistory his = did.resolveHistory();
		assertNotNull(his);
		assertEquals(did, his.getDid());
		assertEquals(DIDHistory.STATUS_VALID, his.getStatus());
		assertEquals(3, his.getTransactionCount());
		List<DIDTransaction> txs = his.getAllTransactions();
		assertNotNull(txs);
		assertEquals(3, txs.size());

		for (int i = 0; i < txs.size(); i++) {
			DIDTransaction tx = txs.get(i);
			assertEquals(did, tx.getDid());
			assertEquals(txids[i], tx.getTransactionId());
			assertEquals(sigs[i], tx.getDocument().getProof().getSignature());
		}
	}

	@Test
	public void testUpdateAndResolveAsync() throws DIDException {
		String[] txids = new String[3];
		String[] sigs = new String[3];

		TestData testData = new TestData();
		DIDStore store = testData.setup(DUMMY_TEST);
		testData.initIdentity();
		testData.waitForWalletAvaliable();

		// Create new DID and publish to ID sidechain.
		DIDDocument doc = store.newDid(TestConfig.storePass);
		DID did = doc.getSubject();

		System.out.print("Publishing new DID: " + did + "...");
		long s1 = System.currentTimeMillis();
		CompletableFuture<String> tf = store.publishDidAsync(did, 1, TestConfig.storePass)
				.thenApply((tx) -> {
					long duration = (System.currentTimeMillis() - s1 + 500) / 1000;
					System.out.println("OK(" + duration + "s)");
					return tx;
				});
		String txid = tf.join();
		assertNotNull(txid);

		txids[2] = txid;
		sigs[2] = doc.getProof().getSignature();

		testData.waitForWalletAvaliable();
		CompletableFuture<DIDDocument> rf = did.resolveAsync(true);
		DIDDocument resolved = rf.join();
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		String lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update
		DIDDocument.Builder db = doc.edit();
		HDKey.DerivedKey key = TestData.generateKeypair();
		db.addAuthenticationKey("key1", key.getPublicKeyBase58());
		doc = db.seal(TestConfig.storePass);
		assertEquals(2, doc.getPublicKeyCount());
		assertEquals(2, doc.getAuthenticationKeyCount());
		store.storeDid(doc);

		System.out.print("Updating DID: " + did + "...");
		long s2 = System.currentTimeMillis();
		tf = store.publishDidAsync(did, 1, TestConfig.storePass)
				.thenApply((tx) -> {
					long duration = (System.currentTimeMillis() - s2 + 500) / 1000;
					System.out.println("OK(" + duration + "s)");
					return tx;
				});
		txid = tf.join();
		assertNotNull(txid);

		txids[1] = txid;
		sigs[1] = doc.getProof().getSignature();

		testData.waitForWalletAvaliable();
		rf = did.resolveAsync(true);
		resolved = rf.join();
		assertNotEquals(lastTxid, resolved.getTransactionId());
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update again
		db = doc.edit();
		key = TestData.generateKeypair();
		db.addAuthenticationKey("key2", key.getPublicKeyBase58());
		doc = db.seal(TestConfig.storePass);
		assertEquals(3, doc.getPublicKeyCount());
		assertEquals(3, doc.getAuthenticationKeyCount());
		store.storeDid(doc);

		System.out.print("Updating DID: " + did + "...");
		long s3 = System.currentTimeMillis();
		tf = store.publishDidAsync(did, 1, TestConfig.storePass)
				.thenApply((tx) -> {
					long duration = (System.currentTimeMillis() - s3 + 500) / 1000;
					System.out.println("OK(" + duration + "s)");
					return tx;
				});
		txid = tf.join();
		assertNotNull(txid);

		txids[0] = txid;
		sigs[0] = doc.getProof().getSignature();

		testData.waitForWalletAvaliable();
		rf = did.resolveAsync(true);
		resolved = rf.join();
		assertNotEquals(lastTxid, resolved.getTransactionId());
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		CompletableFuture<DIDHistory> hf = did.resolveHistoryAsync();
		DIDHistory his = hf.join();
		assertNotNull(his);
		assertEquals(did, his.getDid());
		assertEquals(DIDHistory.STATUS_VALID, his.getStatus());
		assertEquals(3, his.getTransactionCount());
		List<DIDTransaction> txs = his.getAllTransactions();
		assertNotNull(txs);
		assertEquals(3, txs.size());

		for (int i = 0; i < txs.size(); i++) {
			DIDTransaction tx = txs.get(i);
			assertEquals(did, tx.getDid());
			assertEquals(txids[i], tx.getTransactionId());
			assertEquals(sigs[i], tx.getDocument().getProof().getSignature());
		}

	}

	@Test
	public void testUpdateAndResolveWithCredentials() throws DIDException {
		TestData testData = new TestData();
		DIDStore store = testData.setup(DUMMY_TEST);
		testData.initIdentity();
		testData.waitForWalletAvaliable();

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
		store.storeDid(doc);

		System.out.print("Publishing new DID: " + did + "...");
		long start = System.currentTimeMillis();
		String txid = store.publishDid(did, 1, TestConfig.storePass);
		long duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");
		assertNotNull(txid);

		testData.waitForWalletAvaliable();
		DIDDocument resolved = did.resolve(true);
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		String lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update
		selfIssuer = new Issuer(doc);
		cb = selfIssuer.issueFor(did);

		props.clear();
		props.put("nation", "Singapore");
		props.put("passport", "S653258Z07");

		vc = cb.id("passport")
				.type("BasicProfileCredential", "SelfProclaimedCredential")
				.properties(props)
				.seal(TestConfig.storePass);
		assertNotNull(vc);

		db = doc.edit();
		db.addCredential(vc);
		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertEquals(2, doc.getCredentialCount());
		store.storeDid(doc);

		System.out.print("Updating DID: " + did + "...");
		start = System.currentTimeMillis();
		txid = store.publishDid(did, 1, TestConfig.storePass);
		duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");
		assertNotNull(txid);

		testData.waitForWalletAvaliable();
		resolved = did.resolve(true);
		assertNotEquals(lastTxid, resolved.getTransactionId());
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update again
		selfIssuer = new Issuer(doc);
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

		db = doc.edit();
		db.addCredential(vc);
		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertEquals(3, doc.getCredentialCount());
		store.storeDid(doc);

		System.out.print("Updating DID: " + did + "...");
		start = System.currentTimeMillis();
		txid = store.publishDid(did, 1, TestConfig.storePass);
		duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");
		assertNotNull(txid);

		testData.waitForWalletAvaliable();
		resolved = did.resolve(true);
		assertNotEquals(lastTxid, resolved.getTransactionId());
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);
	}

	@Test
	public void testUpdateAndResolveWithCredentialsAsync() throws DIDException {
		TestData testData = new TestData();
		DIDStore store = testData.setup(DUMMY_TEST);
		testData.initIdentity();
		testData.waitForWalletAvaliable();

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
		store.storeDid(doc);

		System.out.print("Publishing new DID: " + did + "...");
		long s1 = System.currentTimeMillis();
		CompletableFuture<String> tf = store.publishDidAsync(did, 1, TestConfig.storePass)
				.thenApply((tx) -> {
					long duration = (System.currentTimeMillis() - s1 + 500) / 1000;
					System.out.println("OK(" + duration + "s)");
					return tx;
				});
		String txid = tf.join();
		assertNotNull(txid);

		testData.waitForWalletAvaliable();
		CompletableFuture<DIDDocument> rf = did.resolveAsync(true);
		DIDDocument resolved = rf.join();
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		String lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update
		selfIssuer = new Issuer(doc);
		cb = selfIssuer.issueFor(did);

		props.clear();
		props.put("nation", "Singapore");
		props.put("passport", "S653258Z07");

		vc = cb.id("passport")
				.type("BasicProfileCredential", "SelfProclaimedCredential")
				.properties(props)
				.seal(TestConfig.storePass);
		assertNotNull(vc);

		db = doc.edit();
		db.addCredential(vc);
		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertEquals(2, doc.getCredentialCount());
		store.storeDid(doc);

		System.out.print("Updating DID: " + did + "...");
		long s2 = System.currentTimeMillis();
		tf = store.publishDidAsync(did, 1, TestConfig.storePass)
				.thenApply((tx) -> {
					long duration = (System.currentTimeMillis() - s2 + 500) / 1000;
					System.out.println("OK(" + duration + "s)");
					return tx;
				});
		txid = tf.join();
		assertNotNull(txid);

		testData.waitForWalletAvaliable();
		rf = did.resolveAsync(true);
		resolved = rf.join();
		assertNotEquals(lastTxid, resolved.getTransactionId());
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);

		// Update again
		selfIssuer = new Issuer(doc);
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

		db = doc.edit();
		db.addCredential(vc);
		doc = db.seal(TestConfig.storePass);
		assertNotNull(doc);
		assertEquals(3, doc.getCredentialCount());
		store.storeDid(doc);

		System.out.print("Updating DID: " + did + "...");
		long s3 = System.currentTimeMillis();
		tf = store.publishDidAsync(did, 1, TestConfig.storePass)
				.thenApply((tx) -> {
					long duration = (System.currentTimeMillis() - s3 + 500) / 1000;
					System.out.println("OK(" + duration + "s)");
					return tx;
				});
		txid = tf.join();
		assertNotNull(txid);

		testData.waitForWalletAvaliable();
		rf = did.resolveAsync(true);
		resolved = rf.join();
		assertNotEquals(lastTxid, resolved.getTransactionId());
		assertEquals(did, resolved.getSubject());
		assertTrue(resolved.isValid());
		assertEquals(doc.toString(true), resolved.toString(true));

		lastTxid = resolved.getTransactionId();
		System.out.println("Last transaction id: " + lastTxid);
	}

	@Test
	public void testRestore() throws DIDException, IOException {
		if (DUMMY_TEST)
			return;

		TestData testData = new TestData();
		DIDStore store = testData.setup(false);

		String mnemonic = testData.loadRestoreMnemonic();

		store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
				TestConfig.passphrase, TestConfig.storePass, true);

		System.out.print("Synchronizing from IDChain...");
		long start = System.currentTimeMillis();
		store.synchronize(TestConfig.storePass);
		long duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");

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

	@Test
	public void testRestoreAsync() throws DIDException, IOException {
		if (DUMMY_TEST)
			return;

		TestData testData = new TestData();
		DIDStore store = testData.setup(false);

		String mnemonic = testData.loadRestoreMnemonic();

		store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
				TestConfig.passphrase, TestConfig.storePass, true);

		System.out.print("Synchronizing from IDChain...");
		long start = System.currentTimeMillis();
		CompletableFuture<Void> f = store.synchronizeAsync(TestConfig.storePass)
				.thenRun(() -> {
					long duration = (System.currentTimeMillis() - start + 500) / 1000;
					System.out.println("OK(" + duration + "s)");
				});

		f.join();

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

	@Test
	public void testSyncWithLocalModification1() throws DIDException, IOException {
		if (DUMMY_TEST)
			return;

		TestData testData = new TestData();
		DIDStore store = testData.setup(false);

		String mnemonic = testData.loadRestoreMnemonic();

		store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
				TestConfig.passphrase, TestConfig.storePass, true);

		System.out.print("Synchronizing from IDChain...");
		long start = System.currentTimeMillis();
		store.synchronize(TestConfig.storePass);
		long duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");

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
			DIDDocument d = store.loadDid(did);
			assertNotNull(d);
			assertEquals(did, d.getSubject());
			assertEquals(4, d.getCredentialCount());

			List<DIDURL> vcs = store.listCredentials(did);
			assertEquals(4, vcs.size());

			for (DIDURL id : vcs) {
				VerifiableCredential vc = store.loadCredential(did, id);
				assertNotNull(vc);
				assertEquals(id, vc.getId());
			}
		}

		input.close();

		DID modifiedDid = dids.get(0);
		DIDDocument doc = store.loadDid(modifiedDid);
		DIDDocument.Builder db = doc.edit();
		db.addService("test1", "TestType", "http://test.com/");
		doc = db.seal(TestConfig.storePass);
		store.storeDid(doc);
		String modifiedSignature = doc.getProof().getSignature();

		System.out.print("Synchronizing again from IDChain...");
		start = System.currentTimeMillis();
		store.synchronize(TestConfig.storePass);
		duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");

		dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		assertEquals(5, dids.size());

		didStrings = new ArrayList<String>(dids.size());
		for (DID id : dids)
			didStrings.add(id.toString());

		input = new BufferedReader(new InputStreamReader(
				getClass().getClassLoader().getResourceAsStream("testdata/dids.restore")));

		while ((didstr = input.readLine()) != null) {
			assertTrue(didStrings.contains(didstr));

			DID did = new DID(didstr);
			DIDDocument d = store.loadDid(did);
			assertNotNull(d);
			assertEquals(did, d.getSubject());
			assertEquals(4, d.getCredentialCount());

			List<DIDURL> vcs = store.listCredentials(did);
			assertEquals(4, vcs.size());

			for (DIDURL id : vcs) {
				VerifiableCredential vc = store.loadCredential(did, id);
				assertNotNull(vc);
				assertEquals(id, vc.getId());
			}
		}

		input.close();

		doc = store.loadDid(modifiedDid);
		assertEquals(modifiedSignature, doc.getProof().getSignature());
	}

	@Test
	public void testSyncWithLocalModification2() throws DIDException, IOException {
		if (DUMMY_TEST)
			return;

		TestData testData = new TestData();
		DIDStore store = testData.setup(false);

		String mnemonic = testData.loadRestoreMnemonic();

		store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
				TestConfig.passphrase, TestConfig.storePass, true);

		System.out.print("Synchronizing from IDChain...");
		long start = System.currentTimeMillis();
		store.synchronize(TestConfig.storePass);
		long duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");

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
			DIDDocument d = store.loadDid(did);
			assertNotNull(d);
			assertEquals(did, d.getSubject());
			assertEquals(4, d.getCredentialCount());

			List<DIDURL> vcs = store.listCredentials(did);
			assertEquals(4, vcs.size());

			for (DIDURL id : vcs) {
				VerifiableCredential vc = store.loadCredential(did, id);
				assertNotNull(vc);
				assertEquals(id, vc.getId());
			}
		}

		input.close();

		DID modifiedDid = dids.get(0);
		DIDDocument doc = store.loadDid(modifiedDid);
		String originSignature = doc.getProof().getSignature();

		DIDDocument.Builder db = doc.edit();
		db.addService("test1", "TestType", "http://test.com/");
		doc = db.seal(TestConfig.storePass);
		store.storeDid(doc);
		assertNotEquals(originSignature, doc.getProof().getSignature());

		System.out.print("Synchronizing again from IDChain...");
		start = System.currentTimeMillis();
		store.synchronize((c, l) -> c, TestConfig.storePass);
		duration = (System.currentTimeMillis() - start + 500) / 1000;
		System.out.println("OK(" + duration + "s)");

		dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		assertEquals(5, dids.size());

		didStrings = new ArrayList<String>(dids.size());
		for (DID id : dids)
			didStrings.add(id.toString());

		input = new BufferedReader(new InputStreamReader(
				getClass().getClassLoader().getResourceAsStream("testdata/dids.restore")));

		while ((didstr = input.readLine()) != null) {
			assertTrue(didStrings.contains(didstr));

			DID did = new DID(didstr);
			DIDDocument d = store.loadDid(did);
			assertNotNull(d);
			assertEquals(did, d.getSubject());
			assertEquals(4, d.getCredentialCount());

			List<DIDURL> vcs = store.listCredentials(did);
			assertEquals(4, vcs.size());

			for (DIDURL id : vcs) {
				VerifiableCredential vc = store.loadCredential(did, id);
				assertNotNull(vc);
				assertEquals(id, vc.getId());
			}
		}

		input.close();

		doc = store.loadDid(modifiedDid);
		assertEquals(originSignature, doc.getProof().getSignature());
	}

	@Test
	public void testSyncWithLocalModificationAsync() throws DIDException, IOException {
		if (DUMMY_TEST)
			return;

		TestData testData = new TestData();
		DIDStore store = testData.setup(false);

		String mnemonic = testData.loadRestoreMnemonic();

		store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
				TestConfig.passphrase, TestConfig.storePass, true);

		System.out.print("Synchronizing from IDChain...");
		long s1 = System.currentTimeMillis();
		CompletableFuture<Void> f = store.synchronizeAsync(TestConfig.storePass)
				.thenRun(() -> {
					long duration = (System.currentTimeMillis() - s1 + 500) / 1000;
					System.out.println("OK(" + duration + "s)");
				});

		f.join();

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
			DIDDocument d = store.loadDid(did);
			assertNotNull(d);
			assertEquals(did, d.getSubject());
			assertEquals(4, d.getCredentialCount());

			List<DIDURL> vcs = store.listCredentials(did);
			assertEquals(4, vcs.size());

			for (DIDURL id : vcs) {
				VerifiableCredential vc = store.loadCredential(did, id);
				assertNotNull(vc);
				assertEquals(id, vc.getId());
			}
		}

		input.close();

		DID modifiedDid = dids.get(0);
		DIDDocument doc = store.loadDid(modifiedDid);
		String originSignature = doc.getProof().getSignature();

		DIDDocument.Builder db = doc.edit();
		db.addService("test1", "TestType", "http://test.com/");
		doc = db.seal(TestConfig.storePass);
		store.storeDid(doc);
		assertNotEquals(originSignature, doc.getProof().getSignature());

		System.out.print("Synchronizing again from IDChain...");
		long s2 = System.currentTimeMillis();
		f = store.synchronizeAsync((c, l) -> c, TestConfig.storePass)
				.thenRun(() -> {
					long duration = (System.currentTimeMillis() - s2 + 500) / 1000;
					System.out.println("OK(" + duration + "s)");
				});

		f.join();

		dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		assertEquals(5, dids.size());

		didStrings = new ArrayList<String>(dids.size());
		for (DID id : dids)
			didStrings.add(id.toString());

		input = new BufferedReader(new InputStreamReader(
				getClass().getClassLoader().getResourceAsStream("testdata/dids.restore")));

		while ((didstr = input.readLine()) != null) {
			assertTrue(didStrings.contains(didstr));

			DID did = new DID(didstr);
			DIDDocument d = store.loadDid(did);
			assertNotNull(d);
			assertEquals(did, d.getSubject());
			assertEquals(4, d.getCredentialCount());

			List<DIDURL> vcs = store.listCredentials(did);
			assertEquals(4, vcs.size());

			for (DIDURL id : vcs) {
				VerifiableCredential vc = store.loadCredential(did, id);
				assertNotNull(vc);
				assertEquals(id, vc.getId());
			}
		}

		input.close();

		doc = store.loadDid(modifiedDid);
		assertEquals(originSignature, doc.getProof().getSignature());
	}
}
