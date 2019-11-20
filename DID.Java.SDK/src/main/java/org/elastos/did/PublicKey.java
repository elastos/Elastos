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

import org.elastos.did.util.Base58;
import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;

public class PublicKey extends DIDObject {
	private DID controller;
	private String keyBase58;

	protected PublicKey(DIDURL id, String type, DID controller, String keyBase58) {
		super(id, type);
		this.controller = controller;
		this.keyBase58 = keyBase58;
	}

	protected PublicKey(DIDURL id, DID controller, String keyBase58) {
		this(id, Constants.defaultPublicKeyType, controller, keyBase58);
	}

	public DID getController() {
		return controller;
	}

	public String getPublicKeyBase58() {
		return keyBase58;
	}

	public byte[] getPublicKeyBytes() {
		return Base58.decode(keyBase58);
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;

		if (obj instanceof PublicKey) {
			PublicKey ref = (PublicKey)obj;

			if (getId().equals(ref.getId()) &&
					getType().equals(ref.getType()) &&
					getController().equals(ref.getController()) &&
					getPublicKeyBase58().equals(ref.getPublicKeyBase58()))
				return true;
		}

		return false;
	}

	protected static PublicKey fromJson(JsonNode node, DID ref)
			throws MalformedDocumentException {
		Class<MalformedDocumentException> clazz = MalformedDocumentException.class;

		DIDURL id = JsonHelper.getDidUrl(node, Constants.id,
					ref, "publicKey' id", clazz);

		String type = JsonHelper.getString(node, Constants.type, true,
					Constants.defaultPublicKeyType, "publicKey' type", clazz);

		DID controller = JsonHelper.getDid(node, Constants.controller,
					true, ref, "publicKey' controller", clazz);

		String keyBase58 = JsonHelper.getString(node, Constants.publicKeyBase58,
					false, null, "publicKeyBase58", clazz);

		return new PublicKey(id, type, controller, keyBase58);
	}

	protected void toJson(JsonGenerator generator, DID ref, boolean compact)
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
		if (!compact || !getType().equals(Constants.defaultPublicKeyType)) {
			generator.writeFieldName(Constants.type);
			generator.writeString(getType());
		}

		// controller
		if (!compact || !controller.equals(ref)) {
			generator.writeFieldName(Constants.controller);
			generator.writeString(controller.toExternalForm());
		}

		// publicKeyBase58
		generator.writeFieldName(Constants.publicKeyBase58);
		generator.writeString(keyBase58);

		generator.writeEndObject();
	}
}
