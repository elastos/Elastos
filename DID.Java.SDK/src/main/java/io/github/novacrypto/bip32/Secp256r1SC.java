/*
 *  BIP32 library, a Java implementation of BIP32
 *  Copyright (C) 2017-2018 Alan Evans, NovaCrypto
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

import org.spongycastle.asn1.x9.X9ECParameters;
import org.spongycastle.crypto.ec.CustomNamedCurves;
import org.spongycastle.math.ec.ECPoint;

import java.math.BigInteger;

final class Secp256r1SC {

    static final X9ECParameters CURVE = CustomNamedCurves.getByName("secp256r1");

    static BigInteger n() {
        return CURVE.getN();
    }

    static byte[] pointSerP(final ECPoint point) {
        return point.getEncoded(true);
    }

    static byte[] pointSerP_gMultiply(final BigInteger p) {
        return pointSerP(gMultiply(p));
    }

    static ECPoint gMultiplyAndAddPoint(final BigInteger p, final byte[] toAdd) {
        return gMultiply(p).add(decode(toAdd));
    }

    private static ECPoint decode(final byte[] toAdd) {
        return CURVE.getCurve().decodePoint(toAdd);
    }

    private static ECPoint gMultiply(BigInteger p) {
        return CURVE.getG()
                .multiply(p);
    }
}