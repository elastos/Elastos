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

import java.io.IOException;
import java.util.Collection;
import java.util.Date;
import java.util.Map;
import java.util.Set;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class Claims implements Map<String, Object> {
	/** JWT {@code Issuer} claims parameter name: <code>"iss"</code> */
	public static final String ISSUER = "iss";

	/** JWT {@code Subject} claims parameter name: <code>"sub"</code> */
	public static final String SUBJECT = "sub";

	/** JWT {@code Audience} claims parameter name: <code>"aud"</code> */
	public static final String AUDIENCE = "aud";

	/** JWT {@code Expiration} claims parameter name: <code>"exp"</code> */
	public static final String EXPIRATION = "exp";

	/** JWT {@code Not Before} claims parameter name: <code>"nbf"</code> */
	public static final String NOT_BEFORE = "nbf";

	/** JWT {@code Issued At} claims parameter name: <code>"iat"</code> */
	public static final String ISSUED_AT = "iat";

	/** JWT {@code JWT ID} claims parameter name: <code>"jti"</code> */
	public static final String ID = "jti";

	private io.jsonwebtoken.Claims impl;

	protected Claims(io.jsonwebtoken.Claims impl) {
		this.impl = impl;
	}

	protected io.jsonwebtoken.Claims getImpl() {
		return impl;
	}

	/**
	 * Returns the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.1">
	 * <code>iss</code></a> (issuer) value or {@code null} if not present.
	 *
	 * @return the JWT {@code iss} value or {@code null} if not present.
	 */
	public String getIssuer() {
		return impl.getIssuer();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.1">
	 * <code>iss</code></a> (issuer) value. A {@code null} value will remove the
	 * property from the JSON map.
	 *
	 * @param iss the JWT {@code iss} value or {@code null} to remove the
	 *            property from the JSON map.
	 * @return the {@code Claims} instance for method chaining.
	 */
	public Claims setIssuer(String iss) {
		impl.setIssuer(iss);
		return this;
	}

	/**
	 * Returns the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.2">
	 * <code>sub</code></a> (subject) value or {@code null} if not present.
	 *
	 * @return the JWT {@code sub} value or {@code null} if not present.
	 */
	public String getSubject() {
		return impl.getSubject();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.2">
	 * <code>sub</code></a> (subject) value. A {@code null} value will remove
	 * the property from the JSON map.
	 *
	 * @param sub the JWT {@code sub} value or {@code null} to remove the
	 *            property from the JSON map.
	 * @return the {@code Claims} instance for method chaining.
	 */
	public Claims setSubject(String sub) {
		impl.setSubject(sub);
		return this;
	}

	/**
	 * Returns the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.3">
	 * <code>aud</code></a> (audience) value or {@code null} if not present.
	 *
	 * @return the JWT {@code aud} value or {@code null} if not present.
	 */
	public String getAudience() {
		return impl.getAudience();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.3">
	 * <code>aud</code></a> (audience) value. A {@code null} value will remove
	 * the property from the JSON map.
	 *
	 * @param aud the JWT {@code aud} value or {@code null} to remove the
	 *            property from the JSON map.
	 * @return the {@code Claims} instance for method chaining.
	 */
	public Claims setAudience(String aud) {
		impl.setAudience(aud);
		return this;
	}

	/**
	 * Returns the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.4">
	 * <code>exp</code></a> (expiration) timestamp or {@code null} if not
	 * present.
	 *
	 * <p>
	 * A JWT obtained after this timestamp should not be used.
	 * </p>
	 *
	 * @return the JWT {@code exp} value or {@code null} if not present.
	 */
	public Date getExpiration() {
		return impl.getExpiration();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.4">
	 * <code>exp</code></a> (expiration) timestamp. A {@code null} value will
	 * remove the property from the JSON map.
	 *
	 * <p>
	 * A JWT obtained after this timestamp should not be used.
	 * </p>
	 *
	 * @param exp the JWT {@code exp} value or {@code null} to remove the
	 *            property from the JSON map.
	 * @return the {@code Claims} instance for method chaining.
	 */
	public Claims setExpiration(Date exp) {
		impl.setExpiration(exp);
		return this;
	}

	/**
	 * Returns the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.5">
	 * <code>nbf</code></a> (not before) timestamp or {@code null} if not
	 * present.
	 *
	 * <p>
	 * A JWT obtained before this timestamp should not be used.
	 * </p>
	 *
	 * @return the JWT {@code nbf} value or {@code null} if not present.
	 */
	public Date getNotBefore() {
		return impl.getNotBefore();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.5">
	 * <code>nbf</code></a> (not before) timestamp. A {@code null} value will
	 * remove the property from the JSON map.
	 *
	 * <p>
	 * A JWT obtained before this timestamp should not be used.
	 * </p>
	 *
	 * @param nbf the JWT {@code nbf} value or {@code null} to remove the
	 *            property from the JSON map.
	 * @return the {@code Claims} instance for method chaining.
	 */
	public Claims setNotBefore(Date nbf) {
		impl.setNotBefore(nbf);
		return this;
	}

	/**
	 * Returns the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.6">
	 * <code>iat</code></a> (issued at) timestamp or {@code null} if not
	 * present.
	 *
	 * <p>
	 * If present, this value is the timestamp when the JWT was created.
	 * </p>
	 *
	 * @return the JWT {@code nbf} value or {@code null} if not present.
	 */
	public Date getIssuedAt() {
		return impl.getIssuedAt();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.6">
	 * <code>iat</code></a> (issued at) timestamp. A {@code null} value will
	 * remove the property from the JSON map.
	 *
	 * <p>
	 * The value is the timestamp when the JWT was created.
	 * </p>
	 *
	 * @param iat the JWT {@code iat} value or {@code null} to remove the
	 *            property from the JSON map.
	 * @return the {@code Claims} instance for method chaining.
	 */
	public Claims setIssuedAt(Date iat) {
		impl.setIssuedAt(iat);
		return this;
	}

	/**
	 * Returns the JWTs <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.7">
	 * <code>jti</code></a> (JWT ID) value or {@code null} if not present.
	 *
	 * <p>
	 * This value is a CaSe-SenSiTiVe unique identifier for the JWT. If
	 * available, this value is expected to be assigned in a manner that ensures
	 * that there is a negligible probability that the same value will be
	 * accidentally assigned to a different data object. The ID can be used to
	 * prevent the JWT from being replayed.
	 * </p>
	 *
	 * @return the JWT {@code jti} value or {@code null} if not present.
	 */
	public String getId() {
		return impl.getId();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-4.1.7">
	 * <code>jti</code></a> (JWT ID) value. A {@code null} value will remove the
	 * property from the JSON map.
	 *
	 * <p>
	 * This value is a CaSe-SenSiTiVe unique identifier for the JWT. If
	 * specified, this value MUST be assigned in a manner that ensures that
	 * there is a negligible probability that the same value will be
	 * accidentally assigned to a different data object. The ID can be used to
	 * prevent the JWT from being replayed.
	 * </p>
	 *
	 * @param jti the JWT {@code jti} value or {@code null} to remove the
	 *            property from the JSON map.
	 * @return the {@code Claims} instance for method chaining.
	 */
	public Claims setId(String jti) {
		impl.setId(jti);
		return this;
	}

	/**
	 * Returns the JWTs claim ({@code claimName}) value as a type
	 * {@code requiredType}, or {@code null} if not present.
	 *
	 * @param claimName    name of claim
	 * @param requiredType the type of the value expected to be returned
	 * @param <T>          the type of the value expected to be returned
	 * @return the JWT {@code claimName} value or {@code null} if not present.
	 */
	public <T> T get(String claimName, Class<T> requiredType) {
		if (JsonNode.class.equals(requiredType)) {
			@SuppressWarnings({ "unchecked", "rawtypes" })
			Class<Map<String, Object>> clazz = (Class)Map.class;
			Map<String, Object> map = impl.get(claimName, clazz);
			return requiredType.cast(map2JsonNode(map));
		}

		return impl.get(claimName, requiredType);
	}

	@Override
	public int size() {
		return impl.size();
	}

	@Override
	public boolean isEmpty() {
		return impl.isEmpty();
	}

	@Override
	public boolean containsKey(Object key) {
		return impl.containsKey(key);
	}

	@Override
	public boolean containsValue(Object value) {
		return impl.containsValue(value);
	}

	@Override
	public Object get(Object key) {
		return impl.get(key);
	}

	public String getAsJson(Object key) {
		Object v = impl.get(key);

		if (v instanceof Map) {
			@SuppressWarnings("unchecked")
			JsonNode n = map2JsonNode((Map<String, Object>)v);
			return n.toString();
		} else {
			throw new UnsupportedOperationException();
		}
	}

	@Override
	public Object put(String key, Object value) {
		if (value instanceof JsonNode)
			return impl.put(key, jsonNode2Map((JsonNode)value));

		return impl.put(key, value);
	}

	public Object putWithJson(String key, String json) {
		return impl.put(key, json2Map(json));
	}

	@Override
	public Object remove(Object key) {
		return impl.remove(key);
	}

	@Override
	public void putAll(Map<? extends String, ? extends Object> m) {
		impl.putAll(m);
	}

	public void putAll(JsonNode node) {
		impl.putAll(jsonNode2Map(node));
	}

	public void putAllWithJson(String json) {
		impl.putAll(json2Map(json));
	}

	@Override
	public void clear() {
		impl.clear();
	}

	@Override
	public Set<String> keySet() {
		return impl.keySet();
	}

	@Override
	public Collection<Object> values() {
		return impl.values();
	}

	@Override
	public Set<Entry<String, Object>> entrySet() {
		return impl.entrySet();
	}

	protected static Map<String, Object> jsonNode2Map(JsonNode node) {
		ObjectMapper mapper = new ObjectMapper();
		Map<String, Object> map = mapper.convertValue(node,
				new TypeReference<Map<String, Object>>(){});

		return map;
	}

	protected static Map<String, Object> json2Map(String json) {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode node = mapper.readTree(json);
			Map<String, Object> map = mapper.convertValue(node,
					new TypeReference<Map<String, Object>>(){});

			return map;
		} catch (IOException e) {
			throw new IllegalArgumentException(e);
		}
	}

	protected static JsonNode map2JsonNode(Map<String, Object> map) {
		ObjectMapper mapper = new ObjectMapper();
		return mapper.convertValue(map, JsonNode.class);
	}
}
