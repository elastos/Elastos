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

import org.bitcoinj.core.ECKey.ECDSASignature;
import org.bitcoinj.core.Sha256Hash;
import org.bitcoinj.core.SignatureDecodeException;
import org.bitcoinj.crypto.ChildNumber;
import org.bitcoinj.crypto.DeterministicKey;
import org.bitcoinj.crypto.HDKeyDerivation;
import org.bitcoinj.crypto.HDPath;
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
	public static final int EXTENDED_KEY_BYTES = 82;
	public static final int EXTENDED_PRIVATEKEY_BYTES = EXTENDED_KEY_BYTES;
	public static final int EXTENDED_PUBLICKEY_BYTES = EXTENDED_KEY_BYTES;


    private final static byte PADDING_IDENTITY	= 0x67;
    private final static byte PADDING_STANDARD	= (byte)0xAD;

    private DeterministicKey key;

	// Derive path: m/44'/0'/0'/0/index
	public static final String DERIVE_PATH_PREFIX = "44H/0H/0H/0/";

	// Pre-derive publickey path: m/44'/0'/0'
	public static final String PRE_DERIVED_PUBLICKEY_PATH = "44H/0H/0H";

	private HDKey(DeterministicKey key) {
		this.key = key;
	}

	public HDKey(String mnemonic, String passphrase) {
		this(Mnemonic.toSeed(mnemonic, passphrase));
	}

	public HDKey(byte[] seed) {
		this(HDKeyDerivation.createMasterPrivateKey(seed));
	}

	public byte[] getPrivateKeyBytes() {
		return key.getPrivKeyBytes();
	}

	public String getPrivateKeyBase58() {
		return Base58.encode(getPrivateKeyBytes());
	}

	public byte[] getPublicKeyBytes() {
		return key.getPubKey();
	}

	public String getPublicKeyBase58() {
		return Base58.encode(getPublicKeyBytes());
	}

	public byte[] serialize() {
		return Base58.decode(serializeBase58());
	}

	public String serializeBase58() {
		return key.serializePrivB58(MainNetParams.get());
	}

	public byte[] serializePublicKey() {
		return Base58.decode(serializePublicKeyBase58());
	}

    public String serializePublicKeyBase58() {
    	return key.serializePubB58(MainNetParams.get());
    }

	public static HDKey deserialize(byte[] keyData) {
		/*
		DeterministicKey k = DeterministicKey.deserialize(
				MainNetParams.get(), keyData);
		return new HDKey(k);
		*/
		return deserializeBase58(Base58.encode(keyData));
	}

	public static HDKey deserializeBase58(String keyData) {
		DeterministicKey k = DeterministicKey.deserializeB58(
				keyData, MainNetParams.get());
		return new HDKey(k);
	}

	public static byte[] paddingToExtendedPrivateKey(byte[] privateKeyBytes) {
		byte[] extendedPrivateKeyBytes = new byte[EXTENDED_PRIVATEKEY_BYTES];

		int version = MainNetParams.get().getBip32HeaderP2PKHpriv();
		extendedPrivateKeyBytes[0] = (byte)((version >> 24) & 0xFF);
		extendedPrivateKeyBytes[1] = (byte)((version >> 16) & 0xFF);
		extendedPrivateKeyBytes[2] = (byte)((version >> 8) & 0xFF);
		extendedPrivateKeyBytes[3] = (byte)(version & 0xFF);

		System.arraycopy(privateKeyBytes, 0,
				extendedPrivateKeyBytes, 46, 32);

		byte[] hash = Sha256Hash.hashTwice(extendedPrivateKeyBytes, 0, 78);
		System.arraycopy(hash, 0, extendedPrivateKeyBytes, 78, 4);

		return extendedPrivateKeyBytes;
	}

	public static byte[] paddingToExtendedPublicKey(byte[] publicKeyBytes) {
		byte[] extendedPublicKeyBytes = new byte[EXTENDED_PUBLICKEY_BYTES];

		int version = MainNetParams.get().getBip32HeaderP2PKHpub();
		extendedPublicKeyBytes[0] = (byte)((version >> 24) & 0xFF);
		extendedPublicKeyBytes[1] = (byte)((version >> 16) & 0xFF);
		extendedPublicKeyBytes[2] = (byte)((version >> 8) & 0xFF);
		extendedPublicKeyBytes[3] = (byte)(version & 0xFF);

		System.arraycopy(publicKeyBytes, 0,
				extendedPublicKeyBytes, 45, 33);

		byte[] hash = Sha256Hash.hashTwice(extendedPublicKeyBytes, 0, 78);
		System.arraycopy(hash, 0, extendedPublicKeyBytes, 78, 4);

		return extendedPublicKeyBytes;
	}

	public HDKey derive(String path) {
		HDPath derivePath = HDPath.parsePath(path);

		DeterministicKey child = key;
		for (ChildNumber childNumber: derivePath)
			child = HDKeyDerivation.deriveChildKey(child, childNumber);

		return new HDKey(child);
	}

	public HDKey derive(int index, boolean hardened) {
		ChildNumber childNumber = new ChildNumber(index, hardened);
		return new HDKey(HDKeyDerivation.deriveChildKey(key, childNumber));
	}

	public HDKey derive(int index) {
		return derive(index, false);
	}

    public KeyPair getJCEKeyPair() throws InvalidKeyException {
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
    		ECPublicKeyParameters pubParams = new ECPublicKeyParameters(
    				CURVE_PARAMS.getCurve().decodePoint(getPublicKeyBytes()), CURVE);
    		ECPublicKeySpec pubSpec = new ECPublicKeySpec(new java.security.spec.ECPoint(
    				pubParams.getQ().getXCoord().toBigInteger(),
    				pubParams.getQ().getYCoord().toBigInteger()), paramSpec);
    		pub = keyFactory.generatePublic(pubSpec);

	    	if (key.hasPrivKey()) {
	    		BigInteger keyInt = new BigInteger(1, getPrivateKeyBytes());
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

	public static String toAddress(byte[] pk) {
		return Base58.encode(getBinAddress(pk));
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

	public void wipe() {
		// TODO:
	}
}
