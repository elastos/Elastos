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

package org.elastos.did.backend;

import java.io.IOException;
import java.io.Reader;
import java.io.StringWriter;
import java.io.Writer;
import java.util.LinkedList;
import java.util.List;

import org.elastos.did.DID;
import org.elastos.did.exception.DIDTransactionException;
import org.elastos.did.exception.MalformedResolveResultException;
import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class ResolveResult {
	private final static String DID = "did";
	private final static String STATUS = "status";
	private final static String TRANSACTION = "transaction";

	public static final int STATUS_VALID = 0;
	public static final int STATUS_EXPIRED = 1;
	public static final int STATUS_DEACTIVATED = 2;
	public static final int STATUS_NOT_FOUND = 3;

	private DID did;
	private int status;
	private List<IDTransactionInfo> idtxs;

	protected ResolveResult(DID did, int status) {
		this.did = did;
		this.status = status;
	}

	public DID getDid() {
		return did;
	}

	public int getStatus() {
		return status;
	}

	public int getTransactionCount() {
		if (idtxs == null)
			return 0;

		return idtxs.size();
	}

	public IDTransactionInfo getTransactionInfo(int index) {
		if (idtxs == null)
			return null;

		return idtxs.get(index);
	}

	protected synchronized void addTransactionInfo(IDTransactionInfo ti) {
		if (idtxs == null)
			idtxs = new LinkedList<IDTransactionInfo>();

		idtxs.add(ti);
	}

	public void toJson(Writer out) throws IOException {
		JsonFactory factory = new JsonFactory();
		JsonGenerator generator = factory.createGenerator(out);

		generator.writeStartObject();

		generator.writeStringField(DID, did.toString());
		generator.writeNumberField(STATUS, status);

		if (status != STATUS_NOT_FOUND) {
			generator.writeFieldName(TRANSACTION);
			generator.writeStartArray();

			for (IDTransactionInfo ti : idtxs)
				ti.toJson(generator);

			generator.writeEndArray();
		}

		generator.writeEndObject();
		generator.close();
	}

	public String toJson() throws IOException {
		Writer out = new StringWriter(4096);
		toJson(out);
		return out.toString();
	}

	public static ResolveResult fromJson(JsonNode result)
			throws MalformedResolveResultException {
		Class<MalformedResolveResultException> exceptionClass = MalformedResolveResultException.class;

		if (result == null || result.size() == 0)
			throw new MalformedResolveResultException("Empty resolve result.");

		DID did = JsonHelper.getDid(result, DID, false, null,
				"Resolved result DID", exceptionClass);

		int status = JsonHelper.getInteger(result, STATUS, false, -1,
					"Resolved status", exceptionClass);

		ResolveResult rr = new ResolveResult(did, status);

		if (status != STATUS_NOT_FOUND) {
			JsonNode txs = result.get(TRANSACTION);
			if (txs == null || !txs.isArray() || txs.size() == 0)
				throw new MalformedResolveResultException("Invalid resolve result, missing transaction.");

			for (int i = 0; i < txs.size(); i++) {
				try {
					IDTransactionInfo ti = IDTransactionInfo.fromJson(txs.get(i));
					rr.addTransactionInfo(ti);
				} catch (DIDTransactionException e) {
					new MalformedResolveResultException(e);
				}
			}
		}

		return rr;
	}

	public static ResolveResult fromJson(String json)
			throws MalformedResolveResultException {
		if (json == null || json.isEmpty())
			throw new IllegalArgumentException();

		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode result = mapper.readTree(json);
			return fromJson(result);
		} catch (IOException e) {
			throw new MalformedResolveResultException("Parse resolve result error.", e);
		}
	}

	public static ResolveResult fromJson(Reader in)
			throws MalformedResolveResultException {
		if (in == null)
			throw new IllegalArgumentException();

		ObjectMapper mapper = new ObjectMapper();
		try {
			JsonNode result = mapper.readTree(in);
			return fromJson(result);
		} catch (IOException e) {
			throw new MalformedResolveResultException("Parse resolve result error.", e);
		}
	}

	@Override
	public String toString() {
		try {
			return toJson();
		} catch (IOException ignore) {
		}

		return super.toString();
	}
}
