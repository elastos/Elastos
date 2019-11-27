package org.elastos.credential;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import org.elastos.did.DID;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDException;
import org.elastos.did.DIDStoreException;
import org.elastos.did.DIDURL;
import org.elastos.did.TestConfig;
import org.elastos.did.TestData;
import org.elastos.did.util.Base58;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class IssuerTest {
	@Rule
	public ExpectedException expectedEx = ExpectedException.none();

	@BeforeClass
	public static void setup() throws DIDStoreException {
		TestData.setup();
	}

	@AfterClass
	public static void cleanup() {
		TestData.cleanup();
	}

	@Test
	public void newIssuerTestWithSignKey() throws DIDException {
		DIDDocument doc = TestData.getPrimaryDid().resolve();
		DIDURL signKey = doc.getDefaultPublicKey();

		Issuer issuer = new Issuer(TestData.getPrimaryDid(), signKey);

		assertEquals(TestData.getPrimaryDid(), issuer.getDid());
		assertEquals(signKey, issuer.getSignKey());
	}

	@Test
	public void newIssuerTestWithoutSignKey() throws DIDException {
		Issuer issuer = new Issuer(TestData.getPrimaryDid());

		DIDDocument doc = TestData.getPrimaryDid().resolve();
		DIDURL signKey = doc.getDefaultPublicKey();

		assertEquals(TestData.getPrimaryDid(), issuer.getDid());
		assertEquals(signKey, issuer.getSignKey());
	}

	@Test
	public void newIssuerTestWithInvalidKey() throws DIDException {
		expectedEx.expect(DIDException.class);
		expectedEx.expectMessage("No private key.");

		DIDDocument doc = TestData.getPrimaryDid().resolve();
		DIDURL signKey = new DIDURL(TestData.getPrimaryDid(), "auth-key");

		doc.modify();
		doc.addAuthenticationKey(signKey, Base58.encode(new byte[33]));

		Issuer issuer = new Issuer(TestData.getPrimaryDid(), signKey);

		// Dead code.
		assertEquals(TestData.getPrimaryDid(), issuer.getDid());
	}

	@Test
	public void newIssuerTestWithInvalidKey2() throws DIDException {
		expectedEx.expect(DIDException.class);
		expectedEx.expectMessage("Invalid sign key id.");

		DIDURL signKey = new DIDURL(TestData.getPrimaryDid(), "notExist");
		Issuer issuer = new Issuer(TestData.getPrimaryDid(), signKey);

		// Dead code.
		assertEquals(TestData.getPrimaryDid(), issuer.getDid());
	}

	@Test
	public void IssueTest() throws DIDException {
		Issuer issuer = new Issuer(TestData.getPrimaryDid());

		Map<String, String> props= new HashMap<String, String>();
		props.put("name", "John");
		props.put("gender", "Male");
		props.put("nation", "Singapore");
		props.put("language", "English");
		props.put("email", "john@example.com");
		props.put("twitter", "@john");

		Issuer.CredentialBuilder cb = issuer.issueFor("did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
		VerifiableCredential vc = cb.id("credential-1")
			.type("BasicProfileCredential", "InternetAccountCredential")
			.properties(props)
			.sign(TestConfig.storePass);

		DID did = new DID("did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
		DIDURL vcId = new DIDURL(did, "credential-1");

		assertEquals(vcId, vc.getId());

		assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("InternetAccountCredential"));
		assertFalse(Arrays.asList(vc.getTypes()).contains("SelfProclaimedCredential"));

		assertEquals(issuer.getDid(), vc.getIssuer());
		assertEquals(did, vc.getSubject().getId());

		assertEquals("John", vc.getSubject().getProperty("name"));
		assertEquals("Male", vc.getSubject().getProperty("gender"));
		assertEquals("Singapore", vc.getSubject().getProperty("nation"));
		assertEquals("English", vc.getSubject().getProperty("language"));
		assertEquals("john@example.com", vc.getSubject().getProperty("email"));
		assertEquals("@john", vc.getSubject().getProperty("twitter"));

		assertTrue(vc.verify());
		assertFalse(vc.isExpired());
	}

	@Test
	public void IssueTest2() throws DIDException {
		// Self claimed
		Issuer issuer = new Issuer(TestData.getPrimaryDid());

		Map<String, String> props= new HashMap<String, String>();
		props.put("name", "Testing Issuer");
		props.put("nation", "Singapore");
		props.put("language", "English");
		props.put("email", "issuer@example.com");

		Issuer.CredentialBuilder cb = issuer.issueFor(issuer.getDid());
		VerifiableCredential vc = cb.id("credential-2")
			.type("BasicProfileCredential", "SelfProclaimedCredential")
			.properties(props)
			.sign(TestConfig.storePass);

		DIDURL vcId = new DIDURL(issuer.getDid(), "credential-2");

		assertEquals(vcId, vc.getId());

		assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("SelfProclaimedCredential"));
		assertFalse(Arrays.asList(vc.getTypes()).contains("InternetAccountCredential"));

		assertEquals(issuer.getDid(), vc.getIssuer());
		assertEquals(issuer.getDid(), vc.getSubject().getId());

		assertEquals("Testing Issuer", vc.getSubject().getProperty("name"));
		assertEquals("Singapore", vc.getSubject().getProperty("nation"));
		assertEquals("English", vc.getSubject().getProperty("language"));
		assertEquals("issuer@example.com", vc.getSubject().getProperty("email"));

		assertTrue(vc.verify());
		assertFalse(vc.isExpired());
	}

}
