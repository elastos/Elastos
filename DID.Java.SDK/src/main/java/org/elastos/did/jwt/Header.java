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

import java.util.Collection;
import java.util.Map;
import java.util.Set;

public class Header implements Map<String, Object> {
	/** JWT {@code Type} (typ) value: <code>"JWT"</code> */
	public static final String JWT_TYPE = "JWT";

	/** JWT {@code Type} header parameter name: <code>"typ"</code> */
	public static final String TYPE = "typ";

	/** JWT {@code Content Type} header parameter name: <code>"cty"</code> */
	public static final String CONTENT_TYPE = "cty";

	/**
	 * JWT {@code Compression Algorithm} header parameter name:
	 * <code>"zip"</code>
	 */
	public static final String COMPRESSION_ALGORITHM = "zip";

	private io.jsonwebtoken.Header<?> impl;

	protected Header(io.jsonwebtoken.Header<?> impl) {
		this.impl = impl;
	}

	protected io.jsonwebtoken.Header<?> getImpl() {
		return impl;
	}

	/**
	 * Returns the <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-5.1">
	 * <code>typ</code></a> (type) header value or {@code null} if not present.
	 *
	 * @return the {@code typ} header value or {@code null} if not present.
	 */
	public String getType() {
		return getImpl().getType();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-5.1">
	 * <code>typ</code></a> (Type) header value. A {@code null} value will
	 * remove the property from the JSON map.
	 *
	 * @param typ the JWT JOSE {@code typ} header value or {@code null} to
	 *            remove the property from the JSON map.
	 * @return the {@code Header} instance for method chaining.
	 */
	public Header setType(String typ) {
		getImpl().setType(typ);
		return this;
	}

	/**
	 * Returns the <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-5.2">
	 * <code>cty</code></a> (Content Type) header value or {@code null} if not
	 * present.
	 *
	 * <p>
	 * In the normal case where nested signing or encryption operations are not
	 * employed (i.e. a compact serialization JWT), the use of this header
	 * parameter is NOT RECOMMENDED. In the case that nested signing or
	 * encryption is employed, this Header Parameter MUST be present; in this
	 * case, the value MUST be {@code JWT}, to indicate that a Nested JWT is
	 * carried in this JWT. While media type names are not case-sensitive, it is
	 * RECOMMENDED that {@code JWT} always be spelled using uppercase characters
	 * for compatibility with legacy implementations. See <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#appendix-A.2">JWT
	 * Appendix A.2</a> for an example of a Nested JWT.
	 * </p>
	 *
	 * @return the {@code typ} header parameter value or {@code null} if not
	 *         present.
	 */
	public String getContentType() {
		return getImpl().getContentType();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#section-5.2">
	 * <code>cty</code></a> (Content Type) header parameter value. A
	 * {@code null} value will remove the property from the JSON map.
	 *
	 * <p>
	 * In the normal case where nested signing or encryption operations are not
	 * employed (i.e. a compact serialization JWT), the use of this header
	 * parameter is NOT RECOMMENDED. In the case that nested signing or
	 * encryption is employed, this Header Parameter MUST be present; in this
	 * case, the value MUST be {@code JWT}, to indicate that a Nested JWT is
	 * carried in this JWT. While media type names are not case-sensitive, it is
	 * RECOMMENDED that {@code JWT} always be spelled using uppercase characters
	 * for compatibility with legacy implementations. See <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25#appendix-A.2">JWT
	 * Appendix A.2</a> for an example of a Nested JWT.
	 * </p>
	 *
	 * @param cty the JWT JOSE {@code cty} header value or {@code null} to
	 *            remove the property from the JSON map.
	 */
	public Header setContentType(String cty) {
		getImpl().setContentType(cty);
		return this;
	}

	/**
	 * Returns the JWT <code>zip</code> (Compression Algorithm) header value or
	 * {@code null} if not present.
	 *
	 * @return the {@code zip} header parameter value or {@code null} if not
	 *         present.
	 * @since 0.6.0
	 */
	public String getCompressionAlgorithm() {
		return getImpl().getCompressionAlgorithm();
	}

	/**
	 * Sets the JWT <code>zip</code> (Compression Algorithm) header parameter
	 * value. A {@code null} value will remove the property from the JSON map.
	 * <p>
	 * <p>
	 * The compression algorithm is NOT part of the <a href=
	 * "https://tools.ietf.org/html/draft-ietf-oauth-json-web-token-25">JWT
	 * specification</a> and must be used carefully since, is not expected that
	 * other libraries (including previous versions of this one) be able to
	 * deserialize a compressed JTW body correctly.
	 * </p>
	 *
	 * @param zip the JWT compression algorithm {@code zip} value or
	 *            {@code null} to remove the property from the JSON map.
	 * @since 0.6.0
	 */
	public Header setCompressionAlgorithm(String zip) {
		getImpl().setCompressionAlgorithm(zip);
		return this;
	}

	@Override
	public int size() {
		return getImpl().size();
	}

	@Override
	public boolean isEmpty() {
		return getImpl().isEmpty();
	}

	@Override
	public boolean containsKey(Object key) {
		return getImpl().containsKey(key);
	}

	@Override
	public boolean containsValue(Object value) {
		return getImpl().containsValue(value);
	}

	@Override
	public Object get(Object key) {
		return getImpl().get(key);
	}

	@Override
	public Object put(String key, Object value) {
		return getImpl().put(key, value);
	}

	@Override
	public Object remove(Object key) {
		return getImpl().remove(key);
	}

	@Override
	public void putAll(Map<? extends String, ? extends Object> m) {
		getImpl().putAll(m);
	}

	@Override
	public void clear() {
		getImpl().clear();
	}

	@Override
	public Set<String> keySet() {
		return getImpl().keySet();
	}

	@Override
	public Collection<Object> values() {
		return getImpl().values();
	}

	@Override
	public Set<Entry<String, Object>> entrySet() {
		return getImpl().entrySet();
	}
}
