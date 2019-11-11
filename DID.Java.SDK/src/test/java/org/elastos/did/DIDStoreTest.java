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
import static org.junit.Assert.fail;

import java.io.File;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.elastos.credential.Issuer;
import org.elastos.credential.VerifiableCredential;
import org.elastos.did.DIDStore.Entry;
import org.elastos.did.adaptor.SPVAdaptor;
import org.elastos.did.util.Mnemonic;
import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class DIDStoreTest {
	private static String storeRoot = "/PATH/TO/DIDStore";
	private static String storePass = "passwd";
	private static String passphrase = "secret";
	private static DIDStore store;
	private static DIDAdaptor adaptor;

	private static String walletDir = "/PATH/TO/wallet";
	private static String walletId = "test";
	private static String networkConfig = "/PATH/TO/privnet.json";
	private static String resolver = "https://coreservices-didsidechain-privnet.elastos.org";
	private static LinkedHashMap<DID, String> ids;

	private static DID primaryDid;

	public static void waitForDidRegisted(DID did) throws DIDException {
		do {
			System.out.println("Waiting for DID '" + did + "' registed.");

			DIDDocument doc = store.resolveDid(did, true);
			if (doc == null) {
				try {
					Thread.sleep(5000);
				} catch (InterruptedException ignore) {
				}
			} else
				break;
		} while (true);
	}

	@BeforeClass
	public static void setup() {
		//adaptor = new FakeConsoleAdaptor();
		adaptor = new SPVAdaptor(walletDir, walletId, networkConfig, resolver,
				new SPVAdaptor.PasswordCallback() {

					@Override
					public String getPassword(String walletDir, String walletId) {
						return "helloworld";
					}
				});
	}

	@Test
	public void test00CreateEmptyStore0() throws DIDStoreException {
		String tempStoreRoot = "/Users/jingyu/Temp/TestDIDStore";

    	Util.deleteFile(new File(tempStoreRoot));

    	DIDStore.initialize("filesystem", tempStoreRoot, storePass, adaptor);

    	DIDStore tempStore = DIDStore.getInstance();

    	assertFalse(tempStore.hasPrivateIdentity());

    	File file = new File(tempStoreRoot);
    	assertTrue(file.exists());
    	assertTrue(file.isDirectory());

    	file = new File(tempStoreRoot + File.separator + ".DIDStore");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	assertFalse(tempStore.hasPrivateIdentity());
	}

	@Test(expected = DIDStoreException.class)
	public void test00CreateEmptyStore1() throws DIDStoreException {
		String tempStoreRoot = "/Users/jingyu/Temp/TestDIDStore";

    	DIDStore.initialize("filesystem", tempStoreRoot, storePass, adaptor);

    	DIDStore tempStore = DIDStore.getInstance();

    	tempStore.newDid(storePass, "my first did");
	}

	@Test
	public void test10InitPrivateIdentity0() throws DIDStoreException {
		String tempStoreRoot = "/Users/jingyu/Temp/TestDIDStore";

		Util.deleteFile(new File(tempStoreRoot));

    	DIDStore.initialize("filesystem", tempStoreRoot, storePass, adaptor);

    	DIDStore tempStore = DIDStore.getInstance();

    	assertFalse(tempStore.hasPrivateIdentity());

    	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
    	tempStore.initPrivateIdentity(mnemonic, passphrase, storePass, true);

    	File file = new File(tempStoreRoot + File.separator + "private"
    			+ File.separator + "key");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(tempStoreRoot + File.separator + "private"
    			+ File.separator + "index");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	assertTrue(tempStore.hasPrivateIdentity());

    	DIDStore.initialize("filesystem", tempStoreRoot, storePass, adaptor);

    	tempStore = DIDStore.getInstance();

    	assertTrue(tempStore.hasPrivateIdentity());
	}

	// Can not decrypt root private key because wrong storepass
	@Test(expected = DIDStoreException.class)
	public void test10InitPrivateIdentity1() throws DIDStoreException {
		String tempStoreRoot = "/Users/jingyu/Temp/TestDIDStore";

		DIDStore.initialize("filesystem", tempStoreRoot, "password", adaptor);

    	DIDStore tempStore = DIDStore.getInstance();

    	assertTrue(tempStore.hasPrivateIdentity());
	}

    @Test
    public void test20Setup() throws DIDStoreException {
    	Util.deleteFile(new File(storeRoot));

    	DIDStore.initialize("filesystem", storeRoot, storePass, adaptor);

    	store = DIDStore.getInstance();

    	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
    	store.initPrivateIdentity(mnemonic, passphrase, storePass, true);

    	ids = new LinkedHashMap<DID, String>(128);
    }

	@Test
	public void test30CreateDID1() throws DIDException {
		String hint = "my first did";

    	DIDDocument doc = store.newDid(storePass, hint);
    	primaryDid = doc.getSubject();
    	store.publishDid(doc, storePass);

    	File file = new File(storeRoot + File.separator + "ids"
    			+ File.separator + doc.getSubject().getMethodSpecificId()
    			+ File.separator + "document");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(storeRoot + File.separator + "ids"
    			+ File.separator + "."
    			+ doc.getSubject().getMethodSpecificId() + ".meta");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	ids.put(doc.getSubject(), hint);
	}

	@Test
	public void test30CreateDID2() throws DIDStoreException {
    	DIDDocument doc = store.newDid(storePass, null);

    	File file = new File(storeRoot + File.separator + "ids"
    			+ File.separator + doc.getSubject().getMethodSpecificId()
    			+ File.separator + "document");
    	assertTrue(file.exists());
    	assertTrue(file.isFile());

    	file = new File(storeRoot + File.separator + "ids"
    			+ File.separator + "."
    			+ doc.getSubject().getMethodSpecificId() + ".meta");
    	assertFalse(file.exists());

    	ids.put(doc.getSubject(), null);
	}

	@Test
	public void test30CreateDID3() throws DIDStoreException {
    	for (int i = 0; i < 100; i++) {
    		String hint = "my did " + i;
    		DIDDocument doc = store.newDid(storePass, hint);

	    	File file = new File(storeRoot + File.separator + "ids"
	    			+ File.separator + doc.getSubject().getMethodSpecificId()
	    			+ File.separator + "document");
	    	assertTrue(file.exists());
	    	assertTrue(file.isFile());

	    	file = new File(storeRoot + File.separator + "ids"
	    			+ File.separator + "."
	    			+ doc.getSubject().getMethodSpecificId() + ".meta");
	    	assertTrue(file.exists());
	    	assertTrue(file.isFile());

	    	ids.put(doc.getSubject(), hint);
    	}
	}

	@Test
	public void test40DeleteDID1() throws DIDStoreException {
		Iterator<DID> dids = ids.keySet().iterator();
		int i = 0;

		while (dids.hasNext()) {
			DID did = dids.next();

			if (++i % 9 != 0 || did.equals(primaryDid))
				continue;

    		boolean deleted = store.deleteDid(did);
    		assertTrue(deleted);

	    	File file = new File(storeRoot + File.separator + "ids"
	    			+ File.separator + did.getMethodSpecificId());
	    	assertFalse(file.exists());

	    	file = new File(storeRoot + File.separator + "ids"
	    			+ File.separator + "."
	    			+ did.getMethodSpecificId() + ".meta");
	    	assertFalse(file.exists());

    		deleted = store.deleteDid(did);
    		assertFalse(deleted);

	    	dids.remove();
    	}
	}

	@Test
	public void test40PublishDID() throws DIDStoreException, MalformedDocumentException {
		Iterator<DID> dids = ids.keySet().iterator();
		int i = 0;

		while (dids.hasNext()) {
			DID did = dids.next();

			if (++i % 9 != 0)
				continue;

			DIDDocument doc = store.loadDid(did);
    		store.publishDid(doc, new DIDURL(did, "primary"), storePass);
    	}
	}

	@Test
	public void test50IssueSelfClaimCredential1() throws DIDException {
		Issuer issuer = new Issuer(primaryDid);

		Map<String, String> props = new HashMap<String, String>();
		props.put("name", "Elastos");
		props.put("email", "contact@elastos.org");
		props.put("website", "https://www.elastos.org/");
		props.put("phone", "12345678900");

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.YEAR, cal.get(Calendar.YEAR) + 1);
		Date expire = cal.getTime();
		VerifiableCredential vc = issuer.issueFor(primaryDid)
				.id("cred-1")
				.type(new String[] {"SelfProclaimedCredential", "BasicProfileCredential" })
				.expirationDate(expire)
				.properties(props)
				.sign(storePass);

		DIDDocument doc = store.resolveDid(primaryDid);
		doc.modify();
		doc.addCredential(vc);
		store.storeDid(doc);

		doc = store.resolveDid(primaryDid);
		DIDURL vcId = new DIDURL(primaryDid, "cred-1");
		vc = doc.getCredential(vcId);
		assertNotNull(vc);
		assertEquals(vcId, vc.getId());
		assertEquals(primaryDid, vc.getSubject().getId());
	}

	@Test
	public void test50IssueSelfClaimCredential2() throws DIDException {
		DID issuerDid = primaryDid;
		Issuer issuer = new Issuer(issuerDid);

		for (DID did : ids.keySet()) {
			Map<String, String> props = new HashMap<String, String>();
			props.put("name", "Elastos-" + did.getMethodSpecificId());
			props.put("email", "contact@elastos.org");
			props.put("website", "https://www.elastos.org/");
			props.put("phone", did.getMethodSpecificId());

			Calendar cal = Calendar.getInstance();
			cal.set(Calendar.YEAR, cal.get(Calendar.YEAR) + 1);
			Date expire = cal.getTime();
			VerifiableCredential vc = issuer.issueFor(did)
					.id("cred-1")
					.type(new String[] { "BasicProfileCredential" })
					.expirationDate(expire)
					.properties(props)
					.sign(storePass);

			store.storeCredential(vc, "default");

			props.clear();
			props.put("name", "CyberRepublic-" + did.getMethodSpecificId());
			props.put("email", "contact@CyberRepublic.org");
			props.put("website", "https://www.CyberRepublic.org/");
			props.put("phone", did.getMethodSpecificId());

			vc = issuer.issueFor(did)
				.id("cred-2")
				.type(new String[] { "BasicProfileCredential" })
				.expirationDate(expire)
				.properties(props)
				.sign(storePass);

			store.storeCredential(vc);

	    	File file = new File(storeRoot + File.separator + "ids"
	    			+ File.separator + did.getMethodSpecificId() + File.separator
	    			+ "credentials" + File.separator + "cred-1");
	    	assertTrue(file.exists());
	    	assertTrue(file.isFile());
	    	assertTrue(file.length() > 0);

	    	file = new File(storeRoot + File.separator + "ids"
	    			+ File.separator + did.getMethodSpecificId() + File.separator
	    			+ "credentials" + File.separator + ".cred-1.meta");
	    	assertTrue(file.exists());
	    	assertTrue(file.isFile());
	    	assertTrue(file.length() > 0);

	    	file = new File(storeRoot + File.separator + "ids"
	    			+ File.separator + did.getMethodSpecificId() + File.separator
	    			+ "credentials" + File.separator + "cred-2");
	    	assertTrue(file.exists());
	    	assertTrue(file.isFile());
	    	assertTrue(file.length() > 0);

	    	file = new File(storeRoot + File.separator + "ids"
	    			+ File.separator + did.getMethodSpecificId() + File.separator
	    			+ "credentials" + File.separator + ".cred-2.meta");
	    	assertFalse(file.exists());
		}
	}

	@Test
	public void test60DeleteCredential1() throws DIDException {
		boolean deleted = store.deleteCredential(primaryDid, new DIDURL(primaryDid, "cred-1"));
		assertTrue(deleted);

		deleted = store.deleteCredential(primaryDid, new DIDURL(primaryDid, "cred-2"));
		assertTrue(deleted);

		deleted = store.deleteCredential(primaryDid, new DIDURL(primaryDid, "cred-3"));
		assertFalse(deleted);

    	File file = new File(storeRoot + File.separator + "ids"
    			+ File.separator + primaryDid.getMethodSpecificId() + File.separator
    			+ "credentials" + File.separator + "cred-1");
    	assertFalse(file.exists());

    	file = new File(storeRoot + File.separator + "ids"
    			+ File.separator + primaryDid.getMethodSpecificId() + File.separator
    			+ "credentials" + File.separator + ".cred-1.meta");
    	assertFalse(file.exists());

    	file = new File(storeRoot + File.separator + "ids"
    			+ File.separator + primaryDid.getMethodSpecificId() + File.separator
    			+ "credentials" + File.separator + "cred-2");
    	assertFalse(file.exists());

    	file = new File(storeRoot + File.separator + "ids"
    			+ File.separator + primaryDid.getMethodSpecificId() + File.separator
    			+ "credentials" + File.separator + ".cred-2.meta");
    	assertFalse(file.exists());

	}

	@Test
	public void test60ListCredential1() throws DIDException {
		for (DID did : ids.keySet()) {
			List<Entry<DIDURL, String>> creds = store.listCredentials(did);

			if (did.equals(primaryDid))
				assertEquals(0, creds.size());
			else
				assertEquals(2, creds.size());

			for (Entry<DIDURL, String> cred : creds) {
				if (cred.getKey().getFragment().equals("cred-1"))
					assertEquals("default", cred.getValue());
				else if (cred.getKey().getFragment().equals("cred-2"))
					assertNull(cred.getValue());
				else
					fail("Unexpected credential id '" + cred.getKey() + "'.");
			}
		}
	}

	@Test
	public void test60LoadCredential1() throws DIDException {
		for (DID did : ids.keySet()) {
			if (did.equals(primaryDid))
				continue;

			DIDURL id1 = new DIDURL(did, "cred-1");
			VerifiableCredential vc1 = store.loadCredential(did, id1);
			assertNotNull(vc1);

			DIDURL id2 = new DIDURL(did, "cred-2");
			VerifiableCredential vc2 = store.loadCredential(did, id2);
			assertNotNull(vc2);

			assertEquals(id1, vc1.getId());
			assertEquals(primaryDid, vc1.getIssuer());
			assertEquals(did, vc1.getSubject().getId());
			assertEquals("Elastos-" + did.getMethodSpecificId(), vc1.getSubject().getProperty("name"));

			assertEquals(id2, vc2.getId());
			assertEquals(primaryDid, vc2.getIssuer());
			assertEquals(did, vc2.getSubject().getId());
			assertEquals("CyberRepublic-" + did.getMethodSpecificId(), vc2.getSubject().getProperty("name"));
		}
	}

    @Test
    public void test99Check() throws DIDStoreException {
    	List<DIDStore.Entry<DID, String>> dids = store.listDids(DIDStore.DID_ALL);

    	assertEquals(ids.size(), dids.size());

    	for (DIDStore.Entry<DID, String> entry : dids) {
    		assertTrue(ids.containsKey(entry.getKey()));
    		assertEquals(ids.get(entry.getKey()), entry.getValue());
    	}
    }
}
