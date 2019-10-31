package org.elastos.did;

import java.io.File;

public final class Util {
	public static void deleteFile(File file) {
		if (file.isDirectory()) {
			File[] children = file.listFiles();
			for (File child : children)
				deleteFile(child);
		}

		file.delete();
	}
}
