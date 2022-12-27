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
import java.util.Properties;

public final class TestConfig {
	public static String networkConfig;
	public static String resolver;

	public static boolean dummyBackend;

	public static boolean verbose;

	public static String passphrase;

	public static String tempDir;

	public static String storeRoot;
	public static String storePass;

	public static String walletDir;
	public static String walletId;
	public static String walletPassword;

	static {
		InputStream input = TestConfig.class
				.getClassLoader().getResourceAsStream("test.conf");

		Properties config = new Properties();
		try {
			config.load(input);
		} catch (IOException e) {
			e.printStackTrace();
		}

		String sysTemp = System.getProperty("java.io.tmpdir");

		networkConfig = config.getProperty("network");
		resolver = config.getProperty("resolver");

		dummyBackend = Boolean.valueOf(config.getProperty("backend.dummy"));
		verbose = Boolean.valueOf(config.getProperty("dummystore.verbose"));

		passphrase = config.getProperty("mnemnoic.passphrase");

		tempDir = config.getProperty("temp.dir", sysTemp);

		storeRoot = config.getProperty("store.root", sysTemp + "/DIDStore");
		storePass = config.getProperty("store.pass");

		walletDir = config.getProperty("wallet.dir");
		walletId = config.getProperty("wallet.id");
		walletPassword = config.getProperty("wallet.password");
	}
}
