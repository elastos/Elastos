/*
 *  BIP32derivation
 *  Copyright (C) 2017-2019 Alan Evans, NovaCrypto
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Original source: https://github.com/NovaCrypto/BIP32derivation
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto.bip32.derivation;

import java.util.HashMap;
import java.util.Map;

/**
 * Non-thread safe result cache for ckd functions.
 * <p>
 * If the same child of the same parent is requested a second time, the original result will be returned.
 *
 * @param <Key> Key Node type.
 */
public final class CkdFunctionResultCacheDecorator<Key> implements CkdFunction<Key> {

    public static <Key> CkdFunction<Key> newCacheOf(final CkdFunction<Key> decorated) {
        return new CkdFunctionResultCacheDecorator<>(decorated);
    }

    private final CkdFunction<Key> decoratedCkdFunction;

    private final Map<Key, HashMap<Integer, Key>> cache = new HashMap<>();

    private CkdFunctionResultCacheDecorator(final CkdFunction<Key> decoratedCkdFunction) {
        this.decoratedCkdFunction = decoratedCkdFunction;
    }

    @Override
    public Key deriveChildKey(final Key parent, final int childIndex) {
        final Map<Integer, Key> mapForParent = getMapOf(parent);
        Key child = mapForParent.get(childIndex);
        if (child == null) {
            child = decoratedCkdFunction.deriveChildKey(parent, childIndex);
            mapForParent.put(childIndex, child);
        }
        return child;
    }

    private Map<Integer, Key> getMapOf(final Key parentKey) {
        HashMap<Integer, Key> mapForParent = cache.get(parentKey);
        if (mapForParent == null) {
            mapForParent = new HashMap<>();
            cache.put(parentKey, mapForParent);
        }
        return mapForParent;
    }
}