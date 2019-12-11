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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import org.junit.Before;
import org.junit.Test;

public class DIDURLTest {
	private static final String testDID = "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN";
	private static final String params = "elastos:foo=testvalue;bar=123;keyonly;elastos:foobar=12345";
	private static final String path = "/path/to/the/resource";
	private static final String query = "qkey=qvalue&qkeyonly&test=true";
	private static final String fragment = "testfragment";
	private static final String testURL = testDID + ";" + params + path + "?" + query + "#" + fragment;

	private DIDURL url;

    @Before
    public void setup() throws MalformedDIDURLException {
    	url = new DIDURL(testURL);
    }

	@Test
	public void testConstructor() throws MalformedDIDURLException {
		String testURL = testDID;
		DIDURL url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID + ";" + params;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID + path;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID + "?" + query;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID + "#" + fragment;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID + ";" + params + path;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID + ";" + params + path + "?" + query;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID + ";" + params + path + "?" + query + "#" + fragment;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID  + path + "?" + query + "#" + fragment;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID + ";" + params + "?" + query + "#" + fragment;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID + ";" + params + path + "#" + fragment;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());

		testURL = testDID + ";" + params + path + "?" + query;
		url = new DIDURL(testURL);
		assertEquals(testURL, url.toString());
	}

	@Test(expected = MalformedDIDURLException.class)
	public void testConstructorError1() throws MalformedDIDURLException {
		@SuppressWarnings("unused")
		DIDURL url = new DIDURL("id:elastos:1234567890" + ";" + params + path + "?" + query + "#" + fragment);
	}

	@Test(expected = MalformedDIDURLException.class)
	public void testConstructorError2() throws MalformedDIDURLException {
		@SuppressWarnings("unused")
		DIDURL url = new DIDURL("did:example:1234567890" + ";" + params + path + "?" + query + "#" + fragment);
	}

	@Test(expected = MalformedDIDURLException.class)
	public void testConstructorError3() throws MalformedDIDURLException {
		@SuppressWarnings("unused")
		DIDURL url = new DIDURL("did:elastos:1234567890" + ";" + path + "?" + query + "#" + fragment);
	}

	@Test(expected = MalformedDIDURLException.class)
	public void testConstructorError4() throws MalformedDIDURLException {
		@SuppressWarnings("unused")
		DIDURL url = new DIDURL("did:example:1234567890" + ";" + params + path + "?" + "#" + fragment);
	}

	@Test(expected = MalformedDIDURLException.class)
	public void testConstructorError5() throws MalformedDIDURLException {
		@SuppressWarnings("unused")
		DIDURL url = new DIDURL("did:example:1234567890" + ";" + params + path + "?" + query + "#");
	}

	@Test
	public void testGetDid() {
		assertEquals(testDID, url.getDid().toString());
	}

	@Test
	public void testGetParameters() {
		assertEquals(params, url.getParameters());
	}

	@Test
	public void testGetParameter() {
		assertEquals("testvalue", url.getParameter("elastos:foo"));
		assertNull(url.getParameter("foo"));
		assertEquals("123", url.getParameter("bar"));
		assertEquals("12345", url.getParameter("elastos:foobar"));
		assertNull(url.getParameter("foobar"));
		assertNull(url.getParameter("keyonly"));
	}

	@Test
	public void testHasParameter() {
		assertTrue(url.hasParameter("elastos:foo"));
		assertTrue(url.hasParameter("bar"));
		assertTrue(url.hasParameter("elastos:foobar"));
		assertTrue(url.hasParameter("keyonly"));

		assertFalse(url.hasParameter("notexist"));
		assertFalse(url.hasParameter("foo"));
		assertFalse(url.hasParameter("boobar"));
	}

	@Test
	public void testGetPath() {
		assertEquals(path, url.getPath());
	}

	@Test
	public void testGetQuery() {
		assertEquals(query, url.getQuery());
	}

	@Test
	public void testGetQueryParameter() {
		assertEquals("qvalue", url.getQueryParameter("qkey"));
		assertEquals("true", url.getQueryParameter("test"));
		assertNull(url.getQueryParameter("qkeyonly"));
	}

	@Test
	public void testHasQueryParameter() {
		assertTrue(url.hasQueryParameter("qkeyonly"));
		assertTrue(url.hasQueryParameter("qkey"));
		assertTrue(url.hasQueryParameter("test"));

		assertFalse(url.hasQueryParameter("notexist"));
	}

	@Test
	public void testGetFragment() {
		assertEquals(fragment, url.getFragment());
	}

	@Test
	public void testToString() {
		assertEquals(testURL, url.toString());
	}

	@Test
	public void testHashCode() throws MalformedDIDURLException {
		DIDURL other = new DIDURL(testURL);
		assertEquals(url.hashCode(), other.hashCode());

		other = new DIDURL("did:elastos:1234567890#test");
		assertNotEquals(url.hashCode(), other.hashCode());
	}

	@SuppressWarnings("unlikely-arg-type")
	@Test
	public void testEquals() throws MalformedDIDURLException {
		DIDURL other = new DIDURL(testURL);
		assertTrue(url.equals(other));
		assertTrue(url.equals(testURL));

		other = new DIDURL("did:elastos:1234567890#test");
		assertFalse(url.equals(other));
		assertFalse(url.equals("did:elastos:1234567890#test"));
	}
}
