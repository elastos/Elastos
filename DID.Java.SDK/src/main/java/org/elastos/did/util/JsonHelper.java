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

import java.lang.reflect.Constructor;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDException;
import org.elastos.did.DIDURL;
import org.elastos.did.MalformedDIDException;
import org.elastos.did.MalformedDIDURLException;

import com.fasterxml.jackson.databind.JsonNode;

public class JsonHelper {
	private final static SimpleDateFormat dateFormat =
			new SimpleDateFormat(Constants.DATE_FORMAT);

	static {
		dateFormat.setTimeZone(Constants.UTC);
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

	public static <E extends DIDException> DID getDid(JsonNode node,
			String name,boolean optional, DID ref, String hint,
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
			String name, DID ref, String hint,
			Class<E> exceptionClass) throws E {
		JsonNode vn = node.get(name);
		if (vn == null)
			throw ExceptionFactory.create(exceptionClass, "Missing " + hint + ".");

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
			return dateFormat.parse(value);
		} catch (ParseException e) {
			throw ExceptionFactory.create(exceptionClass, "Invalid " + hint + ": " + value, e);
		}
	}

	public static String format(Date date) {
		return dateFormat.format(date);
	}

}
