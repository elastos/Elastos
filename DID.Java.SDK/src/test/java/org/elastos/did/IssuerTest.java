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
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

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
		DIDStore store = testData.setupStore(true);

		DIDDocument issuerDoc = testData.loadTestIssuer();

		DIDURL signKey = issuerDoc.getDefaultPublicKey();

		Issuer issuer = new Issuer(issuerDoc.getSubject(), signKey, store);

		assertEquals(issuerDoc.getSubject(), issuer.getDid());
		assertEquals(signKey, issuer.getSignKey());
	}

	@Test
	public void newIssuerTestWithoutSignKey() throws DIDException, IOException {
		TestData testData = new TestData();
		DIDStore store = testData.setupStore(true);

		DIDDocument issuerDoc = testData.loadTestIssuer();

		Issuer issuer = new Issuer(issuerDoc.getSubject(), store);

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
		assertEquals(issuerDoc.getSubject(), issuer.getDid());
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

		assertEquals("John", vc.getSubject().getPropertyAsString("name"));
		assertEquals("Male", vc.getSubject().getPropertyAsString("gender"));
		assertEquals("Singapore", vc.getSubject().getPropertyAsString("nation"));
		assertEquals("English", vc.getSubject().getPropertyAsString("language"));
		assertEquals("john@example.com", vc.getSubject().getPropertyAsString("email"));
		assertEquals("@john", vc.getSubject().getPropertyAsString("twitter"));

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

		assertEquals("Testing Issuer", vc.getSubject().getPropertyAsString("name"));
		assertEquals("Singapore", vc.getSubject().getPropertyAsString("nation"));
		assertEquals("English", vc.getSubject().getPropertyAsString("language"));
		assertEquals("issuer@example.com", vc.getSubject().getPropertyAsString("email"));

		assertFalse(vc.isExpired());
		assertTrue(vc.isGenuine());
		assertTrue(vc.isValid());
	}

	@Test
	public void IssueJsonPropsCredentialTest()
			throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setupStore(true);

		DIDDocument issuerDoc = testData.loadTestIssuer();

		String props = "{\"name\":\"Jay Holtslander\",\"alternateName\":\"Jason Holtslander\",\"booleanValue\":true,\"numberValue\":1234,\"doubleValue\":9.5,\"nationality\":\"Canadian\",\"birthPlace\":{\"type\":\"Place\",\"address\":{\"type\":\"PostalAddress\",\"addressLocality\":\"Vancouver\",\"addressRegion\":\"BC\",\"addressCountry\":\"Canada\"}},\"affiliation\":[{\"type\":\"Organization\",\"name\":\"Futurpreneur\",\"sameAs\":[\"https://twitter.com/futurpreneur\",\"https://www.facebook.com/futurpreneur/\",\"https://www.linkedin.com/company-beta/100369/\",\"https://www.youtube.com/user/CYBF\"]}],\"alumniOf\":[{\"type\":\"CollegeOrUniversity\",\"name\":\"Vancouver Film School\",\"sameAs\":\"https://en.wikipedia.org/wiki/Vancouver_Film_School\",\"year\":2000},{\"type\":\"CollegeOrUniversity\",\"name\":\"CodeCore Bootcamp\"}],\"gender\":\"Male\",\"Description\":\"Technologist\",\"disambiguatingDescription\":\"Co-founder of CodeCore Bootcamp\",\"jobTitle\":\"Technical Director\",\"worksFor\":[{\"type\":\"Organization\",\"name\":\"Skunkworks Creative Group Inc.\",\"sameAs\":[\"https://twitter.com/skunkworks_ca\",\"https://www.facebook.com/skunkworks.ca\",\"https://www.linkedin.com/company/skunkworks-creative-group-inc-\",\"https://plus.google.com/+SkunkworksCa\"]}],\"url\":\"https://jay.holtslander.ca\",\"image\":\"https://s.gravatar.com/avatar/961997eb7fd5c22b3e12fb3c8ca14e11?s=512&r=g\",\"address\":{\"type\":\"PostalAddress\",\"addressLocality\":\"Vancouver\",\"addressRegion\":\"BC\",\"addressCountry\":\"Canada\"},\"sameAs\":[\"https://twitter.com/j_holtslander\",\"https://pinterest.com/j_holtslander\",\"https://instagram.com/j_holtslander\",\"https://www.facebook.com/jay.holtslander\",\"https://ca.linkedin.com/in/holtslander/en\",\"https://plus.google.com/+JayHoltslander\",\"https://www.youtube.com/user/jasonh1234\",\"https://github.com/JayHoltslander\",\"https://profiles.wordpress.org/jasonh1234\",\"https://angel.co/j_holtslander\",\"https://www.foursquare.com/user/184843\",\"https://jholtslander.yelp.ca\",\"https://codepen.io/j_holtslander/\",\"https://stackoverflow.com/users/751570/jay\",\"https://dribbble.com/j_holtslander\",\"http://jasonh1234.deviantart.com/\",\"https://www.behance.net/j_holtslander\",\"https://www.flickr.com/people/jasonh1234/\",\"https://medium.com/@j_holtslander\"]}";

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

		assertEquals("Technologist", vc.getSubject().getPropertyAsString("Description"));
		assertEquals("Jason Holtslander", vc.getSubject().getPropertyAsString("alternateName"));
		assertEquals("1234", vc.getSubject().getPropertyAsString("numberValue"));
		assertEquals("9.5", vc.getSubject().getPropertyAsString("doubleValue"));

		assertNotNull(vc.getSubject().getProperties());

		assertFalse(vc.isExpired());
		assertTrue(vc.isGenuine());
		assertTrue(vc.isValid());
	}
}
