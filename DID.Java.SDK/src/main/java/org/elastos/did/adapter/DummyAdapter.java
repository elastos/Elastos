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

package org.elastos.did.adapter;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.util.Calendar;
import java.util.LinkedList;
import java.util.Random;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDAdapter;
import org.elastos.did.backend.IDChainRequest;
import org.elastos.did.backend.IDTransactionInfo;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.did.exception.DIDTransactionException;

import com.fasterxml.jackson.core.JsonEncoding;
import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerator;

public class DummyAdapter implements DIDAdapter {
	private static Random random = new Random();

	private boolean verbose;
	private LinkedList<IDTransactionInfo> idtxs;

	public DummyAdapter(boolean verbose) {
		this.verbose = verbose;

		idtxs = new LinkedList<IDTransactionInfo>();
	}

	public DummyAdapter() {
		this(false);
	}

	private static String generateTxid() {
        StringBuffer sb = new StringBuffer();
        while(sb.length() < 32){
            sb.append(Integer.toHexString(random.nextInt()));
        }

        return sb.toString();
    }

	private IDTransactionInfo getLastTransaction(DID did) {
		for (IDTransactionInfo ti : idtxs) {
			if (ti.getDid().equals(did))
				return ti;
		}

		return null;
	}

	private String createIdTransaction(String payload, String memo)
			throws DIDTransactionException {
		IDChainRequest request = IDChainRequest.fromJson(payload);

		if (verbose) {
			System.out.println("ID Transaction: " + request.getOperation()
					+ "[" + request.getDid() + "]");
			System.out.println("    " + request.toJson(false));

			if (request.getOperation() != IDChainRequest.Operation.DEACTIVATE)
				System.out.println("    " + request.getDocument().toString(true));
		}

		if (!request.isValid())
			throw new DIDTransactionException("Invalid ID transaction request.");

		if (request.getOperation() != IDChainRequest.Operation.DEACTIVATE) {
			if (!request.getDocument().isValid())
				throw new DIDTransactionException("Invalid DID Document.");
		}

		IDTransactionInfo ti = getLastTransaction(request.getDid());

		switch (request.getOperation()) {
		case CREATE:
			if (ti != null)
				throw new DIDTransactionException("DID already exist.");

			break;

		case UPDATE:
			if (ti == null)
				throw new DIDTransactionException("DID not exist.");

			if (ti.getOperation() == IDChainRequest.Operation.DEACTIVATE)
				throw new DIDTransactionException("DID already dactivated.");

			if (!request.getPreviousTxid().equals(ti.getTransactionId()))
				throw new DIDTransactionException("Previous transaction id missmatch.");

			break;

		case DEACTIVATE:
			if (ti == null)
				throw new DIDTransactionException("DID not exist.");

			if (ti.getOperation() == IDChainRequest.Operation.DEACTIVATE)
				throw new DIDTransactionException("DID already dactivated.");

			break;
		}

		ti = new IDTransactionInfo(generateTxid(),
				Calendar.getInstance(Constants.UTC).getTime(), request);
		idtxs.add(0, ti);

		return ti.getTransactionId();
	}

	@Override
	public void createIdTransaction(String payload, String memo,
			int confirms, TransactionCallback callback) {
		try {
			String txid = createIdTransaction(payload, memo);
			callback.accept(txid, 0, null);
		} catch (Exception e) {
			callback.accept(null, -1, e.getMessage());
		}
	}

	@Override
	public InputStream resolve(String requestId, String did, boolean all)
			throws DIDResolveException {
		ByteArrayOutputStream os = new ByteArrayOutputStream(4096);
		JsonFactory factory = new JsonFactory();
		boolean matched = false;

		if (verbose)
			System.out.print("Resolve: " + did + "...");

		if (!did.startsWith("did:elastos:"))
			did = "did:elastos:" + did;

		try {
			JsonGenerator generator = factory.createGenerator(os, JsonEncoding.UTF8);
			generator.writeStartObject();

			generator.writeStringField("id", requestId);
			generator.writeStringField("jsonrpc", "2.0");
			generator.writeFieldName("result");
			generator.writeStartObject();

			DID target = new DID(did);

			generator.writeStringField("did", target.toString());

			int status = 3;
			IDTransactionInfo last = getLastTransaction(target);
			if (last != null) {
				if (last.getOperation() == IDChainRequest.Operation.DEACTIVATE) {
					status = 2;
			    } else {
					if (last.getRequest().getDocument().isExpired())
						status = 1;
					else
						status = 0;
				}

				matched = true;
			}

			generator.writeNumberField("status", status);

			if (status != 3) {
				generator.writeFieldName("transaction");
				generator.writeStartArray();
				for (IDTransactionInfo ti : idtxs) {
					if (ti.getDid().equals(target)) {
						ti.toJson(generator);

						if (!all)
							break;
					}
				}
				generator.writeEndArray();
			}

			generator.writeEndObject();
			generator.writeEndObject();
			generator.close();
		} catch (Exception e) {
			throw new DIDResolveException(e);
		}

		if (verbose)
			System.out.println(matched ? "success" : "failed");

		return new ByteArrayInputStream(os.toByteArray());
	}

	public void reset() {
		idtxs.clear();
	}
}
