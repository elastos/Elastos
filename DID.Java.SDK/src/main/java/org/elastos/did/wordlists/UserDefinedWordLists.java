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

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import io.github.novacrypto.bip39.WordList;

public class UserDefinedWordLists {
	private Map<Integer, FileWordList> wordLists;

	private static UserDefinedWordLists instance;

	public static void initialize(String dir) {
		if (dir == null || dir.isEmpty())
			throw new IllegalArgumentException();

		initialize(new File(dir));
	}

	public static void initialize(File dir) {
		if (!dir.exists() || !dir.isDirectory())
			throw new IllegalArgumentException("Folder '" +
					dir.getAbsolutePath() +
					"' not exists or not a directory");

		instance = new UserDefinedWordLists(dir);
	}

	public static UserDefinedWordLists getInstance() {
		return instance;
	}

	private UserDefinedWordLists(File dir) {
		wordLists = new HashMap<Integer, FileWordList>(8);

		dir.listFiles(new FileFilter() {
			@Override
			public boolean accept(File file) {
				if (!file.isFile())
					return false;

				String filename = file.getName();
				// The filename should be: number-Language.txt
				if (!filename.matches("(^\\d+)_(.+)\\.txt"))
					return false;

				int pos = filename.indexOf('_');
				int code = Integer.valueOf(filename.substring(0, pos));
				String language = filename.substring(pos + 1, filename.length() - 4);
				wordLists.put(code, new FileWordList(code, language, file));

				return false;
			}
		});
	}

	public WordList getWordList(int language) throws IOException {
		FileWordList wordList = wordLists.get(language);

		if (wordList != null) {
			// for lazy load, load the wordlist on demand.
			if (!wordList.isLoaded())
				wordList.load();

			if (!wordList.isValid())
				return null;
		}

		return wordList;
	}
}
