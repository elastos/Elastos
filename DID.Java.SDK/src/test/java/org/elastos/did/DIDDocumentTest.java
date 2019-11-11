package org.elastos.did;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;

import org.elastos.did.util.Mnemonic;
import org.junit.Test;

public class DIDDocumentTest {
	private static String storeRoot = "/Users/jingyu/Temp/DIDStore";
	private static String storePass = "passwd";
	private static String passphrase = "secret";
	private static DIDStore store;

	@Test
	public void testParseDocument() throws DIDException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream("testdiddoc.json"));
		DIDDocument doc = DIDDocument.fromJson(input);

		assertEquals(4, doc.getPublicKeyCount());

		List<PublicKey> pks = doc.getPublicKeys();
		for (PublicKey pk : pks) {
			assertTrue(pk.getId().getFragment().equals("default")
					|| pk.getId().getFragment().equals("key2")
					|| pk.getId().getFragment().equals("keys3")
					|| pk.getId().getFragment().equals("recovery"));

			if (pk.getId().getFragment().equals("recovery"))
				assertNotEquals(doc.getSubject(), pk.getController());
			else
				assertEquals(doc.getSubject(), pk.getController());
		}

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
		Util.deleteFile(new File(storeRoot));
    	DIDStore.initialize("filesystem", storeRoot, storePass,
    						new FakeConsoleAdaptor());
    	store = DIDStore.getInstance();
    	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
    	store.initPrivateIdentity(mnemonic, passphrase, storePass, true);

    	LinkedHashMap<DID, String> ids = new LinkedHashMap<DID, String>(128);
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

    	Iterator<DID> dids = ids.keySet().iterator();
		while (dids.hasNext()) {
			DID did = dids.next();

			DIDDocument doc = DIDStore.getInstance().loadDid(did);
			String json = doc.toExternalForm(false);
			DIDURL pkid = new DIDURL(did, "primary");

			String sig = doc.sign(pkid, storePass, json.getBytes());
			boolean result = doc.verify(pkid, sig, json.getBytes());
			assertTrue(result);

			result = doc.verify(pkid, sig, json.substring(1).getBytes());
			assertFalse(result);

			sig = doc.sign(storePass, json.getBytes());
			result = doc.verify(sig, json.getBytes());
			assertTrue(result);

			result = doc.verify(sig, json.substring(1).getBytes());
			assertFalse(result);
		}
	}
}
