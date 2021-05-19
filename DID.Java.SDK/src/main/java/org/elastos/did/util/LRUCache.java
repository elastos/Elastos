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

package org.elastos.did.util;

import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;

public class LRUCache<K, V> {
	public static <K, V> Map<K, V> createInstance(
			int initialCapacity, int maxCapacity) {
		return Collections.synchronizedMap(
				new LRUCache<K, V>().new SimpleLRUCache<K, V>(
						initialCapacity, maxCapacity));
	}

	class SimpleLRUCache<Key, Value> extends LinkedHashMap<Key, Value> {
		private static final long serialVersionUID = 1583913975981000601L;

		private int cacheSize;

		protected SimpleLRUCache(int initialCapacity, int maxCapacity) {
			super(initialCapacity, (float)0.75, true);
			this.cacheSize = maxCapacity;
		}

		@Override
		protected boolean removeEldestEntry(Map.Entry<Key, Value> eldest) {
			return size() > cacheSize;
		}
	}
}
