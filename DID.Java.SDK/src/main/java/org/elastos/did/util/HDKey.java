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

package org.elastos.did.util;

import java.math.BigInteger;
import java.util.Arrays;

import org.spongycastle.asn1.x9.X9ECParameters;
import org.spongycastle.crypto.digests.RIPEMD160Digest;
import org.spongycastle.crypto.digests.SHA256Digest;
import org.spongycastle.crypto.ec.CustomNamedCurves;
import org.spongycastle.crypto.params.ECDomainParameters;
import org.spongycastle.math.ec.ECPoint;
import org.spongycastle.math.ec.FixedPointCombMultiplier;
import org.spongycastle.math.ec.FixedPointUtil;

import io.github.novacrypto.bip32.ExtendedPrivateKey;
import io.github.novacrypto.bip39.SeedCalculator;
import io.github.novacrypto.bip44.AddressIndex;
import io.github.novacrypto.bip44.BIP44;
import io.github.novacrypto.hashing.Sha256;

public class HDKey {
	public static final int PUBLICKEY_BYTES = 33;
	public static final int PRIVATEKEY_BYTES = 32;

	protected static final String CURVE_ALGORITHM = "secp256r1";
	protected static X9ECParameters CURVE_PARAMS;
	protected static ECDomainParameters CURVE;

    private final static byte PADDING_IDENTITY	= 0x67;
    private final static byte PADDING_STANDARD	= (byte)0xAD;

    private byte[] seed;
	private ExtendedPrivateKey rootPrivateKey;

	static {
		CURVE_PARAMS = CustomNamedCurves.getByName(CURVE_ALGORITHM);
		FixedPointUtil.precompute(CURVE_PARAMS.getG(), 12);
		CURVE = new ECDomainParameters(CURVE_PARAMS.getCurve(),
				CURVE_PARAMS.getG(),
				CURVE_PARAMS.getN(),
				CURVE_PARAMS.getH());
	}

	private HDKey(byte[] seed) {
		this.seed = seed;
		rootPrivateKey = ExtendedPrivateKey.fromSeed(seed, null);
	}

	public static HDKey fromMnemonic(String mnemonic, String passphrase) {
		byte[] seed = new SeedCalculator()
				// .withWordsFromWordList(English.INSTANCE)
				.calculateSeed(mnemonic, passphrase);

		return new HDKey(seed);
	}

	public static HDKey fromSeed(byte[] seed) {
		return new HDKey(seed);
	}

	public byte[] getSeed() {
		return seed;
	}

	public byte[] getKeyBytes() {
		return rootPrivateKey.extendedKeyByteArray();
	}

	public DerivedKey derive(int index) {
		AddressIndex addressIndex = BIP44.m()
				.purpose44()
				.coinType(0)
				.account(0)
				.external()
				.address(index);

		ExtendedPrivateKey child = rootPrivateKey.derive(addressIndex,
				AddressIndex.DERIVATION);

		return new DerivedKey(child);
	}

	public void wipe() {
		Arrays.fill(seed, (byte)0);
		Arrays.fill(rootPrivateKey.getKeyBytes(), (byte)0);
	}

	public static class DerivedKey {
		private ExtendedPrivateKey privateKey;
		private ECPoint publicKeyPoint;

		private DerivedKey(ExtendedPrivateKey privateKey) {
			this.privateKey = privateKey;
		}

		private ECPoint getPublicKeyPoint() {
			if (publicKeyPoint == null) {
				BigInteger key = new BigInteger(1, privateKey.getKeyBytes());
				if (key.bitLength() > CURVE.getN().bitLength()) {
					key = key.mod(CURVE.getN());
				}

				publicKeyPoint = new FixedPointCombMultiplier().multiply(
						CURVE.getG(), key);
			}

			return publicKeyPoint;
		}

		public static DerivedKey deserialize(byte[] privateKeyBytes) {
			byte[] extendedPrivateKeyBytes = new byte[82];
			System.arraycopy(privateKeyBytes, 0,
					extendedPrivateKeyBytes, 46, 32);

			byte[] hash = Sha256.sha256Twice(extendedPrivateKeyBytes, 0, 78);
			System.arraycopy(hash, 0, extendedPrivateKeyBytes, 78, 4);

			ExtendedPrivateKey privateKey = ExtendedPrivateKey
					.deserializer().deserialize(extendedPrivateKeyBytes);

			return new DerivedKey(privateKey);
		}

		public byte[] serialize() {
			return privateKey.getKeyBytes();
		}

		public byte[] getPublicKeyBytes() {
			return getPublicKeyPoint().getEncoded(true);
		}

		public String getPublicKeyBase58() {
			return Base58.encode(getPublicKeyBytes());
		}

		public String getPublicKeyBase64() {
			return Base64.encodeToString(getPublicKeyBytes(),
					Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		}

		public byte[] getPrivateKeyBytes() {
			return privateKey.getKeyBytes();
		}

		public String getPrivateKeyBase58() {
			return Base58.encode(getPrivateKeyBytes());
		}

		public String getPrivateKeyBase64() {
			return Base64.encodeToString(getPrivateKeyBytes(),
					Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		}

		private static byte[] getRedeemScript(byte[] pk) {
	        byte[] script = new byte[35];
	        script[0] = 33;
	        System.arraycopy(pk, 0, script, 1, 33);
	        script[34] = PADDING_STANDARD;
	        return script;
		}

		private static byte[] sha256Ripemd160(byte[] input) {
	        byte[] sha256 = new byte[32];

	        SHA256Digest sha256Digest = new SHA256Digest();
	        sha256Digest.update(input, 0, input.length);
	        sha256Digest.doFinal(sha256, 0);

	        RIPEMD160Digest digest = new RIPEMD160Digest();
	        digest.update(sha256, 0, sha256.length);
	        byte[] out = new byte[20];
	        digest.doFinal(out, 0);
	        return out;
	    }

		private static byte[] getBinAddress(byte[] pk) {
	        byte[] script = getRedeemScript(pk);

	        byte[] hash = sha256Ripemd160(script);
	        byte[] programHash = new byte[hash.length + 1];
	        programHash[0] = PADDING_IDENTITY;
	        System.arraycopy(hash, 0, programHash, 1, hash.length);

	        hash = Sha256.sha256Twice(programHash);
	        byte[] binAddress = new byte[programHash.length+4];
	        System.arraycopy(programHash, 0, binAddress, 0, programHash.length);
	        System.arraycopy(hash, 0, binAddress, programHash.length, 4);

	        return binAddress;
		}

		public byte[] getBinAddress() {
			return getBinAddress(getPublicKeyBytes());
		}

		public String getAddress() {
	        return Base58.encode(getBinAddress());
		}

		public static String getAddress(byte[] pk) {
			return Base58.encode(getBinAddress(pk));
		}

		public void wipe() {
			byte[] keyBytes = privateKey.getKeyBytes();
			Arrays.fill(keyBytes, (byte)0);
		}

		public static boolean isAddressValid(String address) {
			byte[] binAddress = Base58.decode(address);

			if (binAddress.length != 25)
				return false;

			if (binAddress[0] != PADDING_IDENTITY)
				return false;

			byte[] hash = Sha256.sha256Twice(binAddress, 0, 21);

			return (hash[0] == binAddress[21] && hash[1] == binAddress[22]
					&& hash[2] == binAddress[23] && hash[3] == binAddress[24]);
		}
	}
}
