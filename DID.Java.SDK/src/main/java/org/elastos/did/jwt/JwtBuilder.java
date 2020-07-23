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

import java.security.PrivateKey;
import java.util.Date;
import java.util.Map;

import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.InvalidKeyException;

import com.fasterxml.jackson.databind.JsonNode;

import io.jsonwebtoken.CompressionCodec;
import io.jsonwebtoken.CompressionCodecs;
import io.jsonwebtoken.Jwts;

public class JwtBuilder {
	private String issuer;
	private KeyProvider keyProvider;
	private io.jsonwebtoken.JwtBuilder impl;

	public JwtBuilder(String issuer, KeyProvider keyProvider) {
		this.issuer = issuer;
		this.keyProvider = keyProvider;
		this.impl = Jwts.builder();
	}

	/**
	 * Returns a new {@link JwsHeader} instance suitable for digitally signed
	 * JWTs (aka 'JWS's).
	 *
	 * @return a new {@link JwsHeader} instance suitable for digitally signed
	 *         JWTs (aka 'JWS's).
	 * @see JwtBuilder#setHeader(Header)
	 */
	public static Header createHeader() {
		return new Header(Jwts.header());
	}

	/**
	 * Returns a new {@link JwsHeader} instance suitable for digitally signed
	 * JWTs (aka 'JWS's), populated with the specified name/value pairs.
	 *
	 * @return a new {@link JwsHeader} instance suitable for digitally signed
	 *         JWTs (aka 'JWS's), populated with the specified name/value pairs.
	 * @see JwtBuilder#setHeader(Header)
	 */
	public static Header createHeader(Map<String, Object> header) {
		return new Header(Jwts.header(header));
	}

	/**
	 * Returns a new {@link JwsHeader} instance suitable for digitally signed
	 * JWTs (aka 'JWS's).
	 *
	 * @return a new {@link JwsHeader} instance suitable for digitally signed
	 *         JWTs (aka 'JWS's).
	 * @see JwtBuilder#setHeader(Header)
	 */
	public static JwsHeader createJwsHeader() {
		return new JwsHeader(Jwts.jwsHeader());
	}

	/**
	 * Returns a new {@link JwsHeader} instance suitable for digitally signed
	 * JWTs (aka 'JWS's), populated with the specified name/value pairs.
	 *
	 * @return a new {@link JwsHeader} instance suitable for digitally signed
	 *         JWTs (aka 'JWS's), populated with the specified name/value pairs.
	 * @see JwtBuilder#setHeader(Header)
	 */
	public static JwsHeader createJwsHeader(Map<String, Object> header) {
		return new JwsHeader(Jwts.jwsHeader(header));
	}

	/**
	 * Sets (and replaces) any existing header with the specified header. If you
	 * do not want to replace the existing header and only want to append to it,
	 * use the {@link #setHeader(java.util.Map)} method instead.
	 *
	 * @param header the header to set (and potentially replace any existing
	 *               header).
	 * @return the builder for method chaining.
	 */
	public JwtBuilder setHeader(Header header) {
		impl.setHeader((Map<String, Object>) header.getImpl());
		return this;
	}

	/**
	 * Sets (and replaces) any existing header with the specified header. If you
	 * do not want to replace the existing header and only want to append to it,
	 * use the {@link #setHeader(java.util.Map)} method instead.
	 *
	 * @param header the header to set (and potentially replace any existing
	 *               header).
	 * @return the builder for method chaining.
	 */
	public JwtBuilder setHeader(Map<String, Object> header) {
		impl.setHeader(header);
		return this;
	}

	/**
	 * Applies the specified name/value pairs to the header. If a header does
	 * not yet exist at the time this method is called, one will be created
	 * automatically before applying the name/value pairs.
	 *
	 * @param params the header name/value pairs to append to the header.
	 * @return the builder for method chaining.
	 */
	public JwtBuilder addHeaders(Map<String, Object> params) {
		impl.setHeaderParams(params);
		return this;
	}

	/**
	 * Applies the specified name/value pair to the header. If a header does not
	 * yet exist at the time this method is called, one will be created
	 * automatically before applying the name/value pair.
	 *
	 * @param name  the header parameter name
	 * @param value the header parameter value
	 * @return the builder for method chaining.
	 */
	public JwtBuilder addHeader(String name, Object value) {
		impl.setHeaderParam(name, value);
		return this;
	}

