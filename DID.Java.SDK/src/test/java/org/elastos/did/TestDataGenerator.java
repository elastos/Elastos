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
import org.elastos.did.crypto.Base58;
import org.elastos.did.crypto.HDKey;
import org.elastos.did.exception.DIDException;

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
				new SPVAdapter.PasswordCallback() {
					@Override
					public String getPassword(String walletDir, String walletId) {
						return TestConfig.walletPassword;
					}
				});

		DIDBackend.initialize(TestConfig.resolver, TestData.getResolverCacheDir());
		ResolverCache.reset();

		Utils.deleteFile(new File(storeRoot));
		store = DIDStore.open("filesystem", storeRoot, adapter);

    	String mnemonic =  Mnemonic.getInstance().generate();
    	store.initPrivateIdentity(Mnemonic.ENGLISH, mnemonic,
    			TestConfig.passphrase, TestConfig.storePass, true);

    	testDataDir = new File(TestConfig.tempDir + File.separator +
    			"DIDTestFiles" + File.separator + "testdata");
    	testDataDir.mkdirs();

    	return mnemonic;
	}

	private void createTestIssuer() throws DIDException, IOException {
		DIDDocument doc = store.newDid("Issuer", TestConfig.storePass);

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
		byte[] binSk = store.loadPrivateKey(issuer.getSubject(), id, TestConfig.storePass);
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
		byte[] binSk = store.loadPrivateKey(test.getSubject(), id, TestConfig.storePass);
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

		//System.out.println(vcProfile.isValid() ? "OK" : "Error");
		System.out.println("OK");

		// email credential
		System.out.print("Generate credential: " + vcEmail.getId() + "...");
		json = vcEmail.toString(true);
		writeTo("vc-email.normalized.json", json);

		json = formatJson(json);
		writeTo("vc-email.json", json);

		json = vcEmail.toString(false);
		writeTo("vc-email.compact.json", json);

		//System.out.println(vcEmail.isValid() ? "OK" : "Error");
		System.out.println("OK");

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

		//System.out.println(vcPassport.isValid() ? "OK" : "Error");
		System.out.println("OK");

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

		//System.out.println(vcTwitter.isValid() ? "OK" : "Error");
		System.out.println("OK");

		// Json format credential
		id = new DIDURL(test.getSubject(), "json");
		System.out.print("Generate credential: " + id + "...");

		cb = kycIssuer.issueFor(doc.getSubject());

		String jsonProps = "{\"name\":\"Jay Holtslander\",\"alternateName\":\"Jason Holtslander\",\"booleanValue\":true,\"numberValue\":1234,\"doubleValue\":9.5,\"nationality\":\"Canadian\",\"birthPlace\":{\"type\":\"Place\",\"address\":{\"type\":\"PostalAddress\",\"addressLocality\":\"Vancouver\",\"addressRegion\":\"BC\",\"addressCountry\":\"Canada\"}},\"affiliation\":[{\"type\":\"Organization\",\"name\":\"Futurpreneur\",\"sameAs\":[\"https://twitter.com/futurpreneur\",\"https://www.facebook.com/futurpreneur/\",\"https://www.linkedin.com/company-beta/100369/\",\"https://www.youtube.com/user/CYBF\"]}],\"alumniOf\":[{\"type\":\"CollegeOrUniversity\",\"name\":\"Vancouver Film School\",\"sameAs\":\"https://en.wikipedia.org/wiki/Vancouver_Film_School\",\"year\":2000},{\"type\":\"CollegeOrUniversity\",\"name\":\"CodeCore Bootcamp\"}],\"gender\":\"Male\",\"Description\":\"Technologist\",\"disambiguatingDescription\":\"Co-founder of CodeCore Bootcamp\",\"jobTitle\":\"Technical Director\",\"worksFor\":[{\"type\":\"Organization\",\"name\":\"Skunkworks Creative Group Inc.\",\"sameAs\":[\"https://twitter.com/skunkworks_ca\",\"https://www.facebook.com/skunkworks.ca\",\"https://www.linkedin.com/company/skunkworks-creative-group-inc-\",\"https://plus.google.com/+SkunkworksCa\"]}],\"url\":\"https://jay.holtslander.ca\",\"image\":\"https://s.gravatar.com/avatar/961997eb7fd5c22b3e12fb3c8ca14e11?s=512&r=g\",\"address\":{\"type\":\"PostalAddress\",\"addressLocality\":\"Vancouver\",\"addressRegion\":\"BC\",\"addressCountry\":\"Canada\"},\"sameAs\":[\"https://twitter.com/j_holtslander\",\"https://pinterest.com/j_holtslander\",\"https://instagram.com/j_holtslander\",\"https://www.facebook.com/jay.holtslander\",\"https://ca.linkedin.com/in/holtslander/en\",\"https://plus.google.com/+JayHoltslander\",\"https://www.youtube.com/user/jasonh1234\",\"https://github.com/JayHoltslander\",\"https://profiles.wordpress.org/jasonh1234\",\"https://angel.co/j_holtslander\",\"https://www.foursquare.com/user/184843\",\"https://jholtslander.yelp.ca\",\"https://codepen.io/j_holtslander/\",\"https://stackoverflow.com/users/751570/jay\",\"https://dribbble.com/j_holtslander\",\"http://jasonh1234.deviantart.com/\",\"https://www.behance.net/j_holtslander\",\"https://www.flickr.com/people/jasonh1234/\",\"https://medium.com/@j_holtslander\"]}";

		VerifiableCredential vcJson = cb.id(id)
				.type("InternetAccountCredential", "TwitterCredential")
				.properties(jsonProps)
				.seal(TestConfig.storePass);
		store.storeCredential(vcTwitter, "json");

		json = vcJson.toString(true);
		writeTo("vc-json.normalized.json", json);

		json = formatJson(json);
		writeTo("vc-json.json", json);

		json = vcJson.toString(false);
		writeTo("vc-json.compact.json", json);

		//System.out.println(vcJson.isValid() ? "OK" : "Error");
		System.out.println("OK");

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

		//System.out.println(vp.isValid() ? "OK" : "Error");
		System.out.println("OK");
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

			Issuer selfIssuer = new Issuer(doc);
			Issuer.CredentialBuilder cb = selfIssuer.issueFor(doc.getSubject());

			Map<String, String> props = new HashMap<String, String>();
			props.put("name", "John");
			props.put("nation", "Singapore");
			props.put("language", "English");
			props.put("email", "john@example.com");

			VerifiableCredential vcProfile = cb.id("profile")
					.type("BasicProfileCredential", "SelfProclaimedCredential")
					.properties(props)
					.seal(TestConfig.storePass);

			cb = selfIssuer.issueFor(doc.getSubject());

			props.clear();
			props.put("email", "john@gmail.com");

			VerifiableCredential vcEmail = cb.id("email")
					.type("BasicProfileCredential",
							"InternetAccountCredential", "EmailCredential")
					.properties(props)
					.seal(TestConfig.storePass);

			cb = selfIssuer.issueFor(doc.getSubject());

			props.clear();
			props.put("nation", "Singapore");
			props.put("passport", "S653258Z07");

			VerifiableCredential vcPassport = cb.id("passport")
					.type("BasicProfileCredential", "SelfProclaimedCredential")
					.properties(props)
					.seal(TestConfig.storePass);

			cb = selfIssuer.issueFor(doc.getSubject());

			props.clear();
			props.put("twitter", "@john");

			VerifiableCredential vcTwitter = cb.id("twitter")
					.type("InternetAccountCredential", "TwitterCredential")
					.properties(props)
					.seal(TestConfig.storePass);

			DIDDocument.Builder db = doc.edit();
			db.addCredential(vcProfile);
			db.addCredential(vcEmail);
			db.addCredential(vcPassport);
			db.addCredential(vcTwitter);
			doc = db.seal(TestConfig.storePass);

			store.storeDid(doc);

    		System.out.print("******** Publishing DID: " + doc.getSubject());
	    	store.publishDid(doc.getSubject(), TestConfig.storePass);
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
