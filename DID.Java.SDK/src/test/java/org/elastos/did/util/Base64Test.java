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

package org.elastos.did.util;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

public class Base64Test {
	@Test
	public void testDecode() {
		String input = "eyJpYXQiOjE1MTYyMzkwMjIsInN1YiI6IjEyMzQ1Njc4OTAiLCJuYW1lIjoiSm9obiBEb2UifQ";
		String expectedOutput = "{\"iat\":1516239022,\"sub\":\"1234567890\",\"name\":\"John Doe\"}";

		byte[] output = Base64.decode(input, Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		assertEquals(expectedOutput, new String(output));
	}

	@Test
	public void testEncode() {
		String input = "{\"iat\":1516239022,\"sub\":\"1234567890\",\"name\":\"John Doe\"}";
		String expectedOutput = "eyJpYXQiOjE1MTYyMzkwMjIsInN1YiI6IjEyMzQ1Njc4OTAiLCJuYW1lIjoiSm9obiBEb2UifQ";

		String output = Base64.encodeToString(input.getBytes(), Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		assertEquals(expectedOutput, output);
	}
}
