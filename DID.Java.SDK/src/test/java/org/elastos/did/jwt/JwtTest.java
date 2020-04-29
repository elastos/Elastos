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

package org.elastos.did.jwt;

import static org.junit.Assert.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.io.IOException;
import java.util.Calendar;
import java.util.Date;
import java.util.Map;

import org.elastos.did.DIDDocument;
import org.elastos.did.TestConfig;
import org.elastos.did.TestData;
import org.elastos.did.crypto.Base64;
import org.elastos.did.exception.DIDException;
import org.junit.jupiter.api.Test;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class JwtTest {
	private static final int OPT = Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP;

	public static void printJwt(String token) {
		String [] toks = token.split("\\.");

		if (toks.length != 2 && toks.length != 3) {
			System.out.println("Invalid token: " + token);
			return;
		}

		StringBuilder sb = new StringBuilder(512);
		sb.append(new String(Base64.decode(toks[0], OPT))).append(".")
			.append(new String(Base64.decode(toks[1], OPT))).append(".");
		if (toks.length == 3)
			sb.append(toks[2]);

		System.out.println(sb.toString());
	}

	@Test
	public void jwtTest()
			throws DIDException, IOException, JwtException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		Header h = JwtBuilder.createHeader();
		h.setType(Header.JWT_TYPE)
			.setContentType("json");
		h.put("library", "Elastos DID");
		h.put("version", "1.0");

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.MILLISECOND, 0);
		Date iat = cal.getTime();
		cal.add(Calendar.MONTH, -1);
		Date nbf = cal.getTime();
		cal.add(Calendar.MONTH, 4);
		Date exp = cal.getTime();

		Claims b = JwtBuilder.createClaims();
		b.setSubject("JwtTest")
			.setId("0")
			.setAudience("Test cases")
			.setIssuedAt(iat)
			.setExpiration(exp)
			.setNotBefore(nbf)
			.put("foo", "bar");

		String token = doc.jwtBuilder()
				.setHeader(h)
				.setClaims(b)
				.compact();

		assertNotNull(token);
		printJwt(token);

		JwtParser jp = doc.jwtParserBuilder().build();
		Jwt<Claims> jwt = jp.parseClaimsJwt(token);
		assertNotNull(jwt);

		h = jwt.getHeader();
		assertNotNull(h);
		assertEquals("json", h.getContentType());
		assertEquals(Header.JWT_TYPE, h.getType());
		assertEquals("Elastos DID", h.get("library"));
		assertEquals("1.0", h.get("version"));

		Claims c = jwt.getBody();
		assertNotNull(c);
		assertEquals("JwtTest", c.getSubject());
		assertEquals("0", c.getId());
		assertEquals(doc.getSubject().toString(), c.getIssuer());
		assertEquals("Test cases", c.getAudience());
		assertEquals(iat, c.getIssuedAt());
		assertEquals(exp, c.getExpiration());
		assertEquals(nbf, c.getNotBefore());
		assertEquals("bar", c.get("foo", String.class));
	}

	@Test
	public void jwsTestSignWithDefaultKey()
			throws DIDException, IOException, JwtException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		JwsHeader h = JwtBuilder.createJwsHeader();
		h.setType(Header.JWT_TYPE)
			.setContentType("json");
		h.put("library", "Elastos DID");
		h.put("version", "1.0");

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.MILLISECOND, 0);
		Date iat = cal.getTime();
		cal.add(Calendar.MONTH, -1);
		Date nbf = cal.getTime();
		cal.add(Calendar.MONTH, 4);
		Date exp = cal.getTime();

		Claims b = JwtBuilder.createClaims();
		b.setSubject("JwtTest")
			.setId("0")
			.setAudience("Test cases")
			.setIssuedAt(iat)
			.setExpiration(exp)
			.setNotBefore(nbf)
			.put("foo", "bar");

		String token = doc.jwtBuilder()
				.setHeader(h)
				.setClaims(b)
				.sign(TestConfig.storePass)
				.compact();

		assertNotNull(token);
		printJwt(token);

		JwtParser jp = doc.jwtParserBuilder().build();
		Jws<Claims> jwt = jp.parseClaimsJws(token);
		assertNotNull(jwt);

		h = jwt.getHeader();
		assertNotNull(h);
		assertEquals("json", h.getContentType());
		assertEquals(Header.JWT_TYPE, h.getType());
		assertEquals("Elastos DID", h.get("library"));
		assertEquals("1.0", h.get("version"));

		Claims c = jwt.getBody();
		assertNotNull(c);
		assertEquals("JwtTest", c.getSubject());
		assertEquals("0", c.getId());
		assertEquals(doc.getSubject().toString(), c.getIssuer());
		assertEquals("Test cases", c.getAudience());
		assertEquals(iat, c.getIssuedAt());
		assertEquals(exp, c.getExpiration());
		assertEquals(nbf, c.getNotBefore());
		assertEquals("bar", c.get("foo", String.class));

		String s = jwt.getSignature();
		assertNotNull(s);
	}

	@Test
	public void jwsTestSignWithSpecificKey()
			throws DIDException, IOException, JwtException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.MILLISECOND, 0);
		Date iat = cal.getTime();
		cal.add(Calendar.MONTH, -1);
		Date nbf = cal.getTime();
		cal.add(Calendar.MONTH, 4);
		Date exp = cal.getTime();

		String token = doc.jwtBuilder()
				.addHeader(Header.TYPE, Header.JWT_TYPE)
				.addHeader(Header.CONTENT_TYPE, "json")
				.addHeader("library", "Elastos DID")
				.addHeader("version", "1.0")
				.setSubject("JwtTest")
				.setId("0")
				.setAudience("Test cases")
				.setIssuedAt(iat)
				.setExpiration(exp)
				.setNotBefore(nbf)
				.claim("foo", "bar")
				.signWith("#key2", TestConfig.storePass)
				.compact();

		assertNotNull(token);
		printJwt(token);

		JwtParser jp = doc.jwtParserBuilder().build();
		Jws<Claims> jwt = jp.parseClaimsJws(token);
		assertNotNull(jwt);

		JwsHeader h = jwt.getHeader();
		assertNotNull(h);
		assertEquals("json", h.getContentType());
		assertEquals(Header.JWT_TYPE, h.getType());
		assertEquals("Elastos DID", h.get("library"));
		assertEquals("1.0", h.get("version"));

		Claims c = jwt.getBody();
		assertNotNull(c);
		assertEquals("JwtTest", c.getSubject());
		assertEquals("0", c.getId());
		assertEquals(doc.getSubject().toString(), c.getIssuer());
		assertEquals("Test cases", c.getAudience());
		assertEquals(iat, c.getIssuedAt());
		assertEquals(exp, c.getExpiration());
		assertEquals(nbf, c.getNotBefore());
		assertEquals("bar", c.get("foo", String.class));

		String s = jwt.getSignature();
		assertNotNull(s);
	}

	@Test
	public void jwsTestAutoVerify()
			throws DIDException, IOException, JwtException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.MILLISECOND, 0);
		Date iat = cal.getTime();
		cal.add(Calendar.MONTH, -1);
		Date nbf = cal.getTime();
		cal.add(Calendar.MONTH, 4);
		Date exp = cal.getTime();

		String token = doc.jwtBuilder()
				.addHeader(Header.TYPE, Header.JWT_TYPE)
				.addHeader(Header.CONTENT_TYPE, "json")
				.addHeader("library", "Elastos DID")
				.addHeader("version", "1.0")
				.setSubject("JwtTest")
				.setId("0")
				.setAudience("Test cases")
				.setIssuedAt(iat)
				.setExpiration(exp)
				.setNotBefore(nbf)
				.claim("foo", "bar")
				.signWith("#key2", TestConfig.storePass)
				.compact();

		assertNotNull(token);
		printJwt(token);

		// The JWT parser not related with a DID document
		JwtParser jp = new JwtParserBuilder().build();
		Jws<Claims> jwt = jp.parseClaimsJws(token);
		assertNotNull(jwt);

		JwsHeader h = jwt.getHeader();
		assertNotNull(h);
		assertEquals("json", h.getContentType());
		assertEquals(Header.JWT_TYPE, h.getType());
		assertEquals("Elastos DID", h.get("library"));
		assertEquals("1.0", h.get("version"));

		Claims c = jwt.getBody();
		assertNotNull(c);
		assertEquals("JwtTest", c.getSubject());
		assertEquals("0", c.getId());
		assertEquals(doc.getSubject().toString(), c.getIssuer());
		assertEquals("Test cases", c.getAudience());
		assertEquals(iat, c.getIssuedAt());
		assertEquals(exp, c.getExpiration());
		assertEquals(nbf, c.getNotBefore());
		assertEquals("bar", c.get("foo", String.class));

		String s = jwt.getSignature();
		assertNotNull(s);
	}

	@Test
	public void jwsTestClaimJsonNode()
			throws DIDException, IOException, JwtException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.MILLISECOND, 0);
		Date iat = cal.getTime();
		cal.add(Calendar.MONTH, -1);
		Date nbf = cal.getTime();
		cal.add(Calendar.MONTH, 4);
		Date exp = cal.getTime();

		JsonNode node = loadJson(testData.loadEmailVcNormalizedJson());

		String token = doc.jwtBuilder()
				.addHeader(Header.TYPE, Header.JWT_TYPE)
				.addHeader(Header.CONTENT_TYPE, "json")
				.addHeader("library", "Elastos DID")
				.addHeader("version", "1.0")
				.setSubject("JwtTest")
				.setId("0")
				.setAudience("Test cases")
				.setIssuedAt(iat)
				.setExpiration(exp)
				.setNotBefore(nbf)
				.claim("foo", "bar")
				.claim("vc", node)
				.signWith("#key2", TestConfig.storePass)
				.compact();

		assertNotNull(token);
		printJwt(token);

		// The JWT parser not related with a DID document
		JwtParser jp = new JwtParserBuilder().build();
		Jws<Claims> jwt = jp.parseClaimsJws(token);
		assertNotNull(jwt);

		JwsHeader h = jwt.getHeader();
		assertNotNull(h);
		assertEquals("json", h.getContentType());
		assertEquals(Header.JWT_TYPE, h.getType());
		assertEquals("Elastos DID", h.get("library"));
		assertEquals("1.0", h.get("version"));

		Claims c = jwt.getBody();
		assertNotNull(c);
		assertEquals("JwtTest", c.getSubject());
		assertEquals("0", c.getId());
		assertEquals(doc.getSubject().toString(), c.getIssuer());
		assertEquals("Test cases", c.getAudience());
		assertEquals(iat, c.getIssuedAt());
		assertEquals(exp, c.getExpiration());
		assertEquals(nbf, c.getNotBefore());
		assertEquals("bar", c.get("foo", String.class));

		// get as map
		@SuppressWarnings({ "rawtypes", "unchecked" })
		Class<Map<String, Object>> clazz = (Class)Map.class;
		Map<String, Object> map = c.get("vc", clazz);
		assertNotNull(map);
		assertEquals(testData.loadEmailCredential().getId().toString(), map.get("id"));
		assertEquals(node, map2JsonNode(map));

		// get as JsonNode
		JsonNode n = c.get("vc", JsonNode.class);
		assertNotNull(n);
		assertEquals(testData.loadEmailCredential().getId().toString(), n.get("id").asText());
		assertEquals(node, n);

		// get as json text
		String json = c.getAsJson("vc");
		assertNotNull(json);
		assertEquals(node, loadJson(json));

		String s = jwt.getSignature();
		assertNotNull(s);
	}

	@Test
	public void jwsTestClaimJsonText()
			throws DIDException, IOException, JwtException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.MILLISECOND, 0);
		Date iat = cal.getTime();
		cal.add(Calendar.MONTH, -1);
		Date nbf = cal.getTime();
		cal.add(Calendar.MONTH, 4);
		Date exp = cal.getTime();

		String jsonValue = testData.loadEmailVcNormalizedJson();

		String token = doc.jwtBuilder()
				.addHeader(Header.TYPE, Header.JWT_TYPE)
				.addHeader(Header.CONTENT_TYPE, "json")
				.addHeader("library", "Elastos DID")
				.addHeader("version", "1.0")
				.setSubject("JwtTest")
				.setId("0")
				.setAudience("Test cases")
				.setIssuedAt(iat)
				.setExpiration(exp)
				.setNotBefore(nbf)
				.claim("foo", "bar")
				.claimWithJson("vc", jsonValue)
				.signWith("#key2", TestConfig.storePass)
				.compact();

		assertNotNull(token);
		printJwt(token);

		// The JWT parser not related with a DID document
		JwtParser jp = new JwtParserBuilder().build();
		Jws<Claims> jwt = jp.parseClaimsJws(token);
		assertNotNull(jwt);

		JwsHeader h = jwt.getHeader();
		assertNotNull(h);
		assertEquals("json", h.getContentType());
		assertEquals(Header.JWT_TYPE, h.getType());
		assertEquals("Elastos DID", h.get("library"));
		assertEquals("1.0", h.get("version"));

		Claims c = jwt.getBody();
		assertNotNull(c);
		assertEquals("JwtTest", c.getSubject());
		assertEquals("0", c.getId());
		assertEquals(doc.getSubject().toString(), c.getIssuer());
		assertEquals("Test cases", c.getAudience());
		assertEquals(iat, c.getIssuedAt());
		assertEquals(exp, c.getExpiration());
		assertEquals(nbf, c.getNotBefore());
		assertEquals("bar", c.get("foo", String.class));

		JsonNode node = loadJson(jsonValue);

		// get as map
		@SuppressWarnings({ "rawtypes", "unchecked" })
		Class<Map<String, Object>> clazz = (Class)Map.class;
		Map<String, Object> map = c.get("vc", clazz);
		assertNotNull(map);
		assertEquals(testData.loadEmailCredential().getId().toString(), map.get("id"));
		assertEquals(node, map2JsonNode(map));

		// get as JsonNode
		JsonNode n = c.get("vc", JsonNode.class);
		assertNotNull(n);
		assertEquals(testData.loadEmailCredential().getId().toString(), n.get("id").asText());
		assertEquals(node, n);

		// get as json text
		String json = c.getAsJson("vc");
		assertNotNull(json);
		assertEquals(node, loadJson(json));

		String s = jwt.getSignature();
		assertNotNull(s);
	}

	@Test
	public void jwsTestSetClaimWithJsonNode()
			throws DIDException, IOException, JwtException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.MILLISECOND, 0);
		Date iat = cal.getTime();
		cal.add(Calendar.MONTH, -1);
		Date nbf = cal.getTime();
		cal.add(Calendar.MONTH, 4);
		Date exp = cal.getTime();

		String json = "{\n" +
				"  \"sub\":\"JwtTest\",\n" +
				"  \"jti\":\"0\",\n" +
				"  \"aud\":\"Test cases\",\n" +
				"  \"foo\":\"bar\",\n" +
				"  \"object\":{\n" +
				"    \"hello\":\"world\",\n" +
				"    \"test\":true\n" +
				"  }\n" +
				"}";

		JsonNode node = loadJson(json);

		String token = doc.jwtBuilder()
				.addHeader(Header.TYPE, Header.JWT_TYPE)
				.addHeader(Header.CONTENT_TYPE, "json")
				.addHeader("library", "Elastos DID")
				.addHeader("version", "1.0")
				.setClaims(node)
				.setIssuedAt(iat)
				.setExpiration(exp)
				.setNotBefore(nbf)
				.signWith("#key2", TestConfig.storePass)
				.compact();

		assertNotNull(token);
		printJwt(token);

		// The JWT parser not related with a DID document
		JwtParser jp = new JwtParserBuilder().build();
		Jws<Claims> jwt = jp.parseClaimsJws(token);
		assertNotNull(jwt);

		JwsHeader h = jwt.getHeader();
		assertNotNull(h);
		assertEquals("json", h.getContentType());
		assertEquals(Header.JWT_TYPE, h.getType());
		assertEquals("Elastos DID", h.get("library"));
		assertEquals("1.0", h.get("version"));

		Claims c = jwt.getBody();
		assertNotNull(c);
		assertEquals("JwtTest", c.getSubject());
		assertEquals("0", c.getId());
		assertEquals(doc.getSubject().toString(), c.getIssuer());
		assertEquals("Test cases", c.getAudience());
		assertEquals(iat, c.getIssuedAt());
		assertEquals(exp, c.getExpiration());
		assertEquals(nbf, c.getNotBefore());
		assertEquals("bar", c.get("foo", String.class));

		// get as map
		@SuppressWarnings({ "rawtypes", "unchecked" })
		Class<Map<String, Object>> clazz = (Class)Map.class;
		Map<String, Object> map = c.get("object", clazz);
		assertNotNull(map);

		// get as JsonNode
		JsonNode n = c.get("object", JsonNode.class);
		assertNotNull(n);

		// get as json text
		String v = c.getAsJson("object");
		assertNotNull(v);
		System.out.println(v);

		String s = jwt.getSignature();
		assertNotNull(s);
	}

	@Test
	public void jwsTestSetClaimWithJsonText()
			throws DIDException, IOException, JwtException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.MILLISECOND, 0);
		Date iat = cal.getTime();
		cal.add(Calendar.MONTH, -1);
		Date nbf = cal.getTime();
		cal.add(Calendar.MONTH, 4);
		Date exp = cal.getTime();

		String json = "{\n" +
				"  \"sub\":\"JwtTest\",\n" +
				"  \"jti\":\"0\",\n" +
				"  \"aud\":\"Test cases\",\n" +
				"  \"foo\":\"bar\",\n" +
				"  \"object\":{\n" +
				"    \"hello\":\"world\",\n" +
				"    \"test\":true\n" +
				"  }\n" +
				"}";

		String token = doc.jwtBuilder()
				.addHeader(Header.TYPE, Header.JWT_TYPE)
				.addHeader(Header.CONTENT_TYPE, "json")
				.addHeader("library", "Elastos DID")
				.addHeader("version", "1.0")
				.setClaimsWithJson(json)
				.setIssuedAt(iat)
				.setExpiration(exp)
				.setNotBefore(nbf)
				.signWith("#key2", TestConfig.storePass)
				.compact();

		assertNotNull(token);
		printJwt(token);

		// The JWT parser not related with a DID document
		JwtParser jp = new JwtParserBuilder().build();
		Jws<Claims> jwt = jp.parseClaimsJws(token);
		assertNotNull(jwt);

		JwsHeader h = jwt.getHeader();
		assertNotNull(h);
		assertEquals("json", h.getContentType());
		assertEquals(Header.JWT_TYPE, h.getType());
		assertEquals("Elastos DID", h.get("library"));
		assertEquals("1.0", h.get("version"));

		Claims c = jwt.getBody();
		assertNotNull(c);
		assertEquals("JwtTest", c.getSubject());
		assertEquals("0", c.getId());
		assertEquals(doc.getSubject().toString(), c.getIssuer());
		assertEquals("Test cases", c.getAudience());
		assertEquals(iat, c.getIssuedAt());
		assertEquals(exp, c.getExpiration());
		assertEquals(nbf, c.getNotBefore());
		assertEquals("bar", c.get("foo", String.class));

		// get as map
		@SuppressWarnings({ "rawtypes", "unchecked" })
		Class<Map<String, Object>> clazz = (Class)Map.class;
		Map<String, Object> map = c.get("object", clazz);
		assertNotNull(map);

		// get as JsonNode
		JsonNode n = c.get("object", JsonNode.class);
		assertNotNull(n);

		// get as json text
		String v = c.getAsJson("object");
		assertNotNull(v);
		System.out.println(v);

		String s = jwt.getSignature();
		assertNotNull(s);
	}

	@Test
	public void jwsTestAddClaimWithJsonNode()
			throws DIDException, IOException, JwtException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.MILLISECOND, 0);
		Date iat = cal.getTime();
		cal.add(Calendar.MONTH, -1);
		Date nbf = cal.getTime();
		cal.add(Calendar.MONTH, 4);
		Date exp = cal.getTime();

		String json = "{\n" +
				"  \"sub\":\"JwtTest\",\n" +
				"  \"jti\":\"0\",\n" +
				"  \"aud\":\"Test cases\",\n" +
				"  \"foo\":\"bar\",\n" +
				"  \"object\":{\n" +
				"    \"hello\":\"world\",\n" +
				"    \"test\":true\n" +
				"  }\n" +
				"}";

		JsonNode node = loadJson(json);

		String token = doc.jwtBuilder()
				.addHeader(Header.TYPE, Header.JWT_TYPE)
				.addHeader(Header.CONTENT_TYPE, "json")
				.addHeader("library", "Elastos DID")
				.addHeader("version", "1.0")
				.setIssuedAt(iat)
				.setExpiration(exp)
				.setNotBefore(nbf)
				.addClaims(node)
				.signWith("#key2", TestConfig.storePass)
				.compact();

		assertNotNull(token);
		printJwt(token);

		// The JWT parser not related with a DID document
		JwtParser jp = new JwtParserBuilder().build();
		Jws<Claims> jwt = jp.parseClaimsJws(token);
		assertNotNull(jwt);

		JwsHeader h = jwt.getHeader();
		assertNotNull(h);
		assertEquals("json", h.getContentType());
		assertEquals(Header.JWT_TYPE, h.getType());
		assertEquals("Elastos DID", h.get("library"));
		assertEquals("1.0", h.get("version"));

		Claims c = jwt.getBody();
		assertNotNull(c);
		assertEquals("JwtTest", c.getSubject());
		assertEquals("0", c.getId());
		assertEquals(doc.getSubject().toString(), c.getIssuer());
		assertEquals("Test cases", c.getAudience());
		assertEquals(iat, c.getIssuedAt());
		assertEquals(exp, c.getExpiration());
		assertEquals(nbf, c.getNotBefore());
		assertEquals("bar", c.get("foo", String.class));

		// get as map
		@SuppressWarnings({ "rawtypes", "unchecked" })
		Class<Map<String, Object>> clazz = (Class)Map.class;
		Map<String, Object> map = c.get("object", clazz);
		assertNotNull(map);

		// get as JsonNode
		JsonNode n = c.get("object", JsonNode.class);
		assertNotNull(n);

		// get as json text
		String v = c.getAsJson("object");
		assertNotNull(v);
		System.out.println(v);

		String s = jwt.getSignature();
		assertNotNull(s);
	}

	@Test
	public void jwsTestAddClaimWithJsonText()
			throws DIDException, IOException, JwtException {
		TestData testData = new TestData();
		testData.setup(true);
		testData.initIdentity();

		DIDDocument doc = testData.loadTestDocument();
		assertNotNull(doc);
		assertTrue(doc.isValid());

		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.MILLISECOND, 0);
		Date iat = cal.getTime();
		cal.add(Calendar.MONTH, -1);
		Date nbf = cal.getTime();
		cal.add(Calendar.MONTH, 4);
		Date exp = cal.getTime();

		String json = "{\n" +
				"  \"sub\":\"JwtTest\",\n" +
				"  \"jti\":\"0\",\n" +
				"  \"aud\":\"Test cases\",\n" +
				"  \"foo\":\"bar\",\n" +
				"  \"object\":{\n" +
				"    \"hello\":\"world\",\n" +
				"    \"test\":true\n" +
				"  }\n" +
				"}";

		String token = doc.jwtBuilder()
				.addHeader(Header.TYPE, Header.JWT_TYPE)
				.addHeader(Header.CONTENT_TYPE, "json")
				.addHeader("library", "Elastos DID")
				.addHeader("version", "1.0")
				.setIssuedAt(iat)
				.setExpiration(exp)
				.setNotBefore(nbf)
				.addClaimsWithJson(json)
				.signWith("#key2", TestConfig.storePass)
				.compact();

		assertNotNull(token);
		printJwt(token);

		// The JWT parser not related with a DID document
		JwtParser jp = new JwtParserBuilder().build();
		Jws<Claims> jwt = jp.parseClaimsJws(token);
		assertNotNull(jwt);

		JwsHeader h = jwt.getHeader();
		assertNotNull(h);
		assertEquals("json", h.getContentType());
		assertEquals(Header.JWT_TYPE, h.getType());
		assertEquals("Elastos DID", h.get("library"));
		assertEquals("1.0", h.get("version"));

		Claims c = jwt.getBody();
		assertNotNull(c);
		assertEquals("JwtTest", c.getSubject());
		assertEquals("0", c.getId());
		assertEquals(doc.getSubject().toString(), c.getIssuer());
		assertEquals("Test cases", c.getAudience());
		assertEquals(iat, c.getIssuedAt());
		assertEquals(exp, c.getExpiration());
		assertEquals(nbf, c.getNotBefore());
		assertEquals("bar", c.get("foo", String.class));

		// get as map
		@SuppressWarnings({ "rawtypes", "unchecked" })
		Class<Map<String, Object>> clazz = (Class)Map.class;
		Map<String, Object> map = c.get("object", clazz);
		assertNotNull(map);

		// get as JsonNode
		JsonNode n = c.get("object", JsonNode.class);
		assertNotNull(n);

		// get as json text
		String v = c.getAsJson("object");
		assertNotNull(v);
		System.out.println(v);

		String s = jwt.getSignature();
		assertNotNull(s);
	}

	private JsonNode loadJson(String json) {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(json);
			return node;
		} catch (IOException e) {
			throw new IllegalArgumentException(e);
		}
	}

	protected JsonNode map2JsonNode(Map<String, Object> map) {
		ObjectMapper mapper = new ObjectMapper();
		return mapper.convertValue(map, JsonNode.class);
	}
}
