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

package org.elastos.did;

import java.util.LinkedHashMap;
import java.util.Map;

import org.elastos.did.parser.DIDURLBaseListener;
import org.elastos.did.parser.DIDURLParser;
import org.elastos.did.parser.ParserHelper;

public class DIDURL implements Comparable<DIDURL> {
	private DID did;
	private Map<String, String> parameters;
	private String path;
	private Map<String, String> query;
	private String fragment;

	public DIDURL(DID id, String fragment) {
		this.did = id;
		this.fragment = fragment;
	}

	public DIDURL(String url) throws MalformedDIDURLException {
		try {
			ParserHelper.parse(url, false, new Listener());
		} catch(IllegalArgumentException e) {
			throw new MalformedDIDURLException(e.getMessage());
		}
	}

	public DID getDid() {
		return did;
	}

	public void setDid(DID did) {
		this.did = did;
	}

	private String mapToString(Map<String, String> map, String sep) {
		boolean init = true;
		if (map == null)
			return null;

		StringBuilder builder = new StringBuilder(512);
		for (Map.Entry<String, String> entry : map.entrySet()) {
			if (init)
				init = false;
			else
				builder.append(sep);

			builder.append(entry.getKey());
			if (entry.getValue() != null)
				builder.append("=").append(entry.getValue());
		}

		return builder.toString();
	}

	public String getParameters() {
		return mapToString(parameters, ";");
	}

	public String getParameter(String name) {
		if (parameters == null)
			return null;

		return parameters.get(name);
	}

	public boolean hasParameter(String name) {
		if (parameters == null)
			return false;

		return parameters.containsKey(name);
	}

	void addParameter(String name, String value) {
		parameters.put(name, value);
	}

	public String getPath() {
		return path;
	}

	void setPath(String path) {
		this.path = path;
	}

	public String getQuery() {
		return mapToString(query, "&");
	}

	public String getQueryParameter(String name) {
		if (query == null)
			return null;

		return query.get(name);
	}

	public boolean hasQueryParameter(String name) {
		if (query == null)
			return false;

		return query.containsKey(name);
	}

	void addQueryParameter(String name, String value) {
		query.put(name, value);
	}

	public String getFragment() {
		return fragment;
	}

	void setFragment(String fragment) {
		this.fragment = fragment;
	}

	public String toExternalForm() {
		StringBuilder builder = new StringBuilder(512);
		builder.append(did);

		if (parameters != null && !parameters.isEmpty())
			builder.append(";").append(getParameters());

		if (path != null && !path.isEmpty())
			builder.append(path);

		if (query != null && !query.isEmpty())
			builder.append("?").append(getQuery());

		if (fragment != null && !fragment.isEmpty())
			builder.append("#").append(getFragment());

		return builder.toString();
	}

	@Override
	public String toString() {
		return toExternalForm();
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == this)
			return true;

		if (obj instanceof DIDURL) {
			DIDURL id = (DIDURL)obj;
			return toExternalForm().equals(id.toExternalForm());
		}

		if (obj instanceof String) {
			String url = (String)obj;
			return toExternalForm().equals(url);
		}

		return false;
	}

	@Override
	public int compareTo(DIDURL id) {
		return toExternalForm().compareTo(id.toExternalForm());
	}


	private int mapHashCode(Map<String, String> map) {
		int hash = 0;

		if (map == null)
			return hash;

		for (Map.Entry<String, String> entry : map.entrySet()) {
			hash += entry.getKey().hashCode();
			if (entry.getValue() != null)
				hash += entry.getValue().hashCode();
		}

		return hash;
	}

	@Override
	public int hashCode() {
		int hash = did.hashCode();
		hash += mapHashCode(parameters);
		hash += path == null ? 0 : path.hashCode();
		hash += mapHashCode(query);
		hash += fragment == null ? 0 : fragment.hashCode();

		return hash;
	}

	class Listener extends DIDURLBaseListener {
		private String name;
		private String value;

		@Override
		public void enterDid(DIDURLParser.DidContext ctx) {
			did = new DID();
		}

		@Override
		public void exitMethod(DIDURLParser.MethodContext ctx) {
			String method = ctx.getText();
			if (!method.equals(DID.METHOD))
				throw new IllegalArgumentException("Unknown method: " + method);

			did.setMethod(DID.METHOD);
		}

		@Override
		public void exitMethodSpecificString(
				DIDURLParser.MethodSpecificStringContext ctx) {
			did.setMethodSpecificId(ctx.getText());
		}

		@Override
		public void enterParams(DIDURLParser.ParamsContext ctx) {
			parameters = new LinkedHashMap<String, String>(8);
		}

		@Override
		public void exitParamMethod(DIDURLParser.ParamMethodContext ctx) {
			String method = ctx.getText();
			if (!method.equals(DID.METHOD))
				throw new IllegalArgumentException(
						"Unknown parameter method: " + method);
		}

		@Override
		public void exitParamQName(DIDURLParser.ParamQNameContext ctx) {
			name = ctx.getText();
		}

		@Override
		public void exitParamValue(DIDURLParser.ParamValueContext ctx) {
			value = ctx.getText();
		}

		@Override
		public void exitParam(DIDURLParser.ParamContext ctx) {
			addParameter(name, value);
			name = null;
			value = null;
		}

		@Override
		public void exitPath(DIDURLParser.PathContext ctx) {
			setPath("/" + ctx.getText());
		}

		@Override
		public void enterQuery(DIDURLParser.QueryContext ctx) {
			query = new LinkedHashMap<String, String>(8);
		}

		@Override
		public void exitQueryParamName(DIDURLParser.QueryParamNameContext ctx) {
			name = ctx.getText();
		}

		@Override
		public void exitQueryParamValue(DIDURLParser.QueryParamValueContext ctx) {
			value = ctx.getText();
		}

		@Override
		public void exitQueryParam(DIDURLParser.QueryParamContext ctx) {
			addQueryParameter(name, value);
			name = null;
			value = null;
		}

		@Override
		public void exitFrag(DIDURLParser.FragContext ctx) {
			fragment = ctx.getText();
		}
	}
}
