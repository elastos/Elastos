package org.elastos.did;

import static org.junit.Assert.assertTrue;

import java.io.File;

import org.elastos.did.adapter.SPVAdapter;

public class BulkCreate {
	private static DIDAdapter adapter;

	//@BeforeClass
	public static void setup() throws DIDStoreException {
		adapter = new SPVAdapter(TestConfig.walletDir, TestConfig.walletId,
				TestConfig.networkConfig, TestConfig.resolver,
				new SPVAdapter.PasswordCallback() {

					@Override
					public String getPassword(String walletDir, String walletId) {
						return "helloworld";
					}
				});

    	TestData.deleteFile(new File(TestConfig.storeRoot));
    	DIDStore.initialize("filesystem", TestConfig.storeRoot, adapter);
	}

	//@Test
	public void test30PrepareForRestore() throws DIDStoreException, InterruptedException, MalformedDocumentException {
    	DIDStore store = DIDStore.getInstance();

    	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
    	store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
    			TestConfig.passphrase, TestConfig.storePass, true);

    	System.out.println("Mnemonic: " + mnemonic);

		for (int i = 0; i < 10; i++) {
    		System.out.print("******** Waiting for wallet available");
	    	while (true) {
	    		if (((SPVAdapter)adapter).isAvailable()) {
	    			System.out.println(" OK");
	    			break;
	    		} else {
	    			System.out.print(".");
	    		}

	    		Thread.sleep(30000);
	    	}

    		String hint = "my did " + i;
    		DIDDocument doc = store.newDid(TestConfig.storePass, hint);

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

	    	store.publishDid(doc, TestConfig.storePass);
	    	System.out.println(doc.getSubject());
	    	System.out.print("******** Waiting for DID available");
	    	while (true) {
	    		Thread.sleep(30000);
	    		try {
		    		DIDDocument d = store.resolveDid(doc.getSubject(), true);
		    		if (d != null) {
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
	}
}