	/**
	 * Sets the JWT's payload to be a plaintext (non-JSON) string. If you want
	 * the JWT body to be JSON, use the {@link #setClaims(java.util.Map)}
	 * methods instead.
	 *
	 * <p>
	 * The payload and claims properties are mutually exclusive - only one of
	 * the two may be used.
	 * </p>
	 *
	 * @param payload the plaintext (non-JSON) string that will be the body of
	 *                the JWT.
	 * @return the builder for method chaining.
	 */
	public JwtBuilder setPayload(String payload) {
		impl.setPayload(payload);
		return this;
	}

	/**
	 * Returns a new {@link Claims} instance to be used as a JWT body.
	 *
	 * @return a new {@link Claims} instance to be used as a JWT body.
	 */
	public static Claims createClaims() {
		return new Claims(Jwts.claims());
	}

	/**
	 * Returns a new {@link Claims} instance populated with the specified
	 * name/value pairs.
	 *
	 * @param claims the name/value pairs to populate the new Claims instance.
	 * @return a new {@link Claims} instance populated with the specified
	 *         name/value pairs.
	 */
	public static Claims createClaims(Map<String, Object> claims) {
		return new Claims(Jwts.claims(claims));
	}

	/**
	 * Sets the JWT payload to be a JSON Claims instance. If you do not want the
	 * JWT body to be JSON and instead want it to be a plaintext string, use the
	 * {@link #setPayload(String)} method instead.
	 *
	 * <p>
	 * The payload and claims properties are mutually exclusive - only one of
	 * the two may be used.
	 * </p>
	 *
	 * @param claims the JWT claims to be set as the JWT body.
	 * @return the builder for method chaining.
	 */
	public JwtBuilder setClaims(Claims claims) {
		impl.setClaims(claims.getImpl());
		if (!claims.containsKey(Claims.ISSUER))
			impl.setIssuer(issuer);

		return this;
	}

	/**
	 * Sets the JWT payload to be a JSON Claims instance populated by the
	 * specified name/value pairs. If you do not want the JWT body to be JSON
	 * and instead want it to be a plaintext string, use the
	 * {@link #setPayload(String)} method instead.
	 *
	 * <p>
	 * The payload* and claims* properties are mutually exclusive - only one of
	 * the two may be used.
	 * </p>
	 *
	 * @param claims the JWT claims to be set as the JWT body.
	 * @return the builder for method chaining.
	 */
	public JwtBuilder setClaims(Map<String, ?> claims) {
		impl.setClaims(claims);
		if (!claims.containsKey(Claims.ISSUER))
			impl.setIssuer(issuer);

		return this;
	}

	JwtBuilder setClaims(JsonNode claims) {
		return setClaims(Claims.jsonNode2Map(claims));
	}

	JwtBuilder setClaimsWithJson(String jsonClaims) {
		return setClaims(Claims.json2Map(jsonClaims));
	}

	/**
	 * Adds all given name/value pairs to the JSON Claims in the payload. If a
	 * Claims instance does not yet exist at the time this method is called, one
	 * will be created automatically before applying the name/value pairs.
	 *
	 * <p>
	 * The payload and claims properties are mutually exclusive - only one of
	 * the two may be used.
	 * </p>
	 *
	 * @param claims the JWT claims to be added to the JWT body.
	 * @return the builder for method chaining.
	 */
	public JwtBuilder addClaims(Map<String, Object> claims) {
		impl.addClaims(claims);
		return this;
	}

	JwtBuilder addClaims(JsonNode claims) {
		return addClaims(Claims.jsonNode2Map(claims));
	}

	JwtBuilder addClaimsWithJson(String jsonClaims) {
		return addClaims(Claims.json2Map(jsonClaims));
	}

