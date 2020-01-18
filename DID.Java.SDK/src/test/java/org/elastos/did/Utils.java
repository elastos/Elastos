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

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Utils {
	private static String[] removeIgnoredFiles(String[] names) {
		List<String> lst = new ArrayList<String>(Arrays.asList(names));
		lst.remove(".DS_Store");
		return lst.toArray(new String[0]);
	}

	public static boolean equals(File file1, File file2) throws IOException {
		if (file1 == null && file2 == null)
			return true;

		if (file1 == null ^ file2 == null)
			return false;

		if (file1.compareTo(file2) == 0)
			return true;

		if (file1.exists() ^ file2.exists())
			return false;

		if (!file1.exists())
			return true;

		if (file1.isDirectory() ^ file2.isDirectory())
			return false;

		if (file1.isDirectory()) {
			File dir1 = file1;
			File dir2 = file2;

			String[] files1 = removeIgnoredFiles(dir1.list());
			String[] files2 = removeIgnoredFiles(dir2.list());

			if (files1.length != files2.length)
				return false;

			Arrays.sort(files1);
			Arrays.sort(files2);
			if (!Arrays.equals(files1, files2))
				return false;

			String[] files = files1;
			for (int i = 0; i < files.length; i++) {
				File f1 = new File(dir1, files[i]);
				File f2 = new File(dir2, files[i]);

				if (!equals(f1, f2))
					return false;
			}

			return true;
		} else {
			if (file1.length() != file2.length())
				return false;

			// BufferedInputStream for better performance
	        InputStream in1 = new BufferedInputStream(new FileInputStream(file1));
	        InputStream in2 = new BufferedInputStream(new FileInputStream(file2));

	        try {
		        int ch1 = in1.read();
		        int ch2;
		        while (-1 != ch1) {
		            ch2 = in2.read();
		            if (ch1 != ch2)
		                return false;

		            ch1 = in1.read();
		        }

		        return in2.read() == -1;
	        } finally {
	        	in1.close();
	        	in2.close();
	        }
		}
	}

	public static void deleteFile(File file) {
		if (file.isDirectory()) {
			File[] children = file.listFiles();
			for (File child : children)
				deleteFile(child);
		}

		file.delete();
	}
}

