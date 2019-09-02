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

import java.nio.ByteBuffer;
import java.security.SecureRandom;

final class SecureWorkingBuffer implements WorkingBuffer {

    private ByteBuffer bytes;
    private final byte[] key = new byte[1021];

    SecureWorkingBuffer() {
        new SecureRandom().nextBytes(key);
    }

    @Override
    public void setCapacity(final int atLeast) {
        bytes = ensureCapacity(bytes, atLeast);
        clear(bytes);
    }

    @Override
    public byte get(final int index) {
        assertIndexValid(index);
        return encodeDecode(bytes.get(index), index);
    }

    @Override
    public void put(final int index, final byte value) {
        assertIndexValid(index);
        bytes.put(index, encodeDecode(value, index));
    }

    @Override
    public void clear() {
        clear(bytes);
    }

    private void assertIndexValid(final int index) {
        if (index < 0 || index >= capacity())
            throw new IndexOutOfBoundsException();
    }

    private int capacity() {
        return bytes == null ? 0 : bytes.capacity();
    }

    private ByteBuffer ensureCapacity(final ByteBuffer bytes, final int atLeast) {
        if (bytes != null && bytes.capacity() >= atLeast) {
            return bytes;
        }
        if (bytes != null)
            clear(bytes);
        return ByteBuffer.allocateDirect(atLeast);
    }

    private void clear(final ByteBuffer bytes) {
        bytes.position(0);
        final int capacity = bytes.capacity();
        for (int i = 0; i < capacity; i++) {
            bytes.put(i, encodeDecode((byte) 255, i));
        }
    }

    private byte encodeDecode(final byte b, final int index) {
        return (byte) (b ^ key[index % key.length]);
    }
}