	/**
	 * Sets the JWT Claims <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.1">
	 * <code>iss</code></a> (issuer) value. A {@code null} value will remove the
	 * property from the Claims.
	 *
	 * <p>
	 * This is a convenience method. It will first ensure a Claims instance
	 * exists as the JWT body and then set the Claims issuer field with the
	 * specified value. This allows you to write code like this:
	 * </p>
	 *
	 * <pre>
	 * String jwt = Jwts.builder().setIssuer("Joe").compact();
	 * </pre>
	 *
	 * <p>
	 * instead of this:
	 * </p>
	 *
	 * <pre>
	 * Claims claims = Jwts.claims().setIssuer("Joe");
	 * String jwt = Jwts.builder().setClaims(claims).compact();
	 * </pre>
	 * <p>
	 * if desired.
	 * </p>
	 *
	 * @param iss the JWT {@code iss} value or {@code null} to remove the
	 *            property from the Claims map.
	 * @return the builder instance for method chaining.
	 */
	public JwtBuilder setIssuer(String iss) {
		impl.setIssuer(iss);
		return this;
	}

	/**
	 * Sets the JWT Claims <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.2">
	 * <code>sub</code></a> (subject) value. A {@code null} value will remove
	 * the property from the Claims.
	 *
	 * <p>
	 * This is a convenience method. It will first ensure a Claims instance
	 * exists as the JWT body and then set the Claims subject field with the
	 * specified value. This allows you to write code like this:
	 * </p>
	 *
	 * <pre>
	 * String jwt = Jwts.builder().setSubject("Me").compact();
	 * </pre>
	 *
	 * <p>
	 * instead of this:
	 * </p>
	 *
	 * <pre>
	 * Claims claims = Jwts.claims().setSubject("Me");
	 * String jwt = Jwts.builder().setClaims(claims).compact();
	 * </pre>
	 * <p>
	 * if desired.
	 * </p>
	 *
	 * @param sub the JWT {@code sub} value or {@code null} to remove the
	 *            property from the Claims map.
	 * @return the builder instance for method chaining.
	 */
	public JwtBuilder setSubject(String sub) {
		impl.setSubject(sub);
		return this;
	}

	/**
	 * Sets the JWT Claims <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.3">
	 * <code>aud</code></a> (audience) value. A {@code null} value will remove
	 * the property from the Claims.
	 *
	 * <p>
	 * This is a convenience method. It will first ensure a Claims instance
	 * exists as the JWT body and then set the Claims audience field with the
	 * specified value. This allows you to write code like this:
	 * </p>
	 *
	 * <pre>
	 * String jwt = Jwts.builder().setAudience("You").compact();
	 * </pre>
	 *
	 * <p>
	 * instead of this:
	 * </p>
	 *
	 * <pre>
	 * Claims claims = Jwts.claims().setAudience("You");
	 * String jwt = Jwts.builder().setClaims(claims).compact();
	 * </pre>
	 * <p>
	 * if desired.
	 * </p>
	 *
	 * @param aud the JWT {@code aud} value or {@code null} to remove the
	 *            property from the Claims map.
	 * @return the builder instance for method chaining.
	 */
	public JwtBuilder setAudience(String aud) {
		impl.setAudience(aud);
		return this;
	}

	/**
	 * Sets the JWT Claims <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.4">
	 * <code>exp</code></a> (expiration) value. A {@code null} value will remove
	 * the property from the Claims.
	 *
	 * <p>
	 * A JWT obtained after this timestamp should not be used.
	 * </p>
	 *
	 * <p>
	 * This is a convenience method. It will first ensure a Claims instance
	 * exists as the JWT body and then set the Claims expiration field with the
	 * specified value. This allows you to write code like this:
	 * </p>
	 *
	 * <pre>
	 * String jwt = Jwts.builder()
	 * 		.setExpiration(new Date(System.currentTimeMillis() + 3600000))
	 * 		.compact();
	 * </pre>
	 *
	 * <p>
	 * instead of this:
	 * </p>
	 *
	 * <pre>
	 * Claims claims = Jwts.claims()
	 * 		.setExpiration(new Date(System.currentTimeMillis() + 3600000));
	 * String jwt = Jwts.builder().setClaims(claims).compact();
	 * </pre>
	 * <p>
	 * if desired.
	 * </p>
	 *
	 * @param exp the JWT {@code exp} value or {@code null} to remove the
	 *            property from the Claims map.
	 * @return the builder instance for method chaining.
	 */
	public JwtBuilder setExpiration(Date exp) {
		impl.setExpiration(exp);
		return this;
	}

