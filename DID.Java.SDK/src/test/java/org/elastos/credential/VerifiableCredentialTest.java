package org.elastos.credential;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Arrays;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDException;
import org.elastos.did.DIDURL;
import org.junit.BeforeClass;
import org.junit.Test;

public class VerifiableCredentialTest {
	private static SimpleDateFormat dateFormat;

	@BeforeClass
	public static void setup() {
		dateFormat = new SimpleDateFormat(Constants.DATE_FORMAT);
		dateFormat.setTimeZone(Constants.UTC);
	}

	private VerifiableCredential loadCredential(String file)
			throws MalformedCredentialException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream(file));

		VerifiableCredential vc = VerifiableCredential.fromJson(input);

		try {
			input.close();
		} catch (IOException ignore) {
		}

		return vc;
	}

	@Test
	public void TestKycCredential1() throws DIDException, ParseException {
		VerifiableCredential vc = loadCredential("vc-normalized.json");

		DID issuer = new DID("did:elastos:ip1KJhUo2AoChxnnJcywic6vTSLimqtg1S");

		DID did = new DID("did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
		DIDURL vcId = new DIDURL(did, "credential-1");

		assertEquals(vcId, vc.getId());

		assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("InternetAccountCredential"));

		assertEquals(issuer, vc.getIssuer());
		assertEquals(did, vc.getSubject().getId());

		assertEquals("John", vc.getSubject().getProperty("name"));
		assertEquals("Male", vc.getSubject().getProperty("gender"));
		assertEquals("Singapore", vc.getSubject().getProperty("nation"));
		assertEquals("English", vc.getSubject().getProperty("language"));
		assertEquals("john@example.com", vc.getSubject().getProperty("email"));
		assertEquals("@john", vc.getSubject().getProperty("twitter"));

		assertEquals(dateFormat.parse("2019-12-01T10:00:00Z"), vc.getIssuanceDate());
		assertEquals(dateFormat.parse("2024-12-01T10:00:00Z"), vc.getExpirationDate());

		//assertTrue(vc.verify());
		assertFalse(vc.isExpired());
	}

	@Test
	public void TestKycCredential2() throws DIDException, ParseException {
		VerifiableCredential vc = loadCredential("vc-compact.json");

		DID issuer = new DID("did:elastos:ip1KJhUo2AoChxnnJcywic6vTSLimqtg1S");

		DID did = new DID("did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
		DIDURL vcId = new DIDURL(did, "credential-1");

		assertEquals(vcId, vc.getId());

		assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("InternetAccountCredential"));

		assertEquals(issuer, vc.getIssuer());
		assertEquals(did, vc.getSubject().getId());

		assertEquals("John", vc.getSubject().getProperty("name"));
		assertEquals("Male", vc.getSubject().getProperty("gender"));
		assertEquals("Singapore", vc.getSubject().getProperty("nation"));
		assertEquals("English", vc.getSubject().getProperty("language"));
		assertEquals("john@example.com", vc.getSubject().getProperty("email"));
		assertEquals("@john", vc.getSubject().getProperty("twitter"));

		assertEquals(dateFormat.parse("2019-12-01T10:00:00Z"), vc.getIssuanceDate());
		assertEquals(dateFormat.parse("2024-12-01T10:00:00Z"), vc.getExpirationDate());

		// TODO:
		// assertTrue(vc.verify());
		assertFalse(vc.isExpired());
	}

	@Test
	public void TestSelfClaimedCredential1() throws DIDException, ParseException {
		VerifiableCredential vc = loadCredential("vc-selfclaimed-normalized.json");

		DID did = new DID("did:elastos:ioVnCW6KK9CaHMY2QczsooPQ4XpsYA1BWL");
		DIDURL vcId = new DIDURL(did, "credential-2");

		assertEquals(vcId, vc.getId());

		assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("SelfProclaimedCredential"));

		assertEquals(did, vc.getIssuer());
		assertEquals(did, vc.getSubject().getId());

		assertEquals("Testing Issuer", vc.getSubject().getProperty("name"));
		assertEquals("Singapore", vc.getSubject().getProperty("nation"));
		assertEquals("English", vc.getSubject().getProperty("language"));
		assertEquals("issuer@example.com", vc.getSubject().getProperty("email"));

		assertEquals(dateFormat.parse("2019-12-01T10:00:00Z"), vc.getIssuanceDate());
		assertEquals(dateFormat.parse("2024-12-01T10:00:00Z"), vc.getExpirationDate());

		// TODO:
		// assertTrue(vc.verify());
		assertFalse(vc.isExpired());
	}

	@Test
	public void TestSelfClaimedCredential2() throws DIDException, ParseException {
		VerifiableCredential vc = loadCredential("vc-selfclaimed-compact.json");

		DID did = new DID("did:elastos:ioVnCW6KK9CaHMY2QczsooPQ4XpsYA1BWL");
		DIDURL vcId = new DIDURL(did, "credential-2");

		assertEquals(vcId, vc.getId());

		assertTrue(Arrays.asList(vc.getTypes()).contains("BasicProfileCredential"));
		assertTrue(Arrays.asList(vc.getTypes()).contains("SelfProclaimedCredential"));

		assertEquals(did, vc.getIssuer());
		assertEquals(did, vc.getSubject().getId());

		assertEquals("Testing Issuer", vc.getSubject().getProperty("name"));
		assertEquals("Singapore", vc.getSubject().getProperty("nation"));
		assertEquals("English", vc.getSubject().getProperty("language"));
		assertEquals("issuer@example.com", vc.getSubject().getProperty("email"));

		assertEquals(dateFormat.parse("2019-12-01T10:00:00Z"), vc.getIssuanceDate());
		assertEquals(dateFormat.parse("2024-12-01T10:00:00Z"), vc.getExpirationDate());

		// TODO:
		// assertTrue(vc.verify());
		assertFalse(vc.isExpired());
	}

	@Test
	public void testToJson1() throws DIDException {
		VerifiableCredential vc1 = loadCredential("vc-normalized.json");
		VerifiableCredential vc2 = loadCredential("vc-compact.json");

		// TODO: should compare with original file.
		assertEquals(vc1.toExternalForm(true), vc2.toExternalForm(true));
		assertEquals(vc1.toExternalForm(false), vc2.toExternalForm(false));
	}

	@Test
	public void testToJson2() throws DIDException {
		VerifiableCredential vc1 = loadCredential("vc-selfclaimed-normalized.json");
		VerifiableCredential vc2 = loadCredential("vc-selfclaimed-compact.json");

		// TODO: should compare with original file.
		assertEquals(vc1.toExternalForm(true), vc2.toExternalForm(true));
		assertEquals(vc1.toExternalForm(false), vc2.toExternalForm(false));
	}
}
