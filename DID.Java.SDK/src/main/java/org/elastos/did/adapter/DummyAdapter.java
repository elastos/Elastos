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

import java.io.StringWriter;
import java.io.Writer;
import java.util.Calendar;
import java.util.LinkedList;
import java.util.Random;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDAdapter;
import org.elastos.did.backend.IDChainRequest;
import org.elastos.did.backend.IDTransactionInfo;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDResolveException;

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

	@Override
	public boolean createIdTransaction(String payload, String memo)
			throws DIDException {
		try {
			IDChainRequest request = IDChainRequest.fromJson(payload);

			if (verbose) {
				System.out.println("ID Transaction: " + request.getOperation()
						+ "[" + request.getDid() + "]");
				System.out.println("    " + request.toJson(false));

				if (request.getOperation() != IDChainRequest.Operation.DEACRIVATE)
					System.out.println("    " + request.getDocument().toString());
			}

			IDTransactionInfo ti = new IDTransactionInfo(generateTxid(),
					Calendar.getInstance(Constants.UTC).getTime(), request);
			idtxs.add(0, ti);

			return true;
		} catch (DIDException e) {
			throw new DIDException("Parse ID Transaction payload error.", e);
		}
	}

	@Override
	public String resolve(String did, boolean all) throws DIDResolveException {
		Writer out = new StringWriter(4096);
		JsonFactory factory = new JsonFactory();
		boolean matched = false;

		if (verbose)
			System.out.print("Resolve: " + did + "...");

		if (!did.startsWith("did:elastos:"))
			did = "did:elastos:" + did;

		try {
			JsonGenerator generator = factory.createGenerator(out);
			generator.writeStartObject();

			generator.writeNullField("id");
			generator.writeStringField("jsonrpc", "2.0");
			generator.writeFieldName("result");
			generator.writeStartObject();

			DID target = new DID(did);

			generator.writeStringField("did", target.toString());

			int status = 3;
			for (IDTransactionInfo ti : idtxs) {
				if (ti.getDid().equals(target)) {
					if (ti.getOperation() == IDChainRequest.Operation.DEACRIVATE)
						status = 2;
					else {
						if (ti.getRequest().getDocument().isExpired())
							status = 1;
						else
							status = 0;
					}

					matched = true;
				}
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

		return out.toString();
	}
}
