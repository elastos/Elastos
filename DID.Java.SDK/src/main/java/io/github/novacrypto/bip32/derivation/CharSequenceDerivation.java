/*
 *  BIP32 library, a Java implementation of BIP32
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
 *  Original source: https://github.com/NovaCrypto/BIP32
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto.bip32.derivation;

import static io.github.novacrypto.bip32.Index.hard;
import static io.github.novacrypto.bip32.Index.isHardened;

public enum CharSequenceDerivation implements Derivation<CharSequence> {
    INSTANCE;

    @Override
    public <T> T derive(final T rootKey, final CharSequence derivationPath, final CkdFunction<T> ckdFunction) {
        final int length = derivationPath.length();
        if (length == 0)
            throw new IllegalArgumentException("Path cannot be empty");
        if (derivationPath.charAt(0) != 'm')
            throw new IllegalArgumentException("Path must start with m");
        if (length == 1)
            return rootKey;
        if (derivationPath.charAt(1) != '/')
            throw new IllegalArgumentException("Path must start with m/");
        T currentKey = rootKey;
        int buffer = 0;
        for (int i = 2; i < length; i++) {
            final char c = derivationPath.charAt(i);
            switch (c) {
                case '\'':
                    buffer = hard(buffer);
                    break;
                case '/':
                    currentKey = ckdFunction.deriveChildKey(currentKey, buffer);
                    buffer = 0;
                    break;
                default:
                    buffer *= 10;
                    if (c < '0' || c > '9')
                        throw new IllegalArgumentException("Illegal character in path: " + c);
                    buffer += c - '0';
                    if (isHardened(buffer))
                        throw new IllegalArgumentException("Index number too large");
            }
        }
        return ckdFunction.deriveChildKey(currentKey, buffer);
    }
}