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

import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;

public class Service extends DIDObject {
	private String endpoint;

	protected Service(DIDURL id, String type, String endpoint) {
		super(id, type);
		this.endpoint = endpoint;
	}

	public String getServiceEndpoint() {
		return endpoint;
	}

	static Service fromJson(JsonNode node, DID ref)
			throws MalformedDocumentException {
		Class<MalformedDocumentException> clazz = MalformedDocumentException.class;

		DIDURL id = JsonHelper.getDidUrl(node, Constants.id,
				ref, "service' id", clazz);

		String type = JsonHelper.getString(node, Constants.type, false,
				null, "service' type", clazz);

		String endpoint = JsonHelper.getString(node, Constants.serviceEndpoint,
				false, null, "service' endpoint", clazz);

		return new Service(id, type, endpoint);
	}

	public void toJson(JsonGenerator generator, DID ref, boolean compact)
			throws IOException {
		compact = (ref != null && compact);

		String value;

		generator.writeStartObject();

		// id
		generator.writeFieldName(Constants.id);
		if (compact && getId().getDid().equals(ref))
			value = "#" + getId().getFragment();
		else
			value = getId().toExternalForm();
		generator.writeString(value);

		// type
		generator.writeFieldName(Constants.type);
		generator.writeString(getType());

		// endpoint
		generator.writeFieldName(Constants.serviceEndpoint);
		generator.writeString(endpoint);

		generator.writeEndObject();
	}

}
