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
import java.util.Date;

import org.elastos.did.DID;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDTransaction;
import org.elastos.did.exception.DIDTransactionException;
import org.elastos.did.util.JsonHelper;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.JsonNode;

public class IDChainTransaction implements DIDTransaction {
	private final static String TXID = "txid";
	private final static String TIMESTAMP = "timestamp";
	private final static String OPERATION = "operation";

	private String txId;
	private Date timestamp;
	private IDChainRequest request;

	public IDChainTransaction(String txid, Date timestamp,
			IDChainRequest request) {
		this.txId = txid;
		this.timestamp = timestamp;
		this.request = request;
	}

	@Override
	public String getTransactionId() {
		return txId;
	}

	@Override
	public Date getTimestamp() {
		return timestamp;
	}

	@Override
	public DID getDid() {
		return request.getDid();
	}

	public IDChainRequest.Operation getOperationCode() {
		return request.getOperation();
	}

	public String getPayload() {
		return request.toJson(false);
	}

	public IDChainRequest getRequest() {
		return request;
	}

	@Override
	public String getOperation() {
		return getOperationCode().toString();
	}

	@Override
	public DIDDocument getDocument() {
		return request.getDocument();
	}

	public void toJson(JsonGenerator generator) throws IOException {
		generator.writeStartObject();
		generator.writeStringField(TXID, getTransactionId());
		generator.writeStringField(TIMESTAMP, JsonHelper.formatDate(getTimestamp()));
		generator.writeFieldName(OPERATION);
		request.toJson(generator, false);
		generator.writeEndObject();
	}

	public static IDChainTransaction fromJson(JsonNode node)
			throws DIDTransactionException {
		Class<DIDTransactionException> exceptionClass = DIDTransactionException.class;

		if (node == null || node.size() == 0)
			return null;

		String txid = JsonHelper.getString(node, TXID, false, null,
				"transaction id", exceptionClass);

		Date timestamp = JsonHelper.getDate(node, TIMESTAMP, false,
				null, "transaction timestamp", exceptionClass);

		JsonNode reqNode = node.get(OPERATION);
		if (reqNode == null)
			throw new DIDTransactionException("Missing ID operation.");

		IDChainRequest request = IDChainRequest.fromJson(reqNode);

		return new IDChainTransaction(txid, timestamp, request);
	}
}
