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

package org.elastos.did;

import java.io.IOException;
import java.io.InputStream;
import java.security.SecureRandom;
import java.text.Normalizer;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import org.bitcoinj.crypto.MnemonicCode;
import org.bitcoinj.crypto.MnemonicException;
import org.elastos.did.exception.DIDException;

public class Mnemonic {
	public static final String DEFAULT = null;
	public static final String CHINESE_SIMPLIFIED = "chinese_simplified";
	public static final String CHINESE_TRADITIONAL = "chinese_traditional";
	public static final String CZECH = "czech";
	public static final String ENGLISH = "english";
	public static final String FRENCH = "french";
	public static final String ITALIAN = "italian";
	public static final String JAPANESE = "japanese";
	public static final String KOREAN = "korean";
	public static final String SPANISH = "spanish";

	private static final int TWELVE_WORDS_ENTROPY = 16;

	private MnemonicCode mc;

	private static HashMap<String, Mnemonic> mcTable = new HashMap<String, Mnemonic>(4);

	private Mnemonic(MnemonicCode mc) {
		this.mc = mc;
	}

	public static Mnemonic getInstance() {
		String language = "";

		if (mcTable.containsKey(language))
			return mcTable.get(language);

		Mnemonic m = new Mnemonic(MnemonicCode.INSTANCE);
		mcTable.put(language, m);
		return m;
	}

	public static Mnemonic getInstance(String language) throws DIDException {
		if (language == null || language.isEmpty())
			return getInstance();

		if (mcTable.containsKey(language))
			return mcTable.get(language);

		try {
			InputStream is = MnemonicCode.openDefaultWords(language);
			MnemonicCode mc = new MnemonicCode(is, null);
			Mnemonic m = new Mnemonic(mc);
			mcTable.put(language, m);
			return m;
		} catch (IOException | IllegalArgumentException e) {
			throw new DIDException(e);
		}
	}

	public String generate() throws DIDException {
		try {
			byte[] entropy = new byte[TWELVE_WORDS_ENTROPY];
			new SecureRandom().nextBytes(entropy);
			List<String> words = mc.toMnemonic(entropy);
			return String.join(" ", words);
		} catch (MnemonicException e) {
			throw new DIDException(e);
		}
	}

	public boolean isValid(String mnemonic) {
    	if (mnemonic == null || mnemonic.isEmpty())
    		throw new IllegalArgumentException();

    	mnemonic = Normalizer.normalize(mnemonic, Normalizer.Form.NFD);
		List<String> words = Arrays.asList(mnemonic.split(" "));

    	try {
	    	mc.check(words);
		    return true;
		} catch (MnemonicException e) {
			return false;
		}
	}

	public static byte[] toSeed(String mnemonic, String passphrase) {
    	mnemonic = Normalizer.normalize(mnemonic, Normalizer.Form.NFD);
    	passphrase = Normalizer.normalize(passphrase, Normalizer.Form.NFD);

		List<String> words = Arrays.asList(mnemonic.split(" "));

    	return MnemonicCode.toSeed(words, passphrase);
	}
}
