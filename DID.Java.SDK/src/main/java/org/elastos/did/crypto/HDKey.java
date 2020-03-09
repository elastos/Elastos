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
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.spec.ECParameterSpec;
import java.security.spec.ECPrivateKeySpec;
import java.security.spec.ECPublicKeySpec;
import java.security.spec.InvalidKeySpecException;
import java.util.ArrayList;

import org.bitcoinj.core.ECKey.ECDSASignature;
import org.bitcoinj.core.Sha256Hash;
import org.bitcoinj.core.SignatureDecodeException;
import org.bitcoinj.crypto.ChildNumber;
import org.bitcoinj.crypto.DeterministicHierarchy;
import org.bitcoinj.crypto.DeterministicKey;
import org.bitcoinj.crypto.HDKeyDerivation;
import org.bitcoinj.params.MainNetParams;
import org.elastos.did.Mnemonic;
import org.elastos.did.exception.InvalidKeyException;
import org.spongycastle.crypto.digests.RIPEMD160Digest;
import org.spongycastle.crypto.digests.SHA256Digest;
import org.spongycastle.crypto.params.ECPublicKeyParameters;
import org.spongycastle.jce.spec.ECNamedCurveSpec;

public class HDKey {
	public static final int PUBLICKEY_BYTES = 33;
	public static final int PRIVATEKEY_BYTES = 32;
	public static final int SEED_BYTES = 64;
	public static final int EXTENDED_PRIVATE_BYTES = 82;

    private final static byte PADDING_IDENTITY	= 0x67;
    private final static byte PADDING_STANDARD	= (byte)0xAD;

	private DeterministicHierarchy dh;

	private static final ArrayList<ChildNumber> derivePath;

	static {
		// m/44'/0'/0'/0
		derivePath = new ArrayList<ChildNumber>(4);
		derivePath.add(new ChildNumber(44, true));
		derivePath.add(new ChildNumber(0, true));
		derivePath.add(new ChildNumber(0, true));
		derivePath.add(new ChildNumber(0, false));
	}

	private HDKey(DeterministicKey rootKey) {
		dh = new DeterministicHierarchy(rootKey);
	}

	public static HDKey fromMnemonic(String mnemonic, String passphrase) {
		byte[] seed = Mnemonic.toSeed(mnemonic, passphrase);

		return fromSeed(seed);
	}

	public static HDKey fromSeed(byte[] seed) {
		DeterministicKey key = HDKeyDerivation.createMasterPrivateKey(seed);
		return new HDKey(key);
	}

	public byte[] getKeyBytes() {
		return dh.getRootKey().getPrivKeyBytes();
	}

	public byte[] serialize() {
		return Base58.decode(serializePrivBase58());
	}

	public String serializePrivBase58() {
		return dh.getRootKey().serializePrivB58(MainNetParams.get());
	}

	public String serializePubBase58() {
		//return dh.getRootKey().serializePubB58(MainNetParams.get());

		DeterministicKey child = dh.get(derivePath, false, true);
		return child.serializePubB58(MainNetParams.get());
	}

	public static HDKey deserialize(byte[] keyData) {
		return deserializeBase58(Base58.encode(keyData));
	}

	public static HDKey deserializeBase58(String keyData) {
		DeterministicKey rootKey = DeterministicKey.deserializeB58(
				keyData, MainNetParams.get());
		return new HDKey(rootKey);
	}

	public DerivedKey derive(int index) {
		ChildNumber number = new ChildNumber(index, false);
		DeterministicKey child;

		if (dh.getRootKey().isPubKeyOnly())
			child = HDKeyDerivation.deriveChildKey(dh.getRootKey(), number);
		else
			child = dh.deriveChild(derivePath, false, true, number);

		return new DerivedKey(child);
	}

	public void wipe() {
		// TODO:
	}

	public static class DerivedKey {
		private DeterministicKey key;

		private DerivedKey(DeterministicKey key) {
			this.key = key;
		}

		public static DerivedKey deserialize(byte[] privateKeyBytes) {
			byte[] extendedPrivateKeyBytes = new byte[78];

			int version = MainNetParams.get().getBip32HeaderP2PKHpriv();
			extendedPrivateKeyBytes[0] = (byte)((version >> 24) & 0xFF);
			extendedPrivateKeyBytes[1] = (byte)((version >> 16) & 0xFF);
			extendedPrivateKeyBytes[2] = (byte)((version >> 8) & 0xFF);
			extendedPrivateKeyBytes[3] = (byte)(version & 0xFF);

			System.arraycopy(privateKeyBytes, 0,
					extendedPrivateKeyBytes, 46, 32);

			// byte[] hash = Sha256Hash.hashTwice(extendedPrivateKeyBytes, 0, 78);
			// System.arraycopy(hash, 0, extendedPrivateKeyBytes, 78, 4);

			DeterministicKey key = DeterministicKey.deserialize(
					MainNetParams.get(), extendedPrivateKeyBytes);

			return new DerivedKey(key);
		}

