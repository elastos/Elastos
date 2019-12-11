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
import java.util.Calendar;
import java.util.Date;

import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;

public class Proof {
	private String type;
	private Date created;
	private DIDURL creator;
	private String signature;

	protected Proof(String type, Date created, DIDURL creator, String signature) {
		this.type = type;
		this.created = created;
		this.creator = creator;
		this.signature = signature;
	}

	protected Proof(DIDURL creator, String signature) {
		this(Constants.defaultPublicKeyType,
				Calendar.getInstance(Constants.UTC).getTime(),
				creator, signature);
	}

    public String getType() {
    	return type;
    }

    public Date getCreated() {
    	return created;
    }

    public DIDURL getCreator() {
    	return creator;
    }

    public String getSignature() {
    	return signature;
    }

	protected static Proof fromJson(JsonNode node, DIDURL refSignKey)
			throws MalformedDocumentException {
		Class<MalformedDocumentException> clazz = MalformedDocumentException.class;

		String type = JsonHelper.getString(node, Constants.type,
				true, Constants.defaultPublicKeyType,
				"document proof type", clazz);

		Date created = JsonHelper.getDate(node, Constants.created,
				true, null, "proof created date", clazz);

		DIDURL creator = JsonHelper.getDidUrl(node, Constants.creator, true,
				refSignKey.getDid(), "document proof creator", clazz);
		if (creator == null)
			creator = refSignKey;

		String signature = JsonHelper.getString(node, Constants.signatureValue,
				false, null, "document proof signature", clazz);

		return new Proof(type, created, creator, signature);
	}

	protected void toJson(JsonGenerator generator, boolean normalized)
			throws IOException {
		generator.writeStartObject();

		// type
		if (normalized || !type.equals(Constants.defaultPublicKeyType)) {
			generator.writeFieldName(Constants.type);
			generator.writeString(type);
		}

		// created
		if (created != null) {
			generator.writeFieldName(Constants.created);
			generator.writeString(JsonHelper.format(created));
		}

		// creator
		if (normalized) {
			generator.writeFieldName(Constants.creator);
			generator.writeString(creator.toString());
		}

		// signature
		generator.writeFieldName(Constants.signatureValue);
		generator.writeString(signature);

		generator.writeEndObject();
	}
}
