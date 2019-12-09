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

package org.elastos.credential;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.util.List;

import org.elastos.did.Constants;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDException;
import org.elastos.did.DIDURL;
import org.elastos.did.TestConfig;
import org.elastos.did.TestData;
import org.junit.Test;

public class VerifiablePresentationTest {
	@Test
	public void testReadPresentation() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setupStore(true);

		// For integrity check
		testData.loadTestIssuer();
		DIDDocument testDoc = testData.loadTestDocument();

		VerifiablePresentation vp = testData.loadPresentation();

		assertEquals(Constants.defaultPresentationType, vp.getType());
		assertEquals(testDoc.getSubject(), vp.getSigner());

		assertEquals(4, vp.getCredentialCount());
		List<VerifiableCredential> vcs = vp.getCredentials();
		for (VerifiableCredential vc : vcs) {
			assertEquals(testDoc.getSubject(), vc.getSubject().getId());

			assertTrue(vc.getId().getFragment().equals("profile")
					|| vc.getId().getFragment().equals("email")
					|| vc.getId().getFragment().equals("twitter")
					|| vc.getId().getFragment().equals("passport"));
		}

		assertNotNull(vp.getCredential(new DIDURL(vp.getSigner(), "profile")));
		assertNotNull(vp.getCredential(new DIDURL(vp.getSigner(), "email")));
		assertNotNull(vp.getCredential(new DIDURL(vp.getSigner(), "twitter")));
		assertNotNull(vp.getCredential(new DIDURL(vp.getSigner(), "passport")));
		assertNull(vp.getCredential(new DIDURL(vp.getSigner(), "notExist")));

		assertTrue(vp.isGenuine());
		assertTrue(vp.isValid());
	}

	@Test
	public void testBuild() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setupStore(true);

		// For integrity check
		testData.loadTestIssuer();
		DIDDocument testDoc = testData.loadTestDocument();

		VerifiablePresentation.Builder pb = VerifiablePresentation.createFor(
				testDoc.getSubject());

		VerifiablePresentation vp = pb.credentials(testData.loadProfileCredential())
				.credentials(testData.loadEmailCredential())
				.credentials(testData.loadTwitterCredential())
				.credentials(testData.loadPassportCredential())
				.realm("https://example.com/")
				.nonce("873172f58701a9ee686f0630204fee59")
				.seal(TestConfig.storePass);

		assertNotNull(vp);

		assertEquals(Constants.defaultPresentationType, vp.getType());
		assertEquals(testDoc.getSubject(), vp.getSigner());

		assertEquals(4, vp.getCredentialCount());
		List<VerifiableCredential> vcs = vp.getCredentials();
		for (VerifiableCredential vc : vcs) {
			assertEquals(testDoc.getSubject(), vc.getSubject().getId());

			assertTrue(vc.getId().getFragment().equals("profile")
					|| vc.getId().getFragment().equals("email")
					|| vc.getId().getFragment().equals("twitter")
					|| vc.getId().getFragment().equals("passport"));
		}

		assertNotNull(vp.getCredential(new DIDURL(vp.getSigner(), "profile")));
		assertNotNull(vp.getCredential(new DIDURL(vp.getSigner(), "email")));
		assertNotNull(vp.getCredential(new DIDURL(vp.getSigner(), "twitter")));
		assertNotNull(vp.getCredential(new DIDURL(vp.getSigner(), "passport")));
		assertNull(vp.getCredential(new DIDURL(vp.getSigner(), "notExist")));

		assertTrue(vp.isGenuine());
		assertTrue(vp.isValid());
	}

	@Test
	public void testParseAndSerialize() throws DIDException, IOException {
		TestData testData = new TestData();
		testData.setupStore(true);

		// For integrity check
		testData.loadTestIssuer();
		testData.loadTestDocument();

		VerifiablePresentation vp = testData.loadPresentation();
		assertNotNull(vp);
		assertTrue(vp.isGenuine());
		assertTrue(vp.isValid());

		VerifiablePresentation normalized = VerifiablePresentation.fromJson(
				testData.loadPresentationNormalizedJson());
		assertNotNull(normalized);
		assertTrue(normalized.isGenuine());
		assertTrue(normalized.isValid());

		assertEquals(testData.loadPresentationNormalizedJson(),
				normalized.toExternalForm());
		assertEquals(testData.loadPresentationNormalizedJson(),
				vp.toExternalForm());
	}
}
