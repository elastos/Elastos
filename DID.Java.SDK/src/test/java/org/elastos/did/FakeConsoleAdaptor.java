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

import java.io.IOException;

import org.elastos.did.backend.DIDAdaptor;
import org.elastos.did.util.Base64;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class FakeConsoleAdaptor implements DIDAdaptor {
	@Override
	public boolean createIdTransaction(String payload, String memo)
			throws DIDException {
		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode tx = mapper.readTree(payload);

			JsonNode header = tx.get("header");
			System.out.println("Operation: " + header.get("operation").asText());

			JsonNode _payload = tx.get("payload");
			String json = new String(Base64.decode(_payload.asText(),
					Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP));
			System.out.println("        " + json);

			return true;
		} catch (IOException e) {
			throw new MalformedDocumentException("Parse JSON document error.", e);
		}
	}

	@Override
	public String resolve(String did) throws DIDException {
		System.out.println("Operation: resolve");
		System.out.println("        " + did);
		return null;
	}

}
