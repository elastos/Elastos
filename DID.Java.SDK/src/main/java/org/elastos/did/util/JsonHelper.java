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

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;
import java.util.List;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDURL;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.MalformedDIDException;
import org.elastos.did.exception.MalformedDIDURLException;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;

public class JsonHelper {
	private final static SimpleDateFormat dateFormat =
			new SimpleDateFormat(Constants.DATE_FORMAT);

	private final static SimpleDateFormat isoDateFormat =
			new SimpleDateFormat(Constants.DATE_FORMAT_ISO_8601);

	static {
		dateFormat.setTimeZone(Constants.UTC);
		isoDateFormat.setTimeZone(Constants.UTC);
	}

	static class ExceptionFactory {
		static public <E> E create(Class<E> clazz, String msg) {
			try {
				Constructor<E> ctor = clazz.getConstructor(String.class);
				return ctor.newInstance(msg);
			} catch (Exception e) {
				throw new RuntimeException("Can't instantiate object: "
						+ clazz.getCanonicalName());
			}
		}

		static public <E> E create(Class<E> clazz, String msg, Exception e) {
			try {
				Constructor<E> ctor = clazz.getDeclaredConstructor(
						String.class, Throwable.class);
				return ctor.newInstance(msg, e);
			} catch (Exception ex) {
				ex.printStackTrace();
				throw new RuntimeException("Can't instantiate object: "
						+ clazz.getCanonicalName());
			}
		}
	}

	public static <E extends DIDException> String getString(JsonNode node,
			String name, boolean optional, String ref, String hint,
			Class<E> exceptionClass) throws E {
		JsonNode vn = node.get(name);
		if (vn == null) {
			if (optional)
				return ref;
			else
				throw ExceptionFactory.create(exceptionClass, "Missing " + hint + ".");
		}

		if (!vn.isTextual())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		String value = vn.asText();
		if (value == null || value.isEmpty())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		return value;
	}

	public static <E extends DIDException> int getInteger(JsonNode node,
			String name, boolean optional, int ref, String hint,
			Class<E> exceptionClass) throws E {
		JsonNode vn = node.get(name);
		if (vn == null) {
			if (optional)
				return ref;
			else
				throw ExceptionFactory.create(exceptionClass, "Missing " + hint + ".");
		}

		if (!vn.isNumber())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		return vn.asInt(ref);
	}

	public static <E extends DIDException> DID getDid(JsonNode node,
			String name, boolean optional, DID ref, String hint,
			Class<E> exceptionClass) throws E {
		JsonNode vn = node.get(name);
		if (vn == null) {
			if (optional)
				return ref;
			else
				throw ExceptionFactory.create(exceptionClass, "Missing " + hint + ".");
		}

		if (!vn.isTextual())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		String value = vn.asText();
		if (value == null || value.isEmpty())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		DID id;
		try {
			id = new DID(value);
		} catch (MalformedDIDException e) {
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + ": " + value, e);
		}

		return id;
	}

	public static <E extends DIDException> DIDURL getDidUrl(JsonNode node,
			String name, boolean optional, DID ref, String hint,
			Class<E> exceptionClass) throws E {
		JsonNode vn = node.get(name);
		if (vn == null) {
			if (optional)
				return null;
			else
				throw ExceptionFactory.create(exceptionClass, "Missing " + hint + ".");
		}

		if (!vn.isTextual())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		String value = vn.asText();
		if (value == null || value.isEmpty())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		DIDURL id;
		try {
			if (ref != null && value.startsWith("#"))
				id = new DIDURL(ref, value.substring(1));
			else
				id = new DIDURL(value);
		} catch (MalformedDIDURLException e) {
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + ": " + value, e);
		}

		return id;
	}

	public static <E extends DIDException> DIDURL getDidUrl(JsonNode node,
			String name, DID ref, String hint,
			Class<E> exceptionClass) throws E {
		return getDidUrl(node, name, false, ref, hint, exceptionClass);
	}

	public static <E extends DIDException> DIDURL getDidUrl(JsonNode node,
			DID ref, String hint, Class<E> exceptionClass) throws E {
		if (node == null || !node.isTextual())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		String value = node.asText();
		if (value == null || value.isEmpty())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		DIDURL id;
		try {
			if (ref != null && value.startsWith("#"))
				id = new DIDURL(ref, value.substring(1));
			else
				id = new DIDURL(value);
		} catch (MalformedDIDURLException e) {
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + ": " + value, e);
		}

		return id;
	}

	public static <E extends DIDException> Date getDate(JsonNode node,
			String name, boolean optional, Date ref, String hint,
			Class<E> exceptionClass) throws E {
		JsonNode vn = node.get(name);
		if (vn == null) {
			if (optional)
				return ref;
			else
				throw ExceptionFactory.create(exceptionClass, "Missing " + hint + ".");
		}

		if (!vn.isTextual())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		String value = vn.asText();
		if (value == null || value.isEmpty())
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + " value.");

		try {
			return parseDate(value);
		} catch (ParseException e) {
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + ": " + value, e);
		}
	}

	public static void toJson(JsonGenerator generator, JsonNode node)
			throws IOException {
		toJson(generator, node, false);
	}

	public static void toJson(JsonGenerator generator, JsonNode node,
			boolean objectContext) throws IOException {
		switch (node.getNodeType()) {
		case ARRAY:
			generator.writeStartArray();
			Iterator<JsonNode> elems = node.elements();
			while (elems.hasNext())
				toJson(generator, elems.next());
			generator.writeEndArray();
			break;

		case BINARY:
			generator.writeBinary(node.binaryValue());
			break;

		case BOOLEAN:
			generator.writeBoolean(node.asBoolean());
			break;

		case MISSING:
			generator.writeString(node.asText());
			break;

		case NULL:
			generator.writeNull();
			break;

		case NUMBER:
			generator.writeNumber(node.asText());
			break;

		case OBJECT:
		case POJO:
			if (!objectContext)
				generator.writeStartObject();

			List<String> fields = new ArrayList<String>(node.size());

			Iterator<String> it = node.fieldNames();
			while (it.hasNext())
				fields.add(it.next());

			Collections.sort(fields);
			for (String field : fields) {
				generator.writeFieldName(field);
				toJson(generator, node.get(field));
			}

			if (!objectContext)
				generator.writeEndObject();
			break;

		case STRING:
			generator.writeString(node.asText());
			break;
		}
	}

	public static String formatDate(Date date) {
		return dateFormat.format(date);
	}

	public static Date parseDate(String dataStr) throws ParseException {
		try {
			return dateFormat.parse(dataStr);
		} catch (ParseException ignore) {
		}

		// Failback to ISO 8601 format.
		return isoDateFormat.parse(dataStr);
	}
}
