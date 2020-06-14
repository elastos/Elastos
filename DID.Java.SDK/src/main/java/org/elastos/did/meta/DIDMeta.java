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

import java.text.ParseException;
import java.util.Date;

import org.elastos.did.util.JsonHelper;

public class DIDMeta extends Metadata {
	private static final long serialVersionUID = -6074640560591492115L;

	private final static String TXID = RESERVED_PREFIX + "txid";
	private final static String PREV_TXID = RESERVED_PREFIX + "prevTxid";
	private final static String SIGNATURE = RESERVED_PREFIX + "signature";
	private final static String PUBLISHED = RESERVED_PREFIX + "published";
	private final static String ALIAS = RESERVED_PREFIX + "alias";
	private final static String DEACTIVATED = RESERVED_PREFIX + "deactivated";

	public void setAlias(String alias) {
		put(ALIAS, alias);
	}

	public String getAlias() {
		return (String)get(ALIAS);
	}

	public void setTransactionId(String txid) {
		put(TXID, txid);
	}

	public String getTransactionId() {
		return (String)get(TXID);
	}

	public void setPreviousTransactionId(String txid) {
		put(PREV_TXID, txid);
	}

	public String getPreviousTransactionId() {
		return (String)get(PREV_TXID);
	}

	public void setSignature(String signature) {
		put(SIGNATURE, signature);
	}

	public String getSignature() {
		return (String)get(SIGNATURE);
	}

	public void setPublished(Date timestamp) {
		put(PUBLISHED, JsonHelper.formatDate(timestamp));
	}

	public Date getPublished() {
		try {
			return JsonHelper.parseDate((String)get(PUBLISHED));
		} catch (ParseException e) {
			return null;
		}
	}

	public void setDeactivated(boolean deactivated) {
		put(DEACTIVATED, deactivated);
	}

	public boolean isDeactivated( ) {
		Boolean v = (Boolean)get(DEACTIVATED);
		return v == null ? false : v;
	}
}
