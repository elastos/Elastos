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

package org.elastos.did.wordlists;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

import io.github.novacrypto.bip39.WordList;

public class FileWordList implements WordList {
	private static final int WORD_COUNT = 2048;

	private int languageCode;
	private String languageName;
	private File wordListFile;

	private String[] words;

	protected FileWordList(int code, String language, File file) {
		this.languageCode = code;
		this.languageName = language;
		this.wordListFile = file;
	}

	protected void load() throws IOException {
		File file = wordListFile;
		wordListFile = null;

		BufferedReader in = new BufferedReader(new FileReader(file));

		String word;
		int index = 0;
		String[] words = new String[WORD_COUNT];

		while ((word = in.readLine()) != null && index < WORD_COUNT)
			words[index++] = word;

		in.close();

		if (index != WORD_COUNT)
			throw new IOException("Word list file '" +
					file.getAbsolutePath() + "' isn't a valid word list file.");

		this.words = words;
	}

	protected boolean isLoaded() {
		return wordListFile == null;
	}

	protected boolean isValid() {
		return words != null;
	}

	@Override
	public String getWord(int index) {
		return words[index];
	}

	@Override
	public char getSpace() {
		return ' ';
	}

	public int ordinal() {
		return languageCode;
	}

	public String name() {
		return languageName;
	}
}
