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

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.HashMap;
import java.util.Map;

import org.elastos.did.adapter.SPVAdapter;
import org.elastos.did.backend.ResolverCache;
import org.elastos.did.exception.DIDException;
import org.elastos.did.util.Base58;
import org.elastos.did.util.HDKey;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class TestDataGenerator {
	private File testDataDir;
	private DIDAdapter adapter;
	private DIDDocument issuer;
	private DIDDocument test;
	private DIDStore store;

	private String init(String storeRoot) throws IOException, DIDException {
		adapter = new SPVAdapter(TestConfig.walletDir,
				TestConfig.walletId, TestConfig.networkConfig,
				TestConfig.resolver, new SPVAdapter.PasswordCallback() {
					@Override
					public String getPassword(String walletDir, String walletId) {
						return TestConfig.walletPassword;
					}
				});

		ResolverCache.reset();
		DIDBackend.initialize(adapter);

		TestData.deleteFile(new File(storeRoot));
		store = DIDStore.open("filesystem", storeRoot);

    	String mnemonic = Mnemonic.generate(Mnemonic.ENGLISH);
    	store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
    			TestConfig.passphrase, TestConfig.storePass, true);

    	testDataDir = new File(TestConfig.tempDir + File.separator +
    			"DIDTestFiles" + File.separator + "testdata");
    	testDataDir.mkdirs();


    	return mnemonic;
	}

	private void createTestIssuer() throws DIDException, IOException {
		DIDDocument doc = store.newDid(TestConfig.storePass, "Issuer");

		System.out.print("Generate issuer DID: " + doc.getSubject() + "...");

		Issuer selfIssuer = new Issuer(doc);
		Issuer.CredentialBuilder cb = selfIssuer.issueFor(doc.getSubject());

		Map<String, String> props= new HashMap<String, String>();
		props.put("name", "Test Issuer");
		props.put("nation", "Singapore");
		props.put("language", "English");
		props.put("email", "issuer@example.com");

		VerifiableCredential vc = cb.id("profile")
				.type("BasicProfileCredential", "SelfProclaimedCredential")
				.properties(props)
				.seal(TestConfig.storePass);

		DIDDocument.Builder db = doc.edit();
		db.addCredential(vc);
		issuer = db.seal(TestConfig.storePass);
		store.storeDid(issuer);
		store.storeCredential(vc, "Profile");

		DIDURL id = issuer.getDefaultPublicKey();
		String sk = store.loadPrivateKey(issuer.getSubject(), id);
		byte[] binSk = DIDStore.decryptFromBase64(TestConfig.storePass, sk);
		writeTo("issuer." + id.getFragment() + ".sk", Base58.encode(binSk));

		String json = issuer.toString(true);
		writeTo("issuer.normalized.json", json);

		json = formatJson(json);
		writeTo("issuer.json", json);

		json = issuer.toString(false);
		writeTo("issuer.compact.json", json);

		System.out.println(issuer.isValid() ? "OK" : "Error");
	}

	private void createTestDocument() throws DIDException, IOException {
		DIDDocument doc = store.newDid(TestConfig.storePass);

		// Test document with two embedded credentials
		System.out.print("Generate test DID: " + doc.getSubject() + "...");

		DIDDocument.Builder db = doc.edit();

		HDKey.DerivedKey temp = TestData.generateKeypair();
		db.addAuthenticationKey("key2", temp.getPublicKeyBase58());
		writeTo("document.key2.sk", Base58.encode(temp.serialize()));

		temp = TestData.generateKeypair();
		db.addAuthenticationKey("key3", temp.getPublicKeyBase58());
		writeTo("document.key3.sk", Base58.encode(temp.serialize()));

		temp = TestData.generateKeypair();
		db.addAuthorizationKey("recovery",
				new DID(DID.METHOD, temp.getAddress()).toString(),
				temp.getPublicKeyBase58());

		db.addService("openid", "OpenIdConnectVersion1.0Service",
				"https://openid.example.com/");
		db.addService("vcr", "CredentialRepositoryService",
				"https://did.example.com/credentials");
		db.addService("carrier", "CarrierAddress",
				"carrier://X2tDd1ZTErwnHNot8pTdhp7C7Y9FxMPGD8ppiasUT4UsHH2BpF1d");

		Issuer selfIssuer = new Issuer(doc);
		Issuer.CredentialBuilder cb = selfIssuer.issueFor(doc.getSubject());

		Map<String, String> props= new HashMap<String, String>();
		props.put("name", "John");
		props.put("gender", "Male");
		props.put("nation", "Singapore");
		props.put("language", "English");
		props.put("email", "john@example.com");
		props.put("twitter", "@john");

		VerifiableCredential vcProfile = cb.id("profile")
				.type("BasicProfileCredential", "SelfProclaimedCredential")
				.properties(props)
				.seal(TestConfig.storePass);

		Issuer kycIssuer = new Issuer(issuer);
		cb = kycIssuer.issueFor(doc.getSubject());

		props= new HashMap<String, String>();
		props.put("email", "john@example.com");

		VerifiableCredential vcEmail = cb.id("email")
				.type("BasicProfileCredential",
						"InternetAccountCredential", "EmailCredential")
				.properties(props)
				.seal(TestConfig.storePass);

		db.addCredential(vcProfile);
		db.addCredential(vcEmail);
		test = db.seal(TestConfig.storePass);
		store.storeDid(test, "Test");
		store.storeCredential(vcProfile, "Profile");
		store.storeCredential(vcEmail, "Email");

		DIDURL id = test.getDefaultPublicKey();
		String sk = store.loadPrivateKey(test.getSubject(), id);
		byte[] binSk = DIDStore.decryptFromBase64(TestConfig.storePass, sk);
		writeTo("document." + id.getFragment() + ".sk", Base58.encode(binSk));

		String json = test.toString(true);
		writeTo("document.normalized.json", json);

		json = formatJson(json);
		writeTo("document.json", json);

		json = test.toString(false);
		writeTo("document.compact.json", json);

		System.out.println(test.isValid() ? "OK" : "Error");

		// Profile credential
		System.out.print("Generate credential: " + vcProfile.getId() + "...");
		json = vcProfile.toString(true);
		writeTo("vc-profile.normalized.json", json);

		json = formatJson(json);
		writeTo("vc-profile.json", json);

		json = vcProfile.toString(false);
		writeTo("vc-profile.compact.json", json);

		System.out.println(vcProfile.isValid() ? "OK" : "Error");

		// email credential
		System.out.print("Generate credential: " + vcEmail.getId() + "...");
		json = vcEmail.toString(true);
		writeTo("vc-email.normalized.json", json);

		json = formatJson(json);
		writeTo("vc-email.json", json);

		json = vcEmail.toString(false);
		writeTo("vc-email.compact.json", json);

		System.out.println(vcEmail.isValid() ? "OK" : "Error");

		// Passport credential
		id = new DIDURL(test.getSubject(), "passport");
		System.out.print("Generate credential: " + id + "...");

		cb = selfIssuer.issueFor(doc.getSubject());

		props= new HashMap<String, String>();
		props.put("nation", "Singapore");
		props.put("passport", "S653258Z07");

		VerifiableCredential vcPassport = cb.id(id)
				.type("BasicProfileCredential", "SelfProclaimedCredential")
				.properties(props)
				.seal(TestConfig.storePass);
		store.storeCredential(vcPassport, "Passport");

		json = vcPassport.toString(true);
		writeTo("vc-passport.normalized.json", json);

		json = formatJson(json);
		writeTo("vc-passport.json", json);

		json = vcPassport.toString(false);
		writeTo("vc-passport.compact.json", json);

		System.out.println(vcPassport.isValid() ? "OK" : "Error");

		// Twitter credential
		id = new DIDURL(test.getSubject(), "twitter");
		System.out.print("Generate credential: " + id + "...");

		cb = kycIssuer.issueFor(doc.getSubject());

		props= new HashMap<String, String>();
		props.put("twitter", "@john");

		VerifiableCredential vcTwitter = cb.id(id)
				.type("InternetAccountCredential", "TwitterCredential")
				.properties(props)
				.seal(TestConfig.storePass);
		store.storeCredential(vcTwitter, "Twitter");

		json = vcTwitter.toString(true);
		writeTo("vc-twitter.normalized.json", json);

		json = formatJson(json);
		writeTo("vc-twitter.json", json);

		json = vcTwitter.toString(false);
		writeTo("vc-twitter.compact.json", json);

		System.out.println(vcPassport.isValid() ? "OK" : "Error");

		// Presentation with above credentials
		System.out.print("Generate presentation...");

		VerifiablePresentation.Builder pb = VerifiablePresentation.createFor(
				test.getSubject(), store);

		VerifiablePresentation vp = pb.credentials(vcProfile, vcEmail)
				.credentials(vcPassport)
				.credentials(vcTwitter)
				.realm("https://example.com/")
				.nonce("873172f58701a9ee686f0630204fee59")
				.seal(TestConfig.storePass);

		json = vp.toString();
		writeTo("vp.normalized.json", json);

		json = formatJson(json);
		writeTo("vp.json", json);

		System.out.println(vcPassport.isValid() ? "OK" : "Error");
	}

	public void createTestFiles() throws IOException, DIDException {
		init(TestConfig.tempDir + File.separator +
    			"DIDTestFiles" + File.separator + "teststore");
		createTestIssuer();
		createTestDocument();
	}

	public void createTestDidsForRestore()
			throws IOException, DIDException, InterruptedException {
		System.out.print("Generate mnemonic for restore...");
		String mnemonic = init(TestConfig.storeRoot);
		writeTo("mnemonic.restore", mnemonic);
		System.out.println("OK");

    	StringBuffer dids = new StringBuffer();

    	System.out.println("Generate DIDs for restore......");
		for (int i = 0; i < 5; i++) {
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

	    	DIDDocument doc = store.newDid(TestConfig.storePass);
    		System.out.print("******** Publishing DID: " + doc.getSubject());
	    	store.publishDid(doc, TestConfig.storePass);
	    	while (true) {
	    		Thread.sleep(30000);
	    		try {
		    		DIDDocument d = doc.getSubject().resolve(true);
		    		if (d != null) {
		    			store.storeDid(d);
		    			System.out.println(" OK");
		    			break;
		    		} else {
		    			System.out.print(".");
		    		}
	    		} catch (Exception ignore) {
	    			System.out.print("x");
	    		}
	    	}

	    	dids.append(doc.getSubject()).append("\n");
    	}

		writeTo("dids.restore", dids.toString().trim());
		System.out.println("Generate DIDs for restore......OK");
	}

	private String formatJson(String json) throws IOException {
		ObjectMapper mapper = new ObjectMapper();
		JsonNode node = mapper.readTree(json);
		json = mapper.writerWithDefaultPrettyPrinter().writeValueAsString(node);
		return json;
	}

	private void writeTo(String fileName, String content) throws IOException {
		Writer out = new FileWriter(testDataDir.getPath()
				+ File.separator + fileName);
		out.write(content);
		out.close();

	}

	public static void main(String[] argc) throws Exception {
		TestDataGenerator bc = new TestDataGenerator();

		bc.createTestFiles();
		bc.createTestDidsForRestore();
	}
}