	/**
	 * Sets the JWT Claims <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.5">
	 * <code>nbf</code></a> (not before) value. A {@code null} value will remove
	 * the property from the Claims.
	 *
	 * <p>
	 * A JWT obtained before this timestamp should not be used.
	 * </p>
	 *
	 * <p>
	 * This is a convenience method. It will first ensure a Claims instance
	 * exists as the JWT body and then set the Claims notBefore field with the
	 * specified value. This allows you to write code like this:
	 * </p>
	 *
	 * <pre>
	 * String jwt = Jwts.builder().setNotBefore(new Date()).compact();
	 * </pre>
	 *
	 * <p>
	 * instead of this:
	 * </p>
	 *
	 * <pre>
	 * Claims claims = Jwts.claims().setNotBefore(new Date());
	 * String jwt = Jwts.builder().setClaims(claims).compact();
	 * </pre>
	 * <p>
	 * if desired.
	 * </p>
	 *
	 * @param nbf the JWT {@code nbf} value or {@code null} to remove the
	 *            property from the Claims map.
	 * @return the builder instance for method chaining.
	 */
	public JwtBuilder setNotBefore(Date nbf) {
		impl.setNotBefore(nbf);
		return this;
	}

	/**
	 * Sets the JWT Claims <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.6">
	 * <code>iat</code></a> (issued at) value. A {@code null} value will remove
	 * the property from the Claims.
	 *
	 * <p>
	 * The value is the timestamp when the JWT was created.
	 * </p>
	 *
	 * <p>
	 * This is a convenience method. It will first ensure a Claims instance
	 * exists as the JWT body and then set the Claims issuedAt field with the
	 * specified value. This allows you to write code like this:
	 * </p>
	 *
	 * <pre>
	 * String jwt = Jwts.builder().setIssuedAt(new Date()).compact();
	 * </pre>
	 *
	 * <p>
	 * instead of this:
	 * </p>
	 *
	 * <pre>
	 * Claims claims = Jwts.claims().setIssuedAt(new Date());
	 * String jwt = Jwts.builder().setClaims(claims).compact();
	 * </pre>
	 * <p>
	 * if desired.
	 * </p>
	 *
	 * @param iat the JWT {@code iat} value or {@code null} to remove the
	 *            property from the Claims map.
	 * @return the builder instance for method chaining.
	 */
	public JwtBuilder setIssuedAt(Date iat) {
		impl.setIssuedAt(iat);
		return this;
	}

	/**
	 * Sets the JWT Claims <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.7">
	 * <code>jti</code></a> (JWT ID) value. A {@code null} value will remove the
	 * property from the Claims.
	 *
	 * <p>
	 * The value is a CaSe-SenSiTiVe unique identifier for the JWT. If
	 * specified, this value MUST be assigned in a manner that ensures that
	 * there is a negligible probability that the same value will be
	 * accidentally assigned to a different data object. The ID can be used to
	 * prevent the JWT from being replayed.
	 * </p>
	 *
	 * <p>
	 * This is a convenience method. It will first ensure a Claims instance
	 * exists as the JWT body and then set the Claims id field with the
	 * specified value. This allows you to write code like this:
	 * </p>
	 *
	 * <pre>
	 * String jwt = Jwts.builder().setId(UUID.randomUUID().toString())
	 * 		.compact();
	 * </pre>
	 *
	 * <p>
	 * instead of this:
	 * </p>
	 *
	 * <pre>
	 * Claims claims = Jwts.claims().setId(UUID.randomUUID().toString());
	 * String jwt = Jwts.builder().setClaims(claims).compact();
	 * </pre>
	 * <p>
	 * if desired.
	 * </p>
	 *
	 * @param jti the JWT {@code jti} (id) value or {@code null} to remove the
	 *            property from the Claims map.
	 * @return the builder instance for method chaining.
	 */
	public JwtBuilder setId(String jti) {
		impl.setId(jti);
		return this;
	}

