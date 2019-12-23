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

import org.elastos.did.exception.DIDException;
import org.junit.Test;

public class VerifiableCredentialTest {
	@Test
	public void TestKycCredential() throws DIDException, IOException {
		TestData testData = new TestData();

		// for integrity check
		testData.setupStore(true);
		DIDDocument issuer = testData.loadTestIssuer();
		DIDDocument test = testData.loadTestDocument();

		VerifiableCredential vc = testData.loadEmailCredential();

		assertEquals(new DIDURL(test.getSubject(), "email"), vc.getId());

		assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("InternetAccountCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("EmailCredential"));

		assertEquals(issuer.getSubject(), vc.getIssuer());
		assertEquals(test.getSubject(), vc.getSubject().getId());

		assertEquals("john@example.com", vc.getSubject().getProperty("email"));

		assertNotNull(vc.getIssuanceDate());
		assertNotNull(vc.getExpirationDate());

		assertFalse(vc.isExpired());
		assertTrue(vc.isGenuine());
		assertTrue(vc.isValid());
	}

	@Test
	public void TestSelfProclaimedCredential() throws DIDException, IOException {
		TestData testData = new TestData();

		// for integrity check
		testData.setupStore(true);
		DIDDocument test = testData.loadTestDocument();

		VerifiableCredential vc = testData.loadProfileCredential();

		assertEquals(new DIDURL(test.getSubject(), "profile"), vc.getId());

		assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("SelfProclaimedCredential"));

		assertEquals(test.getSubject(), vc.getIssuer());
		assertEquals(test.getSubject(), vc.getSubject().getId());

		assertEquals("John", vc.getSubject().getProperty("name"));
		assertEquals("Male", vc.getSubject().getProperty("gender"));
		assertEquals("Singapore", vc.getSubject().getProperty("nation"));
		assertEquals("English", vc.getSubject().getProperty("language"));
		assertEquals("john@example.com", vc.getSubject().getProperty("email"));
		assertEquals("@john", vc.getSubject().getProperty("twitter"));

		assertNotNull(vc.getIssuanceDate());
		assertNotNull(vc.getExpirationDate());

		assertFalse(vc.isExpired());
		assertTrue(vc.isGenuine());
		assertTrue(vc.isValid());
	}

	@Test
	public void testParseAndSerializeKycCredential()
			throws DIDException, IOException {
		TestData testData = new TestData();

		String json = testData.loadTwitterVcNormalizedJson();
		VerifiableCredential normalized = VerifiableCredential.fromJson(json);

		json = testData.loadTwitterVcCompactJson();
		VerifiableCredential compact = VerifiableCredential.fromJson(json);

		VerifiableCredential vc = testData.loadTwitterCredential();

		assertEquals(testData.loadTwitterVcNormalizedJson(), normalized.toString(true));
		assertEquals(testData.loadTwitterVcNormalizedJson(), compact.toString(true));
		assertEquals(testData.loadTwitterVcNormalizedJson(), vc.toString(true));

		assertEquals(testData.loadTwitterVcCompactJson(), normalized.toString(false));
		assertEquals(testData.loadTwitterVcCompactJson(), compact.toString(false));
		assertEquals(testData.loadTwitterVcCompactJson(), vc.toString(false));
	}

	@Test
	public void testParseAndSerializeSelfProclaimedCredential()
			throws DIDException, IOException {
		TestData testData = new TestData();

		String json = testData.loadProfileVcNormalizedJson();
		VerifiableCredential normalized = VerifiableCredential.fromJson(json);

		json = testData.loadProfileVcCompactJson();
		VerifiableCredential compact = VerifiableCredential.fromJson(json);

		VerifiableCredential vc = testData.loadProfileCredential();

		assertEquals(testData.loadProfileVcNormalizedJson(), normalized.toString(true));
		assertEquals(testData.loadProfileVcNormalizedJson(), compact.toString(true));
		assertEquals(testData.loadProfileVcNormalizedJson(), vc.toString(true));

		assertEquals(testData.loadProfileVcCompactJson(), normalized.toString(false));
		assertEquals(testData.loadProfileVcCompactJson(), compact.toString(false));
		assertEquals(testData.loadProfileVcCompactJson(), vc.toString(false));
	}
}
