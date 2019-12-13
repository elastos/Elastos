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
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import org.elastos.did.DIDDocument;
import org.elastos.did.DIDURL;
import org.elastos.did.Issuer;
import org.elastos.did.VerifiableCredential;
import org.elastos.did.exception.DIDException;
import org.elastos.did.util.HDKey;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class IssuerTest {
	@Rule
	public ExpectedException expectedEx = ExpectedException.none();

	@Test
	public void newIssuerTestWithSignKey() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setupStore(true);

		DIDDocument issuerDoc = testData.loadTestIssuer();

		DIDURL signKey = issuerDoc.getDefaultPublicKey();

		Issuer issuer = new Issuer(issuerDoc.getSubject(), signKey);

		assertEquals(issuer.getDid(), issuer.getDid());
		assertEquals(signKey, issuer.getSignKey());
	}

	@Test
	public void newIssuerTestWithoutSignKey() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setupStore(true);

		DIDDocument issuerDoc = testData.loadTestIssuer();

		Issuer issuer = new Issuer(issuerDoc.getSubject());

		assertEquals(issuerDoc.getSubject(), issuer.getDid());
		assertEquals(issuerDoc.getDefaultPublicKey(), issuer.getSignKey());
	}

	@Test
	public void newIssuerTestWithInvalidKey() throws DIDException, IOException {
		expectedEx.expect(DIDException.class);
		expectedEx.expectMessage("No private key.");

		TestData testData = new TestData();
		testData.setupStore(true);

		DIDDocument issuerDoc = testData.loadTestIssuer();
		DIDDocument.Builder db = issuerDoc.edit();

		HDKey.DerivedKey key = TestData.generateKeypair();
		DIDURL signKey = new DIDURL(issuerDoc.getSubject(), "testKey");
		db.addAuthenticationKey(signKey, key.getPublicKeyBase58());

		issuerDoc = db.seal(TestConfig.storePass);
		assertTrue(issuerDoc.isValid());

		Issuer issuer = new Issuer(issuerDoc, signKey);

		// Dead code.
		assertEquals(issuer.getDid(), issuer.getDid());
	}

	@Test
	public void newIssuerTestWithInvalidKey2() throws DIDException, IOException {
		expectedEx.expect(DIDException.class);
		expectedEx.expectMessage("Invalid sign key id.");

		TestData testData = new TestData();
		testData.setupStore(true);

		DIDDocument issuerDoc = testData.loadTestIssuer();
		DIDURL signKey = new DIDURL(issuerDoc.getSubject(), "recovery");
		Issuer issuer = new Issuer(issuerDoc, signKey);

		// Dead code.
		assertEquals(issuer.getDid(), issuer.getDid());

	}

	@Test
	public void IssueKycCredentialTest() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setupStore(true);

		DIDDocument issuerDoc = testData.loadTestIssuer();
		DIDDocument testDoc = testData.loadTestDocument();

		Map<String, String> props= new HashMap<String, String>();
		props.put("name", "John");
		props.put("gender", "Male");
		props.put("nation", "Singapore");
		props.put("language", "English");
		props.put("email", "john@example.com");
		props.put("twitter", "@john");

		Issuer issuer = new Issuer(issuerDoc);

		Issuer.CredentialBuilder cb = issuer.issueFor(testDoc.getSubject());
		VerifiableCredential vc = cb.id("testCredential")
			.type("BasicProfileCredential", "InternetAccountCredential")
			.properties(props)
			.seal(TestConfig.storePass);

		DIDURL vcId = new DIDURL(testDoc.getSubject(), "testCredential");

		assertEquals(vcId, vc.getId());

		assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("InternetAccountCredential"));
		assertFalse(Arrays.asList(vc.getTypes()).contains("SelfProclaimedCredential"));

		assertEquals(issuerDoc.getSubject(), vc.getIssuer());
		assertEquals(testDoc.getSubject(), vc.getSubject().getId());

		assertEquals("John", vc.getSubject().getProperty("name"));
		assertEquals("Male", vc.getSubject().getProperty("gender"));
		assertEquals("Singapore", vc.getSubject().getProperty("nation"));
		assertEquals("English", vc.getSubject().getProperty("language"));
		assertEquals("john@example.com", vc.getSubject().getProperty("email"));
		assertEquals("@john", vc.getSubject().getProperty("twitter"));

		assertFalse(vc.isExpired());
		assertTrue(vc.isGenuine());
		assertTrue(vc.isValid());
	}

	@Test
	public void IssueSelfProclaimedCredentialTest() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setupStore(true);

		DIDDocument issuerDoc = testData.loadTestIssuer();

		Map<String, String> props= new HashMap<String, String>();
		props.put("name", "Testing Issuer");
		props.put("nation", "Singapore");
		props.put("language", "English");
		props.put("email", "issuer@example.com");

		Issuer issuer = new Issuer(issuerDoc);

		Issuer.CredentialBuilder cb = issuer.issueFor(issuerDoc.getSubject());
		VerifiableCredential vc = cb.id("myCredential")
			.type("BasicProfileCredential", "SelfProclaimedCredential")
			.properties(props)
			.seal(TestConfig.storePass);

		DIDURL vcId = new DIDURL(issuerDoc.getSubject(), "myCredential");

		assertEquals(vcId, vc.getId());

		assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("SelfProclaimedCredential"));
		assertFalse(Arrays.asList(vc.getTypes()).contains("InternetAccountCredential"));

		assertEquals(issuerDoc.getSubject(), vc.getIssuer());
		assertEquals(issuerDoc.getSubject(), vc.getSubject().getId());

		assertEquals("Testing Issuer", vc.getSubject().getProperty("name"));
		assertEquals("Singapore", vc.getSubject().getProperty("nation"));
		assertEquals("English", vc.getSubject().getProperty("language"));
		assertEquals("issuer@example.com", vc.getSubject().getProperty("email"));

		assertFalse(vc.isExpired());
		assertTrue(vc.isGenuine());
		assertTrue(vc.isValid());
	}
}
