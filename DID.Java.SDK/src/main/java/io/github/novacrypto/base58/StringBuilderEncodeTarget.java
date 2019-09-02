/*
 *  Base58 library, a Java implementation of Base58 encode/decode
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
 *  Original source: https://github.com/NovaCrypto/Base58
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto.base58;

final class StringBuilderEncodeTarget implements EncodeTarget, EncodeTargetCapacity {
    private final StringBuilder sb = new StringBuilder();

    @Override
    public void setCapacity(final int characters) {
        sb.ensureCapacity(characters);
    }

    @Override
    public void append(final char c) {
        sb.append(c);
    }

    @Override
    public String toString() {
        return sb.toString();
    }

    void clear() {
        sb.setLength(0);
    }
}