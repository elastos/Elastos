package org.elastos.did;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.List;

import org.junit.Test;

public class DIDDocumentTest {
	@Test
	public void testParseDocument() throws DIDException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream("testdiddoc.json"));
		DIDDocument doc = DIDDocument.fromJson(input);

		assertEquals(4, doc.getPublicKeyCount());

		List<PublicKey> pks = doc.getPublicKeys();
		for (PublicKey pk : pks) {
			assertTrue(pk.getId().getFragment().equals("default")
					|| pk.getId().getFragment().equals("key2")
					|| pk.getId().getFragment().equals("keys3")
					|| pk.getId().getFragment().equals("recovery"));

			if (pk.getId().getFragment().equals("recovery"))
				assertNotEquals(doc.getSubject(), pk.getController());
			else
				assertEquals(doc.getSubject(), pk.getController());
		}

		assertEquals(3, doc.getAuthenticationKeyCount());
		assertEquals(1, doc.getAuthorizationKeyCount());


		assertEquals(2, doc.getCredentialCount());

		assertEquals(3, doc.getServiceCount());
	}

	@Test
	public void testCompactJson() throws DIDException, IOException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream("testdiddoc.json"));
		DIDDocument doc = DIDDocument.fromJson(input);
		input.close();

		String json = doc.toExternalForm(true);

		File file = new File(getClass().getClassLoader().getResource("compact.json").getFile());
		char[] chars = new char[(int)file.length()];
		input = new InputStreamReader(new FileInputStream(file));
		input.read(chars);
		input.close();

		String expected = new String(chars);

		assertEquals(expected, json);
	}

	@Test
	public void testNormalizedJson() throws DIDException, IOException {
		Reader input = new InputStreamReader(getClass()
				.getClassLoader().getResourceAsStream("testdiddoc.json"));
		DIDDocument doc = DIDDocument.fromJson(input);
		input.close();

		String json = doc.toExternalForm(false);

		File file = new File(getClass().getClassLoader().getResource("normalized.json").getFile());
		char[] chars = new char[(int)file.length()];
		input = new InputStreamReader(new FileInputStream(file));
		input.read(chars);
		input.close();

		String expected = new String(chars);

		assertEquals(expected, json);
	}
}
