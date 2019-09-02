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

import java.math.BigInteger;
import java.util.Arrays;

final class BigIntegerUtils {

    static BigInteger parse256(final byte[] bytes) {
        return new BigInteger(1, bytes);
    }

    static void ser256(final byte[] target, final BigInteger integer) {
        if (integer.bitLength() > target.length * 8)
            throw new RuntimeException("ser256 failed, cannot fit integer in buffer");
        final byte[] modArr = integer.toByteArray();
        Arrays.fill(target, (byte) 0);
        copyTail(modArr, target);
        Arrays.fill(modArr, (byte) 0);
    }

    private static void copyTail(final byte[] src, final byte[] dest) {
        if (src.length < dest.length) {
            System.arraycopy(src, 0, dest, dest.length - src.length, src.length);
        } else {
            System.arraycopy(src, src.length - dest.length, dest, 0, dest.length);
        }
    }
}