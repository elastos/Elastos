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

	public static byte[] encrypt(byte[] plain, int offset, int length,
			String passwd) throws CryptoException {
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

	public static byte[] encrypt(byte[] plain, int offset, String passwd)
			throws CryptoException {
		return encrypt(plain, offset, plain.length - offset, passwd);
	}

	public static byte[] encrypt(byte[] plain, String passwd)
			throws CryptoException {
		return encrypt(plain, 0, plain.length, passwd);
	}

	public static byte[] decrypt(byte[] secret, int offset, int length,
			String passwd) throws CryptoException {
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

	public static byte[] decrypt(byte[] secret, int offset, String passwd)
			throws CryptoException {
		return decrypt(secret, offset, secret.length - offset, passwd);
	}

	public static byte[] decrypt(byte[] secret, String passwd)
			throws CryptoException {
		return decrypt(secret, 0, secret.length, passwd);
	}

	public static String encryptToBase64(byte[] plain, int offset, int length,
			String passwd) throws CryptoException {
		byte[] secret = encrypt(plain, offset, length, passwd);

		return Base64.encodeToString(secret,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
	}

	public static String encryptToBase64(byte[] plain, int offset, String passwd)
			throws CryptoException {
		return encryptToBase64(plain, offset, plain.length - offset, passwd);
	}

	public static String encryptToBase64(byte[] plain, String passwd)
			throws CryptoException {
		return encryptToBase64(plain, 0, plain.length, passwd);
	}

	public static byte[] decryptFromBase64(String secret, String passwd)
			throws CryptoException {
		byte[] secretBytes =   Base64.decode(secret,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);

		return decrypt(secretBytes, passwd);
	}
}