	/**
	 * Sets a custom JWT Claims parameter value. A {@code null} value will
	 * remove the property from the Claims.
	 *
	 * <p>
	 * This is a convenience method. It will first ensure a Claims instance
	 * exists as the JWT body and then set the named property on the Claims
	 * instance. This allows you to write code like this:
	 * </p>
	 *
	 * <pre>
	 * String jwt = Jwts.builder().claim("aName", "aValue").compact();
	 * </pre>
	 *
	 * <p>
	 * instead of this:
	 * </p>
	 *
	 * <pre>
	 * Claims claims = Jwts.claims().put("aName", "aValue");
	 * String jwt = Jwts.builder().setClaims(claims).compact();
	 * </pre>
	 * <p>
	 * if desired.
	 * </p>
	 *
	 * @param name  the JWT Claims property name
	 * @param value the value to set for the specified Claims property name
	 * @return the builder instance for method chaining.
	 */
	public JwtBuilder claim(String name, Object value) {
		impl.claim(name, value);
		return this;
	}

	JwtBuilder claim(String name, JsonNode value) {
		return claim(name, Claims.jsonNode2Map(value));
	}

	JwtBuilder claimWithJson(String name, String jsonValue) {
		return claim(name, Claims.json2Map(jsonValue));
	}

	/**
	 * Signs the constructed JWT with the specified key using the key's
	 * recommended signature algorithm, producing a JWS.
	 *
	 * @param key       the key id to use for signing
	 * @param password  the password for DID store
	 * @return the builder instance for method chaining.
	 * @throws DIDStoreException
	 * @throws InvalidKeyException
	 */
	public JwtBuilder signWith(String key, String password)
			throws InvalidKeyException, DIDStoreException {
		PrivateKey sk = keyProvider.getPrivateKey(key, password);
		impl.setHeaderParam(JwsHeader.KEY_ID, key);
		impl.signWith(sk);
		return this;
	}

	/**
	 * Signs the constructed JWT with the default key using the key's
	 * recommended signature algorithm, producing a JWS.
	 *
	 * @param password the password for DID store
	 * @return the builder instance for method chaining.
	 * @throws DIDStoreException
	 */
	public JwtBuilder sign(String password) throws DIDStoreException {
		try {
			return signWith(null, password);
		} catch (InvalidKeyException ignore) {
			// Should nerver happen
			return null;
		}
	}

	/**
	 * Compresses the JWT body using the specified {@link CompressionCodec}.
	 *
	 * <p>
	 * If your compact JWTs are large, and you want to reduce their total size
	 * during network transmission, this can be useful. For example, when
	 * embedding JWTs in URLs, some browsers may not support URLs longer than a
	 * certain length. Using compression can help ensure the compact JWT fits
	 * within that length. However, NOTE:
	 * </p>
	 *
	 * <h3>Compatibility Warning</h3>
	 *
	 * <p>
	 * The JWT family of specifications defines compression only for JWE (Json
	 * Web Encryption) tokens. Even so, JJWT will also support compression for
	 * JWS tokens as well if you choose to use it. However, be aware that <b>if
	 * you use compression when creating a JWS token, other libraries may not be
	 * able to parse that JWS token</b>. When using compression for JWS tokens,
	 * be sure that that all parties accessing the JWS token support compression
	 * for JWS.
	 * </p>
	 *
	 * <p>
	 * Compression when creating JWE tokens however should be universally
	 * accepted for any library that supports JWE.
	 * </p>
	 *
	 * @param codec compress codec name, could be "deflate" or "gzip".
	 * @return the builder for method chaining.
	 */
	public JwtBuilder compressWith(String codec) {
		CompressionCodec cc = null;

		if (codec == null)
			codec = "deflate";

		if (codec.equalsIgnoreCase("deflate"))
			cc = CompressionCodecs.DEFLATE;
		else if (codec.equalsIgnoreCase("gzip"))
			cc = CompressionCodecs.GZIP;
		else
			throw new IllegalArgumentException(
					"Unsupported compression codec.");

		impl.compressWith(cc);
		return this;
	}

	/**
	 * Actually builds the JWT and serializes it to a compact, URL-safe string
	 * according to the <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-7">JWT
	 * Compact Serialization</a> rules.
	 *
	 * @return A compact URL-safe JWT string.
	 */
	public String compact() {
		return impl.compact();
	}
}
