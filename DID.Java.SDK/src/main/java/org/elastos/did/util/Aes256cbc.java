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

import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

public class Aes256cbc {
	private static void generatrKeyAndIv(String passwd, byte[] key, byte[] iv)
			throws NoSuchAlgorithmException {
		byte[] pass = passwd.getBytes();

		// Create key from passwd
		MessageDigest md = MessageDigest.getInstance("MD5");
		byte[] hash = md.digest(pass);
		System.arraycopy(hash, 0, key, 0, 16);

		md.reset();
		md.update(hash);
		md.update(pass);
		hash = md.digest();
		System.arraycopy(hash, 0, key, 16, 16);

		// Create iv from passwd
		md.reset();
		md.update(hash);
		md.update(pass);
		hash = md.digest();
		System.arraycopy(hash, 0, iv, 0, 16);
	}

	public static byte[] encrypt(String passwd, byte[] plain, int offset,
			int length) throws GeneralSecurityException {
		byte[] key = new byte[32];
		byte[] iv = new byte[16];

		generatrKeyAndIv(passwd, key, iv);

		SecretKeySpec keySpec = new SecretKeySpec(key, "AES");
		IvParameterSpec ivSpec = new IvParameterSpec(iv);

		Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
		cipher.init(Cipher.ENCRYPT_MODE, keySpec, ivSpec);
		byte[] secret = cipher.doFinal(plain, offset, length);

		return secret;
	}

	public static byte[] encrypt(String passwd, byte[] plain, int offset)
			throws GeneralSecurityException {
		return encrypt(passwd, plain, offset, plain.length - offset);
	}

	public static byte[] encrypt(String passwd, byte[] plain)
			throws GeneralSecurityException {
		return encrypt(passwd, plain, 0, plain.length);
	}

	public static byte[] decrypt(String passwd, byte[] secret, int offset,
			int length) throws GeneralSecurityException {
		byte[] key = new byte[32];
		byte[] iv = new byte[16];

		generatrKeyAndIv(passwd, key, iv);

		SecretKeySpec keySpec = new SecretKeySpec(key, "AES");
		IvParameterSpec ivSpec = new IvParameterSpec(iv);

		Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
		cipher.init(Cipher.DECRYPT_MODE, keySpec, ivSpec);
		byte[] plain = cipher.doFinal(secret, offset, length);

		return plain;
	}

	public static byte[] decrypt(String passwd, byte[] secret, int offset)
			throws GeneralSecurityException {
		return decrypt(passwd, secret, offset, secret.length - offset);
	}

	public static byte[] decrypt(String passwd, byte[] secret)
			throws GeneralSecurityException {
		return decrypt(passwd, secret, 0, secret.length);
	}

	public static String encryptToBase64(String passwd, byte[] plain,
			int offset, int length) throws GeneralSecurityException {
		byte[] secret = encrypt(passwd, plain, offset, length);

		return Base64.encodeToString(secret,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
	}

	public static String encryptToBase64(String passwd, byte[] plain, int offset)
			throws GeneralSecurityException {
		return encryptToBase64(passwd, plain, offset, plain.length - offset);
	}

	public static String encryptToBase64(String passwd, byte[] plain)
			throws GeneralSecurityException {
		return encryptToBase64(passwd, plain, 0, plain.length);
	}

	public static byte[] decryptFromBase64(String passwd, String secret)
			throws GeneralSecurityException {
		byte[] secretBytes =   Base64.decode(secret,
				Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);

		return decrypt(passwd, secretBytes);
	}
}
