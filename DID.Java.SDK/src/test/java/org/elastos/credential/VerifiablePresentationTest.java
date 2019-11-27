package org.elastos.credential;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.HashMap;
import java.util.Map;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDException;
import org.elastos.did.DIDStoreException;
import org.elastos.did.DIDURL;
import org.elastos.did.TestConfig;
import org.elastos.did.TestData;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

public class VerifiablePresentationTest {
	@BeforeClass
	public static void setup() throws DIDStoreException {
		TestData.setup();
	}

	@AfterClass
	public static void cleanup() {
		TestData.cleanup();
	}

	@Test
	public void testBuild() throws DIDException {
		Issuer issuer = new Issuer(TestData.getPrimaryDid());

		Map<String, String> props= new HashMap<String, String>();
		props.put("name", "John");
		props.put("nation", "Singapore");
		props.put("language", "English");
		props.put("email", "john@example.com");

		Issuer.CredentialBuilder cb = issuer.issueFor(issuer.getDid());
		VerifiableCredential vc1 = cb.id("credential-1")
			.type("BasicProfileCredential", "SelfProclaimedCredential")
			.properties(props)
			.sign(TestConfig.storePass);

		props.clear();
		props.put("twitter", "@john");
		props.put("googleAccount", "john@gmail.com");

		cb = issuer.issueFor(issuer.getDid());
		VerifiableCredential vc2 = cb.id("credential-2")
			.type("InternetAccountCredential", "SelfProclaimedCredential")
			.properties(props)
			.sign(TestConfig.storePass);

		VerifiablePresentation.Builder pb = VerifiablePresentation.createFor(TestData.getPrimaryDid());

		VerifiablePresentation vp = pb.credentials(vc1, vc2)
			.sign(TestConfig.storePass, "https://example.com/", "873172f58701a9ee686f0630204fee59");

		assertNotNull(vp);
		assertEquals(Constants.defaultPresentationType, vp.getType());
		assertEquals(2, vp.getCredentials().size());

		DIDURL vcId = new DIDURL(TestData.getPrimaryDid(), "credential-1");
		VerifiableCredential vc = vp.getCredential(vcId);
		assertNotNull(vc);
		assertEquals(vcId, vc.getId());

		vcId = new DIDURL(TestData.getPrimaryDid(), "credential-2");
		vc = vp.getCredential(vcId);
		assertNotNull(vc);
		assertEquals(vcId, vc.getId());

		vcId = new DIDURL(TestData.getPrimaryDid(), "credential-3");
		vc = vp.getCredential(vcId);
		assertNull(vc);

		assertTrue(vp.verify());
	}

	@Test
	public void testLoad() throws DIDException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream("vp.json"));

		VerifiablePresentation vp = VerifiablePresentation.fromJson(input);

		try {
			input.close();
		} catch (IOException ignore) {
		}

		assertNotNull(vp);
		assertEquals(Constants.defaultPresentationType, vp.getType());
		assertEquals(2, vp.getCredentials().size());

		DID did = new DID("did:elastos:iTKkmnqCV4z3jnCBor3XngEcDpabDQ2Q1U");

		DIDURL vcId = new DIDURL(did, "credential-1");
		VerifiableCredential vc = vp.getCredential(vcId);
		assertNotNull(vc);
		assertEquals(vcId, vc.getId());

		vcId = new DIDURL(did, "credential-2");
		vc = vp.getCredential(vcId);
		assertNotNull(vc);
		assertEquals(vcId, vc.getId());

		vcId = new DIDURL(did, "credential-3");
		vc = vp.getCredential(vcId);
		assertNull(vc);
	}
}
