package org.elastos.did.util;

import java.util.Arrays;

import org.spongycastle.crypto.BufferedBlockCipher;
import org.spongycastle.crypto.CryptoException;
import org.spongycastle.crypto.digests.MD5Digest;
import org.spongycastle.crypto.engines.AESEngine;
import org.spongycastle.crypto.modes.CBCBlockCipher;
import org.spongycastle.crypto.paddings.PaddedBufferedBlockCipher;
import org.spongycastle.crypto.params.KeyParameter;
import org.spongycastle.crypto.params.ParametersWithIV;

public class Aes256cbc {
	private static void generatrKeyAndIv(String passwd, byte[] key, byte[] iv) {
		byte[] pass = passwd.getBytes();

		// Create key from passwd
		MD5Digest md = new MD5Digest();
		md.update(pass, 0, pass.length);
		md.doFinal(key, 0);

		md.reset();
		md.update(key, 0, 16);
		md.update(pass, 0, pass.length);
		md.doFinal(key, 16);

		// Create iv from passwd
		md.reset();
		md.update(key, 16, 16);
		md.update(pass, 0, pass.length);
		md.doFinal(iv, 0);
	}

	public static byte[] encrypt(String passwd, byte[] plain, int offset,
			int length) throws CryptoException {
		byte[] key = new byte[32];
		byte[] iv = new byte[16];

		generatrKeyAndIv(passwd, key, iv);

		KeyParameter keyParam = new KeyParameter(key);
		ParametersWithIV keyWithIv = new ParametersWithIV(keyParam, iv);

		BufferedBlockCipher cipher = new PaddedBufferedBlockCipher(
					new CBCBlockCipher(new AESEngine()));
        cipher.init(true, keyWithIv);

        byte[] secret = new byte[cipher.getOutputSize(length)];
        int len = cipher.processBytes(plain, offset, length, secret, 0);
		len += cipher.doFinal(secret, len);

		if (len < secret.length)
        	plain = Arrays.copyOf(secret, len);

		return secret;
	}

	public static byte[] encrypt(String passwd, byte[] plain, int offset)
			throws CryptoException {
		return encrypt(passwd, plain, offset, plain.length - offset);
	}

	public static byte[] encrypt(String passwd, byte[] plain)
			throws CryptoException {
		return encrypt(passwd, plain, 0, plain.length);
	}

	public static byte[] decrypt(String passwd, byte[] secret, int offset,
			int length) throws CryptoException {
		byte[] key = new byte[32];
		byte[] iv = new byte[16];

		generatrKeyAndIv(passwd, key, iv);

		KeyParameter keyParam = new KeyParameter(key);
		ParametersWithIV keyWithIv = new ParametersWithIV(keyParam, iv);

        BufferedBlockCipher cipher = new PaddedBufferedBlockCipher(
        		new CBCBlockCipher(new AESEngine()));
        cipher.init(false, keyWithIv);

        byte[] plain = new byte[cipher.getOutputSize(length)];
        int len = cipher.processBytes(secret, offset, length, plain, 0);
        len += cipher.doFinal(plain, len);

        if (len < plain.length)
        	plain = Arrays.copyOf(plain, len);

        return plain;
	}

	public static byte[] decrypt(String passwd, byte[] secret, int offset)
			throws CryptoException {
		return decrypt(passwd, secret, offset, secret.length - offset);
	}

	public static byte[] decrypt(String passwd, byte[] secret)
			throws CryptoException {
		return decrypt(passwd, secret, 0, secret.length);
	}

	public static String encryptToBase64(String passwd, byte[] plain,
			int offset, int length) throws CryptoException {
		byte[] secret = encrypt(passwd, plain, offset, length);

		return Base64.encodeToString(secret,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
	}

	public static String encryptToBase64(String passwd, byte[] plain, int offset)
			throws CryptoException {
		return encryptToBase64(passwd, plain, offset, plain.length - offset);
	}

	public static String encryptToBase64(String passwd, byte[] plain)
			throws CryptoException {
		return encryptToBase64(passwd, plain, 0, plain.length);
	}

	public static byte[] decryptFromBase64(String passwd, String secret)
			throws CryptoException {
		byte[] secretBytes =   Base64.decode(secret,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);

		return decrypt(passwd, secretBytes);
	}
}
