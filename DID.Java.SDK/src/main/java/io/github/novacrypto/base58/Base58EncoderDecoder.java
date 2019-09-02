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

import java.util.Arrays;

import static io.github.novacrypto.base58.CapacityCalculator.maximumBase58StringLength;

final class Base58EncoderDecoder implements GeneralEncoderDecoder {

    private static final char[] DIGITS = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz".toCharArray();
    private static final int[] VALUES = initValues(DIGITS);

    private final WorkingBuffer workingBuffer;
    private final StringBuilderEncodeTarget target = new StringBuilderEncodeTarget();

    Base58EncoderDecoder(final WorkingBuffer workingBuffer) {
        this.workingBuffer = workingBuffer;
    }

    @Override
    public String encode(final byte[] bytes) {
        target.clear();
        encode(bytes, target, target);
        return target.toString();
    }

    @Override
    public void encode(final byte[] bytes, final EncodeTargetFromCapacity target) {
        final int characters = maximumBase58StringLength(bytes.length);
        encode(bytes, target.withCapacity(characters), characters);
    }

    @Override
    public void encode(final byte[] bytes, final EncodeTargetCapacity setCapacity, final EncodeTarget target) {
        final int characters = maximumBase58StringLength(bytes.length);
        setCapacity.setCapacity(characters);
        encode(bytes, target, characters);
    }

    @Override
    public void encode(final byte[] bytes, final EncodeTarget target) {
        final int characters = maximumBase58StringLength(bytes.length);
        encode(bytes, target, characters);
    }

    private void encode(final byte[] bytes, final EncodeTarget target, final int capacity) {
        final char[] a = DIGITS;
        final int bLen = bytes.length;
        final WorkingBuffer d = getBufferOfAtLeastBytes(capacity);
        try {
            int dlen = -1;
            int blanks = 0;
            int j = 0;
            for (int i = 0; i < bLen; i++) {
                int c = bytes[i] & 0xff;
                if (c == 0 && blanks == i) {
                    target.append(a[0]);
                    blanks++;
                }
                j = 0;
                while (j <= dlen || c != 0) {
                    int n;
                    if (j > dlen) {
                        dlen = j;
                        n = c;
                    } else {
                        n = d.get(j);
                        n = (n << 8) + c;
                    }
                    d.put(j, (byte) (n % 58));
                    c = n / 58;
                    j++;
                }
            }
            while (j-- > 0) {
                target.append(a[d.get(j)]);
            }
        } finally {
            d.clear();
        }
    }

    @Override
    public byte[] decode(final CharSequence base58) {
        final ByteArrayTarget target = new ByteArrayTarget();
        decode(base58, target);
        return target.asByteArray();
    }

    @Override
    public void decode(final CharSequence base58, final DecodeTarget target) {
        final int strLen = base58.length();
        final WorkingBuffer d = getBufferOfAtLeastBytes(strLen);
        try {
            int dlen = -1;
            int blanks = 0;
            int j = 0;
            for (int i = 0; i < strLen; i++) {
                j = 0;
                final char charAtI = base58.charAt(i);
                int c = valueOf(charAtI);
                if (c < 0) {
                    throw new BadCharacterException(charAtI);
                }
                if (c == 0 && blanks == i) {
                    blanks++;
                }
                while (j <= dlen || c != 0) {
                    int n;
                    if (j > dlen) {
                        dlen = j;
                        n = c;
                    } else {
                        n = d.get(j) & 0xff;
                        n = n * 58 + c;
                    }
                    d.put(j, (byte) n);
                    c = n >>> 8;
                    j++;
                }
            }
            final int outputLength = j + blanks;
            final DecodeWriter writer = target.getWriterForLength(outputLength);
            for (int i = 0; i < blanks; i++) {
                writer.append((byte) 0);
            }
            final int end = outputLength - 1;
            for (int i = blanks; i < outputLength; i++) {
                writer.append(d.get(end - i));
            }
        } finally {
            d.clear();
        }
    }

    private WorkingBuffer getBufferOfAtLeastBytes(final int atLeast) {
        workingBuffer.setCapacity(atLeast);
        return workingBuffer;
    }

    private static int[] initValues(final char[] alphabet) {
        final int[] lookup = new int['z' + 1];
        Arrays.fill(lookup, -1);
        for (int i = 0; i < alphabet.length; i++)
            lookup[alphabet[i]] = i;
        return lookup;
    }

    private static int valueOf(final char base58Char) {
        if (base58Char >= VALUES.length)
            return -1;
        return VALUES[base58Char];
    }
}