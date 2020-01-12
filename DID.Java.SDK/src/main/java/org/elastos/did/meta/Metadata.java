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

import java.io.IOException;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;

import org.elastos.did.DIDStore;
import org.elastos.did.exception.MalformedMetaException;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.JsonNodeFactory;
import com.fasterxml.jackson.databind.node.ObjectNode;

public abstract class Metadata {
	private final static String EXTRA_PREFIX = "X-";

	private DIDStore store;
	private Map<String, String> extra;

	protected void setExtraInternal(String name, String value) {
		if (!name.startsWith(EXTRA_PREFIX))
			return;

		if (extra == null)
			extra = new TreeMap<String, String>();

		extra.put(name, value);
	}

	protected Map<String, String> getExtra() {
		return extra;
	}

	public void setStore(DIDStore store) {
		this.store = store;
	}

	public DIDStore getStore() {
		return store;
	}

	public boolean attachedStore() {
		return store != null;
	}

	public void setExtra(String name, String value) {
		if (extra == null && value == null)
			return;

		setExtraInternal(EXTRA_PREFIX + name, value);
	}

	public String getExtra(String name) {
		if (extra == null)
			return null;

		return extra.get(EXTRA_PREFIX + name);
	}

	protected abstract void fromNode(JsonNode node) throws MalformedMetaException;

	public static <T extends Metadata> T fromJson(JsonNode node,
			Class<T> clazz) throws MalformedMetaException {
		T meta = null;
		try {
			meta = clazz.newInstance();
		} catch (Exception e) {
			// should never go here!!!
			return null;
		}

		if (node == null || node.size() == 0)
			return meta;

		meta.fromNode(node);

		Iterator<Map.Entry<String,JsonNode>> it = node.fields();
		while (it.hasNext()) {
			Map.Entry<String,JsonNode> field = it.next();
			JsonNode vn = field.getValue();
			String key = field.getKey();
			String value = vn != null ? vn.asText() : null;

			meta.setExtraInternal(key, value);
		}

		return meta;
	}

	protected static <T extends Metadata> T fromJson(String metadata,
			Class<T> clazz) throws MalformedMetaException {
		JsonNode node = null;

		if (metadata != null && !metadata.isEmpty()) {
			ObjectMapper mapper = new ObjectMapper();

			try {
				node = mapper.readTree(metadata);
			} catch (IOException e) {
				throw new MalformedMetaException("Parse DID metadata error.", e);
			}
		}

		return fromJson(node, clazz);
	}

	protected abstract void toNode(ObjectNode node);

	@Override
	public String toString() {
		ObjectNode node = JsonNodeFactory.instance.objectNode();

		toNode(node);

		Map<String, String> extra = getExtra();
		if (extra != null && extra.size() > 0) {
			for (Map.Entry<String, String> entry : extra.entrySet())
				node.put(entry.getKey(), entry.getValue());
		}

		return node.size() != 0 ? node.toString() : null;
	}

	public void merge(Metadata meta) {
		if (meta != null && meta.extra != null)
			extra.putAll(meta.extra);
	}

	public boolean isEmpty() {
		if (extra == null)
			return true;

		return extra.isEmpty();
	}
}
