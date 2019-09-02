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

package org.elastos.did;

import java.util.TimeZone;

public final class Constants {
	public final static String id = "id";
	public final static String publicKey = "publicKey";
	public final static String type = "type";
	public final static String controller = "controller";
	public final static String publicKeyBase58 = "publicKeyBase58";
	public final static String authentication = "authentication";
	public final static String authorization = "authorization";
	public final static String service = "service";
	public final static String serviceEndpoint = "serviceEndpoint";
	public final static String expires = "expires";

	public final static String credential = "verifiableCredential";
	public final static String issuer = "issuer";
	public final static String issuanceDate = "issuanceDate";
	public final static String expirationDate = "expirationDate";
	public final static String credentialSubject = "credentialSubject";
	public final static String proof = "proof";
	public final static String created = "created";
	public final static String verifiableCredential = "verifiableCredential";
	public final static String verificationMethod = "verificationMethod";
	public final static String signature = "signature";
	public final static String nonce = "nonce";
	public final static String realm = "realm";

	public final static String defaultPublicKeyType = "ECDSAsecp256r1";

	public final static String DATE_FORMAT = "yyyy-MM-dd'T'HH:mm:ss'Z'";

	public final static TimeZone UTC = TimeZone.getTimeZone("UTC");
}
