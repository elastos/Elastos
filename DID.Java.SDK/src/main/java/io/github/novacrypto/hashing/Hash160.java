/*
 *  Hash160
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
 *  Original source: https://github.com/NovaCrypto/Hash160
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto.hashing;

import org.spongycastle.crypto.digests.RIPEMD160Digest;

import static io.github.novacrypto.hashing.Sha256.sha256;

public final class Hash160 {

    private static final int RIPEMD160_DIGEST_SIZE = 20;

    public static byte[] hash160(final byte[] bytes) {
        return ripemd160(sha256(bytes));
    }

    public static void hash160into(final byte[] target, final int offset, final byte[] bytes) {
        ripemd160into(sha256(bytes), target, offset);
    }

    private static byte[] ripemd160(final byte[] bytes) {
        final byte[] output = new byte[RIPEMD160_DIGEST_SIZE];
        ripemd160into(bytes, output, 0);
        return output;
    }

    private static void ripemd160into(final byte[] bytes, final byte[] target, final int offset) {
        final RIPEMD160Digest digest = new RIPEMD160Digest();
        digest.update(bytes, 0, bytes.length);
        digest.doFinal(target, offset);
    }
}