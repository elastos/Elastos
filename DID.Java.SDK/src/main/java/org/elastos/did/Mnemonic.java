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
import java.security.SecureRandom;

import org.elastos.did.exception.DIDException;
import org.elastos.did.wordlists.UserDefinedWordLists;

import io.github.novacrypto.bip39.MnemonicGenerator;
import io.github.novacrypto.bip39.MnemonicValidator;
import io.github.novacrypto.bip39.WordList;
import io.github.novacrypto.bip39.Words;
import io.github.novacrypto.bip39.Validation.InvalidChecksumException;
import io.github.novacrypto.bip39.Validation.InvalidWordCountException;
import io.github.novacrypto.bip39.Validation.UnexpectedWhiteSpaceException;
import io.github.novacrypto.bip39.Validation.WordNotFoundException;
import io.github.novacrypto.bip39.wordlists.ChineseSimplified;
import io.github.novacrypto.bip39.wordlists.ChineseTraditional;
import io.github.novacrypto.bip39.wordlists.English;
import io.github.novacrypto.bip39.wordlists.French;
import io.github.novacrypto.bip39.wordlists.Japanese;
import io.github.novacrypto.bip39.wordlists.Spanish;

public class Mnemonic {
	public static final int ENGLISH = 0;
	public static final int FRENCH = 1;
	public static final int SPANISH = 2;
	public static final int CHINESE_SIMPLIFIED = 3;
	public static final int CHINESE_TRADITIONAL = 4;
	public static final int JAPANESE = 5;

	private static WordList getWordList(int language) throws DIDException {
		switch (language) {
		case ENGLISH:
			return English.INSTANCE;

		case FRENCH:
			return French.INSTANCE;

		case SPANISH:
			return Spanish.INSTANCE;

		case CHINESE_SIMPLIFIED:
			return ChineseSimplified.INSTANCE;

		case CHINESE_TRADITIONAL:
			return ChineseTraditional.INSTANCE;

		case JAPANESE:
			return Japanese.INSTANCE;

		default:
			UserDefinedWordLists wordLists;
			wordLists = UserDefinedWordLists.getInstance();
			if (wordLists != null) {
				WordList wordList = null;
				try {
					wordList = wordLists.getWordList(language);
				} catch (IOException e) {
					throw new DIDException("Load word list failed.", e);
				}

				if (wordList != null)
					return wordList;
			}

			return English.INSTANCE;
		}
	}

	public static String generate(int language) throws DIDException {
		if (language < 0)
			throw new IllegalArgumentException();

		StringBuilder mnemonic = new StringBuilder();

		byte[] entropy = new byte[Words.TWELVE.byteLength()];
		new SecureRandom().nextBytes(entropy);

		new MnemonicGenerator(getWordList(language))
		    .createMnemonic(entropy, mnemonic::append);

		return mnemonic.toString();
	}

	public static boolean isValid(int language, String mnemonic) {
		if (language < 0)
			throw new IllegalArgumentException();

	    try {
			MnemonicValidator
			.ofWordList(getWordList(language))
			.validate(mnemonic);
		} catch (InvalidChecksumException e) {
			return false;
		} catch (InvalidWordCountException e) {
			return false;
		} catch (WordNotFoundException e) {
			return false;
		} catch (UnexpectedWhiteSpaceException e) {
			return false;
		} catch (DIDException e) {
			return false;
		}

	    return true;
	}
}
