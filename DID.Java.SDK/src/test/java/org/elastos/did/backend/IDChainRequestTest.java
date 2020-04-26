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

package org.elastos.did.backend;

import static org.junit.jupiter.api.Assertions.assertTrue;

import org.elastos.did.DIDDocument;
import org.elastos.did.exception.DIDException;
import org.junit.jupiter.api.Test;

public class IDChainRequestTest {
	@Test
	public void test() throws DIDException {
		String json = "{\"header\":{\"operation\":\"create\",\"specification\":\"elastos/did/1.0\"},\"payload\":\"eyJpZCI6ImRpZDplbGFzdG9zOmlkeXp2bUV5Z2MxendLNUFkc1plVmFVSkV0VzdRMWliOWciLCJwdWJsaWNLZXkiOlt7ImlkIjoiI3ByaW1hcnkiLCJwdWJsaWNLZXlCYXNlNTgiOiJjaG5pajhrTUhQWkQ3NHFHUzN6NEpKZGNvdjJnZEpucU5NZzZIVnFpbjFCUSJ9XSwiYXV0aGVudGljYXRpb24iOlsiI3ByaW1hcnkiXSwidmVyaWZpYWJsZUNyZWRlbnRpYWwiOlt7ImlkIjoiI25hbWUiLCJ0eXBlIjpbIkJhc2ljUHJvZmlsZUNyZWRlbnRpYWwiLCJTZWxmUHJvY2xhaW1lZENyZWRlbnRpYWwiXSwiaXNzdWFuY2VEYXRlIjoiMjAyMC0wNC0yMVQwMjo0NDoyNloiLCJleHBpcmF0aW9uRGF0ZSI6IjIwMjUtMDQtMjBUMDI6NDQ6MjZaIiwiY3JlZGVudGlhbFN1YmplY3QiOnsibmFtZSI6eyJib29sVHlwZSI6dHJ1ZSwiZG91YmxlVHlwZSI6Mi4xMjM0NTYsImludFR5cGUiOjIwMjAsIm51bGwiOm51bGwsIm9iaiI6eyJib29sVHlwZSI6dHJ1ZSwiZG91YmxlVHlwZSI6MC4xMjM0NTYsImludFR5cGUiOjEwMDAwLCJzdHJpbmdUeXBlIjoiaXBmczovL2VlZWVlZWUifSwic3RyaW5nVHlwZSI6ImlwZnM6Ly9lcmRkZGRkZGRkZGRkZGRkZGQifX0sInByb29mIjp7InZlcmlmaWNhdGlvbk1ldGhvZCI6IiNwcmltYXJ5Iiwic2lnbmF0dXJlIjoicXJsbHV4ZDdJaGFQMHFqVlFzeFZTQVFCRjk4cmhpSzBNVHNCZnBrVmxIS2hLVkdSc3VWYzIwQ0tweno3bTZDZHNUaEt2WGQ3Wm9MeVlLSl9wODdEVWcifX0seyJpZCI6IiN0ZWxlcGhvbmUiLCJ0eXBlIjpbIkJhc2ljUHJvZmlsZUNyZWRlbnRpYWwiLCJTZWxmUHJvY2xhaW1lZENyZWRlbnRpYWwiXSwiaXNzdWFuY2VEYXRlIjoiMjAyMC0wNC0yMVQwMjo0NDozOFoiLCJleHBpcmF0aW9uRGF0ZSI6IjIwMjUtMDQtMjBUMDI6NDQ6MzhaIiwiY3JlZGVudGlhbFN1YmplY3QiOnsidGVsZXBob25lIjp7ImJvb2xUeXBlIjp0cnVlLCJkb3VibGVUeXBlIjoyLjEyMzQ1NiwiaW50VHlwZSI6MjAyMCwibnVsbCI6bnVsbCwib2JqIjp7ImJvb2xUeXBlIjp0cnVlLCJkb3VibGVUeXBlIjowLjEyMzQ1NiwiaW50VHlwZSI6MTAwMDAsInN0cmluZ1R5cGUiOiJpcGZzOi8vZWVlZWVlZSJ9LCJzdHJpbmdUeXBlIjoiaXBmczovL2VyZGRkZGRkZGRkZGRkZGRkZCJ9fSwicHJvb2YiOnsidmVyaWZpY2F0aW9uTWV0aG9kIjoiI3ByaW1hcnkiLCJzaWduYXR1cmUiOiJYVDlRTlJkV193ZmZpcHNVSl9KOTFnVzFSQWwxUS1JZFlhYU10emRPbGt1S21wYWdJcTZ0TUtVX09ibk5makRYdEpqSXpHb0QtbWFIU2ZuMURZX012QSJ9fV0sImV4cGlyZXMiOiIyMDI1LTA0LTIxVDAyOjQzOjQ0WiIsInByb29mIjp7ImNyZWF0ZWQiOiIyMDIwLTA0LTIxVDAyOjQ1OjE5WiIsInNpZ25hdHVyZVZhbHVlIjoiMmZXU3VJbjhPY3JzRkVqbGI2VXZNQ1JJRTNSVmw3WlpndmJ4dE5VakdXUkVYTGg5N0F0N0dVYnFkSVBqX3hHWElmR3BoaVJyWTNEVy1Pa1k4NTlvT0EifX0\",\"proof\":{\"signature\":\"PJvh1BNUE_NmSBdveaEvMLOQq6-JNLEtiYOIEp1qCyPCChW7OrSA9PrRMUvxUsHqXp1Hx26nBChkmeUQeeX1sw\",\"type\":\"ECDSAsecp256r1\",\"verificationMethod\":\"did:elastos:idyzvmEygc1zwK5AdsZeVaUJEtW7Q1ib9g#primary\"}}";
		IDChainRequest req = IDChainRequest.fromJson(json);
		assertTrue(req.isValid());

		DIDDocument doc = req.getDocument();
		// System.out.println(doc.toString(true));
		assertTrue(doc.isGenuine());
	}
}
