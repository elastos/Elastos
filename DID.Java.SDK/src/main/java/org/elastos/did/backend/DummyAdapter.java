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

import java.io.StringWriter;
import java.io.Writer;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.Random;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDAdapter;
import org.elastos.did.DIDException;
import org.elastos.did.MalformedDIDException;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;

public class DummyAdapter implements DIDAdapter {
	private boolean verbose;

	private LinkedList<IDTx> txs;

	static class IDTx {
		private static Random random = new Random();

		private String txId;
		private Date timestamp;
		private IDChainRequest request;

		public IDTx(IDChainRequest request)
				throws MalformedDIDException {
			this.txId = generateTxId();
			this.timestamp = Calendar.getInstance(Constants.UTC).getTime();
			this.request = request;
		}

		private static String generateTxId() {
	        StringBuffer sb = new StringBuffer();
	        while(sb.length() < 32){
	            sb.append(Integer.toHexString(random.nextInt()));
	        }

	        return sb.toString();
	    }

		public String getTxId() {
			return txId;
		}

		public DID getDid() {
			return request.getDid();
		}

		public IDChainRequest.Operation getOperation() {
			return request.getOperation();
		}

		public Date getTimestamp() {
			return timestamp;
		}

		public String getPayload() {
			return request.toJson(false);
		}

		public String toJson() throws DIDException {
			Writer out = new StringWriter(4096);
			JsonFactory factory = new JsonFactory();

			try {
				JsonGenerator generator = factory.createGenerator(out);
				generator.writeStartObject();

				generator.writeNullField("id");
				generator.writeStringField("jsonrpc", "2.0");
				generator.writeFieldName("result");
				generator.writeStartArray();
				request.toJson(generator, false);
				generator.writeEndArray();

				generator.writeEndObject();
				generator.close();
			} catch (Exception e) {
				throw new DIDException("Can not serialize ID transaction.", e);
			}

			return out.toString();
		}
	}

	public DummyAdapter(boolean verbose) {
		this.verbose = verbose;

		txs = new LinkedList<IDTx>();
	}

	public DummyAdapter() {
		this(false);
	}

	@Override
	public boolean createIdTransaction(String payload, String memo)
			throws DIDException {
		try {
			IDChainRequest request = IDChainRequest.fromJson(payload);

			if (verbose) {
				System.out.println("ID Transaction: " + request.getOperation()
						+ request.getDid());
				System.out.println("    " + request.toJson(false));
			}

			txs.add(0, new IDTx(request));

			return true;
		} catch (DIDException e) {
			throw new DIDException("Parse ID Transaction payload error.", e);
		}
	}

	@Override
	public String resolve(String did) throws DIDException {
		if (!did.startsWith("did:elastos:"))
			did = "did:elastos:" + did;

		if (verbose)
			System.out.print("Resolve: " + did + "...");

		DID target = new DID(did);
		for (IDTx tx : txs) {
			if (tx.getDid().equals(target)) {
				if (verbose)
					System.out.println("success");

				return tx.toJson();
			}
		}

		if (verbose)
			System.out.println("failed");

		return null;
	}
}
