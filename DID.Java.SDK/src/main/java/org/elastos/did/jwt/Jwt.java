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

public class Jwt<B> {
	private io.jsonwebtoken.Jwt<?, ?> impl;

	protected Jwt(io.jsonwebtoken.Jwt<?, ?> impl) {
		this.impl = impl;
	}

	protected io.jsonwebtoken.Jwt<?, ?> getImpl() {
		return impl;
	}

	/**
	 * Returns the JWT {@link Header} or {@code null} if not present.
	 *
	 * @return the JWT {@link Header} or {@code null} if not present.
	 */
	public Header getHeader() {
		return new Header(impl.getHeader());
	}

	/**
	 * Returns the JWT body, either a {@code String} or a {@code Claims}
	 * instance.
	 *
	 * @return the JWT body, either a {@code String} or a {@code Claims}
	 *         instance.
	 */
	@SuppressWarnings("unchecked")
	public B getBody() {
		Object body = impl.getBody();
		if (body instanceof io.jsonwebtoken.Claims) {
			body = new Claims((io.jsonwebtoken.Claims) body);
		}

		return (B) body;
	}
}
