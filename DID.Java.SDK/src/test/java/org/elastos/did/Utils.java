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
import java.util.Arrays;

public class Utils {
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
			String[] names1 = file1.list();
			String[] names2 = file2.list();

			if (names1.length != names2.length)
				return false;

			Arrays.sort(names1);
			Arrays.sort(names2);
			if (!Arrays.equals(names1, names2))
				return false;


			File[] files1 = file1.listFiles();
			File[] files2 = file2.listFiles();

			Arrays.sort(files1);
			Arrays.sort(files2);

			for (int i = 0; i < files1.length; i++) {
				if (!equals(files1[i], files2[i]))
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