		public byte[] serialize() {
			return key.getPrivKeyBytes();
		}

		public byte[] getPublicKeyBytes() {
			return key.getPubKey();
		}

		public String getPublicKeyBase58() {
			return Base58.encode(getPublicKeyBytes());
		}

		public String getPublicKeyBase64() {
			return Base64.encodeToString(getPublicKeyBytes(),
					Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		}

		public byte[] getPrivateKeyBytes() {
			return key.getPrivKeyBytes();
		}

		public String getPrivateKeyBase58() {
			return Base58.encode(getPrivateKeyBytes());
		}

		public String getPrivateKeyBase64() {
			return Base64.encodeToString(getPrivateKeyBytes(),
					Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
		}

	    public static KeyPair getKeyPair(byte[] publicKey, byte[] privateKey)
	    		throws InvalidKeyException {
	    	if (publicKey == null && privateKey == null)
	    		throw new IllegalArgumentException();

	    	ECParameterSpec paramSpec = new ECNamedCurveSpec(
	        		"secp256r1", CURVE_PARAMS.getCurve(), CURVE_PARAMS.getG(),
	        		CURVE_PARAMS.getN(), CURVE_PARAMS.getH());

	        KeyFactory keyFactory = null;
			try {
				keyFactory = KeyFactory.getInstance("EC");
			} catch (NoSuchAlgorithmException ignore) {
				// never happen
			}

	    	PublicKey pub = null;
	    	PrivateKey priv = null;

	    	try {
		    	if (publicKey != null) {
		    		ECPublicKeyParameters pubParams = new ECPublicKeyParameters(
		    				CURVE_PARAMS.getCurve().decodePoint(publicKey), CURVE);
		    		ECPublicKeySpec pubSpec = new ECPublicKeySpec(new java.security.spec.ECPoint(
		    				pubParams.getQ().getXCoord().toBigInteger(),
		    				pubParams.getQ().getYCoord().toBigInteger()), paramSpec);
		    		pub = keyFactory.generatePublic(pubSpec);
		    	}

		    	if (privateKey != null) {
		    		BigInteger keyInt = new BigInteger(1, privateKey);
		    		ECPrivateKeySpec privSpec = new ECPrivateKeySpec(keyInt, paramSpec);
		    		priv = keyFactory.generatePrivate(privSpec);
		    	}
			} catch (InvalidKeySpecException e) {
				throw new InvalidKeyException();
			}
	    	return new KeyPair(pub, priv);
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

	        hash = Sha256Hash.hashTwice(programHash);
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
			// TODO:
		}

		public static boolean isAddressValid(String address) {
			byte[] binAddress = Base58.decode(address);

			if (binAddress.length != 25)
				return false;

			if (binAddress[0] != PADDING_IDENTITY)
				return false;

			byte[] hash = Sha256Hash.hashTwice(binAddress, 0, 21);

			return (hash[0] == binAddress[21] && hash[1] == binAddress[22]
					&& hash[2] == binAddress[23] && hash[3] == binAddress[24]);
		}

		public byte[] sign(byte[] sha256Hash) {
			ECDSASignature sig = key.sign(Sha256Hash.wrap(sha256Hash));
			return sig.encodeToDER();
		}

		public byte[] signData(byte[] ... inputs) {
			byte[] hash = sha256Digest(inputs);

			return sign(hash);
		}

		public boolean verify(byte[] sha256Hash, byte[] signature) {
			try {
				return key.verify(sha256Hash, signature);
			} catch (SignatureDecodeException e) {
				return false;
			}
		}

		public boolean verifyData(byte[] signature, byte[] ... inputs) {
			byte[] hash = sha256Digest(inputs);

			return verify(hash, signature);
		}

		private static byte[] sha256Digest(byte[] ... inputs) {
			byte digest[] = new byte[32];

			SHA256Digest sha256 = new SHA256Digest();

			for (byte[] input : inputs)
				sha256.update(input, 0, input.length);

			sha256.doFinal(digest, 0);

			return digest;
		}
	}
}
