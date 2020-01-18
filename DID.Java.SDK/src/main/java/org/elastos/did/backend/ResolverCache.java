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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.Writer;
import java.nio.charset.Charset;
import java.util.Map;

import org.elastos.did.DID;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.did.util.LRUCache;

public class ResolverCache {
	private static final Charset utf8 = Charset.forName("UTF-8");

	private static final int CACHE_INITIAL_CAPACITY = 16;
	private static final int CACHE_MAX_CAPACITY = 32;

	private static File rootDir;
	private static Map<DID, ResolveResult> cache = LRUCache.createInstance(
			CACHE_INITIAL_CAPACITY, CACHE_MAX_CAPACITY);

	public static void setCacheDir(File rootDir) {
		ResolverCache.rootDir = rootDir;

		if (!rootDir.exists())
			rootDir.mkdirs();
	}

	private static File getCacheDir() {
		if (rootDir == null)
			throw new IllegalStateException("No cache dir specified for ResolverCache");

		return rootDir;
	}

	private static File getFile(String id) {
		String filename = getCacheDir().getAbsolutePath() + File.separator + id;
		return new File(filename);
	}

	public static void reset() {
		cache.clear();

		File[] children = getCacheDir().listFiles();
		for (File child : children)
			child.delete();
	}

	public static void store(ResolveResult rr) throws IOException {
		OutputStream os = null;
		Writer out = null;

		try {
			os = new FileOutputStream(getFile(rr.getDid().getMethodSpecificId()));
			out = new OutputStreamWriter(os, utf8);
			out.write(rr.toJson());

			cache.put(rr.getDid(), rr);
		} finally {
			if (out != null) {
				try {
					out.close();
				} catch (IOException ignore) {
				}
			}

			if (os != null) {
				try {
					os.close();
				} catch (IOException ignore) {
				}
			}
		}
	}

	public static ResolveResult load(DID did, long ttl)
			throws DIDResolveException {
		File file = getFile(did.getMethodSpecificId());

		if (!file.exists())
			return null;

		if (System.currentTimeMillis() > (file.lastModified() + ttl))
			return null;

		if (cache.containsKey(did))
			return cache.get(did);

		InputStream is = null;
		Reader in = null;

		try {
			is = new FileInputStream(file);
			in = new InputStreamReader(is, utf8);
			ResolveResult rr = ResolveResult.fromJson(in);
			cache.put(rr.getDid(), rr);
			return rr;
		} catch (IOException e) {
			throw new DIDResolveException(e);
		} finally {
			if (in != null) {
				try {
					in.close();
				} catch (IOException ignore) {
				}
			}

			if (is != null) {
				try {
					is.close();
				} catch (IOException ignore) {
				}
			}
		}
	}
}
