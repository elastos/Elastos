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

package org.elastos.did.meta;

import org.elastos.did.exception.MalformedMetaException;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

public class CredentialMeta extends Metadata {
	private final static String ALIAS = "alias";

	private String alias;

	public void setAlias(String alias) {
		this.alias = alias;
	}

	public String getAlias() {
		return alias;
	}

	public static CredentialMeta fromString(String metadata)
			throws MalformedMetaException {
		return fromString(metadata, CredentialMeta.class);
	}

	@Override
	protected void fromNode(JsonNode node) throws MalformedMetaException {
		JsonNode value = node.get(ALIAS);
		if (value != null)
			setAlias(value.asText());
	}

	@Override
	protected void toNode(ObjectNode node) {
		if (alias != null)
			node.put(ALIAS, alias);
	}

	@Override
	public void merge(Metadata meta) {
		if (meta == null)
			return;

		if (!(meta instanceof CredentialMeta))
			throw new IllegalArgumentException();

		CredentialMeta m = (CredentialMeta)meta;

		if (m.alias != null)
			alias = m.alias;

		super.merge(meta);
	}
}
