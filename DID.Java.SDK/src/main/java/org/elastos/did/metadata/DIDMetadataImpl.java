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

import java.text.ParseException;
import java.util.Date;

import org.elastos.did.DIDMetadata;
import org.elastos.did.DIDStore;
import org.elastos.did.util.JsonHelper;

public class DIDMetadataImpl extends AbstractMetadata implements DIDMetadata {
	private static final long serialVersionUID = -6074640560591492115L;

	private final static String TXID = RESERVED_PREFIX + "txid";
	private final static String PREV_SIGNATURE = RESERVED_PREFIX + "prevSignature";
	private final static String SIGNATURE = RESERVED_PREFIX + "signature";
	private final static String PUBLISHED = RESERVED_PREFIX + "published";
	private final static String ALIAS = RESERVED_PREFIX + "alias";
	private final static String DEACTIVATED = RESERVED_PREFIX + "deactivated";

	public DIDMetadataImpl() {
		this(null);
	}

	public DIDMetadataImpl(DIDStore store) {
		super(store);
	}

	@Override
	public void setAlias(String alias) {
		put(ALIAS, alias);
	}

	@Override
	public String getAlias() {
		return (String)get(ALIAS);
	}

	public void setTransactionId(String txid) {
		put(TXID, txid);
	}

	@Override
	public String getTransactionId() {
		return (String)get(TXID);
	}

	public void setPreviousSignature(String txid) {
		put(PREV_SIGNATURE, txid);
	}

	@Override
	public String getPreviousSignature() {
		return (String)get(PREV_SIGNATURE);
	}

	public void setSignature(String signature) {
		put(SIGNATURE, signature);
	}

	@Override
	public String getSignature() {
		return (String)get(SIGNATURE);
	}

	public void setPublished(Date timestamp) {
		put(PUBLISHED, JsonHelper.formatDate(timestamp));
	}

	@Override
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

	@Override
	public boolean isDeactivated( ) {
		Boolean v = (Boolean)get(DEACTIVATED);
		return v == null ? false : v;
	}
}
