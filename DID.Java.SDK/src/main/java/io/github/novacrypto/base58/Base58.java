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

/**
 * Class for encoding byte arrays to base58.
 * Suitable for small data arrays as the algorithm is O(n^2).
 * Don't share instances across threads.
 * Static methods are threadsafe however.
 */
public final class Base58 {

    Base58() {
    }

    public static GeneralEncoderDecoder newInstanceWithBuffer(final WorkingBuffer workingBuffer) {
        return new Base58EncoderDecoder(workingBuffer);
    }

    public static EncoderDecoder newInstance() {
        return newInstanceWithBuffer(new ByteArrayWorkingBuffer());
    }

    public static SecureEncoderDecoder newSecureInstance() {
        return newInstanceWithBuffer(new SecureWorkingBuffer());
    }

    private static final ThreadLocal<EncoderDecoder> working = new ThreadLocal<>();

    /**
     * Encodes given bytes as a number in base58.
     * Threadsafe, uses an instance per thread.
     *
     * @param bytes bytes to encode
     * @return base58 string representation
     */
    public static String base58Encode(final byte[] bytes) {
        return getThreadSharedBase58().encode(bytes);
    }

    /**
     * Decodes given bytes as a number in base58.
     * Threadsafe, uses an instance per thread.
     *
     * @param base58 string to decode
     * @return number as bytes
     */
    public static byte[] base58Decode(final CharSequence base58) {
        return getThreadSharedBase58().decode(base58);
    }

    private static EncoderDecoder getThreadSharedBase58() {
        EncoderDecoder base58 = working.get();
        if (base58 == null) {
            base58 = newInstance();
            working.set(base58);
        }
        return base58;
    }
}