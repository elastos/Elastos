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

import java.io.Closeable;

/**
 * A store of char data that is encrypted with a one-time-pad.
 * Data is pinned outside of garbage collected heap.
 */
public final class SecureCharBuffer implements Closeable, CharSequence {

    /**
     * @param capacity maximum number of chars for buffer to store
     * @return a new {@link SecureCharBuffer} instance
     */
    public static SecureCharBuffer withCapacity(int capacity) {
        return new SecureCharBuffer(capacity);
    }

    private final SecureByteBuffer buffer;

    private SecureCharBuffer(int capacity) {
        buffer = SecureByteBuffer.withCapacity(capacity * 2);
    }

    public SecureCharBuffer() {
        this(512);
    }

    public void append(char c) {
        final byte msb = (byte) (c >> 8);
        final byte lsb = (byte) (c & 0xff);
        buffer.append(msb);
        buffer.append(lsb);
    }

    public void append(final CharSequence data) {
        final int length = data.length();
        for (int i = 0; i < length; i++) {
            append(data.charAt(i));
        }
    }

    public int length() {
        return buffer.length() / 2;
    }

    public char charAt(int index) {
        final int position = index * 2;
        final byte msb = buffer.get(position);
        final byte lsb = buffer.get(position + 1);
        return (char) (msb << 8 | lsb & 0xff);
    }

    /**
     * Same as {@link #charAt}, provides indexer syntax in Kotlin
     *
     * @param index 0-based index of char to fetch
     * @return char at that index
     */
    public char get(int index) {
        return charAt(index);
    }

    public CharSequence subSequence(final int start, final int end) {
        if (start == 0 && end == length())
            return this;
        return ProxyCharSequence
                .secureSubSequenceProxy(this,
                        start,
                        end,
                        ToStringStrategy.RESTRICT);
    }

    public CharSequence toStringAble() {
        return ProxyCharSequence
                .secureSubSequenceProxy(this, 0, length(),
                        ToStringStrategy.ALLOW);
    }

    public int capacity() {
        return buffer.capacity() / 2;
    }

    public void close() {
        buffer.close();
    }

    @Override
    public String toString() {
        throw new UnsupportedOperationException();
    }
}