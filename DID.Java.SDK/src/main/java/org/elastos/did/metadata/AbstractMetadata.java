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

package org.elastos.did.metadata;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.StringWriter;
import java.io.Writer;
import java.text.ParseException;
import java.util.Date;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;

import org.elastos.did.DIDStore;
import org.elastos.did.exception.MalformedMetaException;
import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public abstract class AbstractMetadata extends TreeMap<String, Object> {
	private static final long serialVersionUID = -3700036981800046481L;

	protected final static String RESERVED_PREFIX = "DX-";
	private final static String LAST_MODIFIED = RESERVED_PREFIX + "lastModified";

	private DIDStore store;

	protected AbstractMetadata(DIDStore store) {
		this.store = store;
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

	public void setLastModified(Date timestamp) {
		put(LAST_MODIFIED, JsonHelper.formatDate(timestamp));
	}

	public Date getLastModified() {
		try {
			String lastModified = (String)get(LAST_MODIFIED);
			return lastModified == null ? null : JsonHelper.parseDate(lastModified);
		} catch (ParseException e) {
			return null;
		}
	}

	public void clearLastModified() {
		remove(LAST_MODIFIED);
	}

	public void setExtra(String key, String value) {
		if (key == null || key.isEmpty())
			throw new IllegalArgumentException();

		put(key, value);
	}

	public String getExtra(String key) {
		if (key == null || key.isEmpty())
			throw new IllegalArgumentException();

		return (String)get(key);
	}

	public void merge(AbstractMetadata metadata) {
		if (metadata == this)
			return;

		metadata.forEach((k, v) -> {
			if (containsKey(k)) {
				if (get(k) == null)
					remove(k);
			} else {
				if (v != null)
					put(k, v);
			}
		});
	}

	public void load(Reader reader) throws MalformedMetaException {
		try {
			ObjectMapper mapper = new ObjectMapper();
			JsonNode node = mapper.readTree(reader);
			load(node);
		} catch (IOException e) {
			throw new MalformedMetaException("Parse DID metadata error.", e);
		}
	}

	public void load(JsonNode node) throws MalformedMetaException {
		for (Iterator<Map.Entry<String, JsonNode>> i = node.fields(); i.hasNext(); ) {
			Map.Entry<String, JsonNode> field = i.next();
			String key = field.getKey();
			JsonNode n = field.getValue();

			switch (n.getNodeType()) {
			case BOOLEAN:
				this.put(key, n.asBoolean());
				break;

			case STRING:
				this.put(key, n.asText());
				break;

			case NUMBER:
				this.put(key, n.asLong());
				break;

			case NULL:
				break;

		    default:
		    	throw new MalformedMetaException("Unsupported field: " + key);
			}
		}
	}

	private void save(JsonGenerator generator) throws IOException {
		generator.writeStartObject();

		for (Map.Entry<String, Object> field: entrySet()) {
			String k = field.getKey();
			Object v = field.getValue();

			if (v == null)
				continue;
			else if (v instanceof Integer)
				generator.writeNumberField(k, (int)v);
			else if (v instanceof Long)
				generator.writeNumberField(k, (long)v);
			else if (v instanceof Boolean)
				generator.writeBooleanField(k, (boolean)v);
			else if (v instanceof String)
				generator.writeStringField(k, (String)v);
			else if (v instanceof Date)
				generator.writeStringField(k, JsonHelper.formatDate((Date)v));
			else
				throw new IOException("Can not serialize attribute: " + k);
		}

		generator.writeEndObject();
	}

	public void save(Writer out) throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

		JsonFactory factory = new JsonFactory();
		JsonGenerator generator = factory.createGenerator(out);
		save(generator);
		generator.close();
	}

	public void save(OutputStream out) throws IOException {
		if (out == null)
			throw new IllegalArgumentException();

		save(new OutputStreamWriter(out, "UTF-8"));
	}

	public String toJson() throws IOException {
		Writer out = new StringWriter(1024);
		save(out);
		return out.toString();
	}

	@Override
	public String toString() {
		try {
			return toJson();
		} catch (IOException ignore) {
			return "";
		}
	}
}
