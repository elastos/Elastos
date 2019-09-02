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

package io.github.novacrypto.bip32;

import java.util.Arrays;

final class ByteArrayWriter {

    private final byte[] bytes;
    private int idx = 0;

    ByteArrayWriter(final byte[] target) {
        this.bytes = target;
    }

    void concat(final byte[] bytesSource, final int length) {
        System.arraycopy(bytesSource, 0, bytes, idx, length);
        idx += length;
    }

    void concat(final byte[] bytesSource) {
        concat(bytesSource, bytesSource.length);
    }

    /**
     * ser32(i): serialize a 32-bit unsigned integer i as a 4-byte sequence, most significant byte first.
     *
     * @param i a 32-bit unsigned integer
     */
    void concatSer32(final int i) {
        concat((byte) (i >> 24));
        concat((byte) (i >> 16));
        concat((byte) (i >> 8));
        concat((byte) (i));
    }

    void concat(final byte b) {
        bytes[idx++] = b;
    }

    static byte[] tail32(final byte[] bytes64) {
        final byte[] ir = new byte[bytes64.length - 32];
        System.arraycopy(bytes64, 32, ir, 0, ir.length);
        return ir;
    }

    static byte[] head32(final byte[] bytes64) {
        return Arrays.copyOf(bytes64, 32);
    }
}