/*
 *  SecureString library, Obfuscated/clearable in memory string management
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
 *  Original source: https://github.com/NovaCrypto/SecureString
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto;

final class ProxyCharSequence implements CharSequence {
    private final CharSequence parent;
    private final int start;
    private final int end;
    private final ToStringStrategy toStringStrategy;

    static CharSequence secureSubSequenceProxy(
            final CharSequence parent,
            final int start,
            final int end,
            final ToStringStrategy toStringStrategy) {
        final int length = parent.length();
        if ((start > end) || (start < 0) || (end > length))
            throw new StringIndexOutOfBoundsException();
        return new ProxyCharSequence(parent, start, end, toStringStrategy);
    }

    private ProxyCharSequence(
            final CharSequence parent,
            final int start,
            final int end,
            final ToStringStrategy toStringStrategy) {
        this.parent = parent;
        this.start = start;
        this.end = end;
        this.toStringStrategy = toStringStrategy;
    }

    public int length() {
        return end - start;
    }

    public char charAt(int index) {
        if (index < 0 || index >= length())
            throw new IndexOutOfBoundsException();
        return parent.charAt(index + start);
    }

    public CharSequence subSequence(int start, int end) {
        if (start == 0 && end == length())
            return this;
        return secureSubSequenceProxy(parent,
                this.start + start,
                this.start + end,
                toStringStrategy);
    }

    @Override
    public String toString() {
        return toStringStrategy.toString(this);
    }
}