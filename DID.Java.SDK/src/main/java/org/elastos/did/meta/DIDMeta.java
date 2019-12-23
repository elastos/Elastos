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
import java.text.SimpleDateFormat;
import java.util.Date;

import org.elastos.did.Constants;
import org.elastos.did.exception.MalformedMetaException;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

public class DIDMeta extends Metadata {
	private final static SimpleDateFormat dateFormat =
			new SimpleDateFormat(Constants.DATE_FORMAT);

	private boolean deactivated;
	private Date updated;
	private String txid;
	private String alias;

	public void setAlias(String alias) {
		this.alias = alias;
	}

	public String getAlias() {
		return alias;
	}

	public void setTransactionId(String txid) {
		this.txid = txid;
	}

	public String getTransactionId() {
		return txid;
	}

	public void setUpdated(Date updated) {
		this.updated = updated;
	}

	public Date getUpdated() {
		return updated;
	}

	public void setDeactivated(boolean deactivated) {
		this.deactivated = deactivated;
	}

	public boolean isDeactivated( ) {
		return deactivated;
	}

	public static DIDMeta fromString(String metadata)
			throws MalformedMetaException {
		return fromString(metadata, DIDMeta.class);
	}

	@Override
	protected void fromNode(JsonNode node) throws MalformedMetaException {
		JsonNode value = node.get(Constants.alias);
		if (value != null)
			setAlias(value.asText());

		value = node.get(Constants.deactivated);
		if (value != null)
			setDeactivated(value.asBoolean());

		value = node.get(Constants.txid);
		if (value != null)
			setTransactionId(value.asText());

		value = node.get(Constants.timestamp);
		if (value != null) {
			try {
				Date updated = dateFormat.parse(value.asText());
				setUpdated(updated);
			} catch (ParseException ignore) {
			}
		}
	}

	@Override
	protected void toNode(ObjectNode node) {
		if (alias != null)
			node.put(Constants.alias, alias);

		if (deactivated)
			node.put(Constants.deactivated, true);

		if (txid != null)
			node.put(Constants.txid, txid);

		if (updated != null)
			node.put(Constants.timestamp, dateFormat.format(updated));
	}

	@Override
	public void merge(Metadata meta) {
		if (meta == null)
			return;

		if (!(meta instanceof DIDMeta))
			throw new IllegalArgumentException();

		DIDMeta m = (DIDMeta)meta;

		if (m.alias != null)
			alias = m.alias;

		deactivated = m.deactivated;

		if (m.txid != null)
			txid = m.txid;

		if (m.updated != null)
			updated = m.updated;

		super.merge(meta);
	}
}
