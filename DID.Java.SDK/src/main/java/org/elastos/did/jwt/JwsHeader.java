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

public class JwsHeader extends Header {
	/** JWS {@code Algorithm} header parameter name: <code>"alg"</code> */
	public static final String ALGORITHM = "alg";

	/** JWS {@code JWT Set URL} header parameter name: <code>"jku"</code> */
	public static final String JWK_SET_URL = "jku";

	/** JWS {@code JSON Web Key} header parameter name: <code>"jwk"</code> */
	public static final String JSON_WEB_KEY = "jwk";

	/** JWS {@code Key ID} header parameter name: <code>"kid"</code> */
	public static final String KEY_ID = "kid";

	/** JWS {@code X.509 URL} header parameter name: <code>"x5u"</code> */
	public static final String X509_URL = "x5u";

	/**
	 * JWS {@code X.509 Certificate Chain} header parameter name:
	 * <code>"x5c"</code>
	 */
	public static final String X509_CERT_CHAIN = "x5c";

	/**
	 * JWS {@code X.509 Certificate SHA-1 Thumbprint} header parameter name:
	 * <code>"x5t"</code>
	 */
	public static final String X509_CERT_SHA1_THUMBPRINT = "x5t";

	/**
	 * JWS {@code X.509 Certificate SHA-256 Thumbprint} header parameter name:
	 * <code>"x5t#S256"</code>
	 */
	public static final String X509_CERT_SHA256_THUMBPRINT = "x5t#S256";

	/** JWS {@code Critical} header parameter name: <code>"crit"</code> */
	public static final String CRITICAL = "crit";

	protected JwsHeader(io.jsonwebtoken.JwsHeader<?> impl) {
		super(impl);
	}

	protected io.jsonwebtoken.JwsHeader<?> getImplAsJwsHeader() {
		return (io.jsonwebtoken.JwsHeader<?>) getImpl();
	}

	@Override
	public JwsHeader setType(String typ) {
		super.setType(typ);
		return this;
	}

	@Override
	public JwsHeader setContentType(String cty) {
		super.setContentType(cty);
		return this;
	}

	@Override
	public JwsHeader setCompressionAlgorithm(String zip) {
		super.setCompressionAlgorithm(zip);
		return this;
	}

	/**
	 * Returns the JWS <a href=
	 * "https://tools.ietf.org/html/draft-ietf-jose-json-web-signature-31#section-4.1.1">
	 * <code>alg</code></a> (algorithm) header value or {@code null} if not
	 * present.
	 *
	 * <p>
	 * The algorithm header parameter identifies the cryptographic algorithm
	 * used to secure the JWS. Consider using
	 * {@link io.jsonwebtoken.SignatureAlgorithm#forName(String)
	 * SignatureAlgorithm.forName} to convert this string value to a type-safe
	 * enum instance.
	 * </p>
	 *
	 * @return the JWS {@code alg} header value or {@code null} if not present.
	 *         This will always be {@code non-null} on validly constructed JWS
	 *         instances, but could be {@code null} during construction.
	 */
	public String getAlgorithm() {
		return getImplAsJwsHeader().getAlgorithm();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-jose-json-web-signature-31#section-4.1.1">
	 * <code>alg</code></a> (Algorithm) header value. A {@code null} value will
	 * remove the property from the JSON map.
	 *
	 * <p>
	 * The algorithm header parameter identifies the cryptographic algorithm
	 * used to secure the JWS. Consider using a type-safe
	 * {@link io.jsonwebtoken.SignatureAlgorithm SignatureAlgorithm} instance
	 * and using its {@link io.jsonwebtoken.SignatureAlgorithm#getValue() value}
	 * as the argument to this method.
	 * </p>
	 *
	 * @param alg the JWS {@code alg} header value or {@code null} to remove the
	 *            property from the JSON map.
	 * @return the {@code Header} instance for method chaining.
	 */
	public JwsHeader setAlgorithm(String alg) {
		getImplAsJwsHeader().setAlgorithm(alg);
		return this;
	}

	/**
	 * Returns the JWS <a href=
	 * "https://tools.ietf.org/html/draft-ietf-jose-json-web-signature-31#section-4.1.4">
	 * <code>kid</code></a> (Key ID) header value or {@code null} if not
	 * present.
	 *
	 * <p>
	 * The keyId header parameter is a hint indicating which key was used to
	 * secure the JWS. This parameter allows originators to explicitly signal a
	 * change of key to recipients. The structure of the keyId value is
	 * unspecified.
	 * </p>
	 *
	 * <p>
	 * When used with a JWK, the keyId value is used to match a JWK
	 * {@code keyId} parameter value.
	 * </p>
	 *
	 * @return the JWS {@code kid} header value or {@code null} if not present.
	 */
	public String getKeyId() {
		return getImplAsJwsHeader().getKeyId();
	}

	/**
	 * Sets the JWT <a href=
	 * "https://tools.ietf.org/html/draft-ietf-jose-json-web-signature-31#section-4.1.4">
	 * <code>kid</code></a> (Key ID) header value. A {@code null} value will
	 * remove the property from the JSON map.
	 *
	 * <p>
	 * The keyId header parameter is a hint indicating which key was used to
	 * secure the JWS. This parameter allows originators to explicitly signal a
	 * change of key to recipients. The structure of the keyId value is
	 * unspecified.
	 * </p>
	 *
	 * <p>
	 * When used with a JWK, the keyId value is used to match a JWK
	 * {@code keyId} parameter value.
	 * </p>
	 *
	 * @param kid the JWS {@code kid} header value or {@code null} to remove the
	 *            property from the JSON map.
	 * @return the {@code Header} instance for method chaining.
	 */
	public JwsHeader setKeyId(String kid) {
		getImplAsJwsHeader().setKeyId(kid);
		return this;
	}
}
