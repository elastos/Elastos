/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.did.crypto;

import static org.bitcoinj.core.ECKey.CURVE;
import static org.bitcoinj.core.ECKey.CURVE_PARAMS;

import java.math.BigInteger;

import org.spongycastle.crypto.digests.SHA256Digest;
import org.spongycastle.crypto.params.ECPrivateKeyParameters;
import org.spongycastle.crypto.params.ECPublicKeyParameters;
import org.spongycastle.crypto.signers.ECDSASigner;
import org.spongycastle.crypto.signers.RandomDSAKCalculator;

public class EcdsaSigner {
	public static byte[] sign(byte[] privateKey, byte[] ... inputs) {
		BigInteger keyInt = new BigInteger(1, privateKey);

		ECPrivateKeyParameters keyParams = new ECPrivateKeyParameters(
				keyInt, CURVE);

		ECDSASigner signer = new ECDSASigner(
				new RandomDSAKCalculator());
		signer.init(true, keyParams);

		BigInteger[] rs = signer.generateSignature(sha256Digest(inputs));

		byte[] r = bigIntegerToBytes(rs[0], 32);
		byte[] s = bigIntegerToBytes(rs[1], 32);

		byte[] sig = new byte[r.length + s.length];
		System.arraycopy(r, 0, sig, 0, r.length);
		System.arraycopy(s, 0, sig, r.length, s.length);

		return sig;
	}

	public static boolean verify(byte[] publicKey, byte[] sig, byte[] ... inputs) {
		if (sig.length != 64) {
			return false;
		}

		ECPublicKeyParameters keyParams = new ECPublicKeyParameters(
				CURVE_PARAMS.getCurve().decodePoint(publicKey), CURVE);

		ECDSASigner signer = new ECDSASigner(
				new RandomDSAKCalculator());
		signer.init(false, keyParams);

		byte rb[] = new byte[sig.length / 2];
		byte sb[] = new byte[sig.length / 2];
		System.arraycopy(sig, 0, rb, 0, rb.length);
		System.arraycopy(sig, sb.length, sb, 0, sb.length);
		BigInteger r = parseBigIntegerPositive(new BigInteger(rb), rb.length * 8);
		BigInteger s = parseBigIntegerPositive(new BigInteger(sb), rb.length * 8);

		return signer.verifySignature(sha256Digest(inputs), r, s);
	}

	private static byte[] sha256Digest(byte[] ... inputs) {
		byte digest[] = new byte[32];

		SHA256Digest sha256 = new SHA256Digest();

		for (byte[] input : inputs)
			sha256.update(input, 0, input.length);

		sha256.doFinal(digest, 0);

		return digest;
	}

	private static BigInteger parseBigIntegerPositive(BigInteger b, int bitlen) {
		if (b.compareTo(BigInteger.ZERO) < 0)
			b = b.add(BigInteger.ONE.shiftLeft(bitlen));
		return b;
	}

	private static byte[] bigIntegerToBytes(BigInteger value, int bytes) {
		byte[] src = value.toByteArray();
		boolean signByte = src[0] == 0;

		int length = signByte ? src.length - 1 : src.length;
		if (length > bytes)
			throw new IllegalArgumentException(
					"Excepted length is samll than BigInteger.");

		byte[] dest = new byte[bytes];
		int srcPos = signByte ? 1 : 0;
		int destPos = bytes - length;
		System.arraycopy(src, srcPos, dest, destPos, length);

		return dest;
	}
}
