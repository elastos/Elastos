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

public final class TestConfig {
	/*
	public final static boolean verbose = false;

	public final static String tempDir = "/PATH/TO/TEMP";

	public final static String storeRoot = "/PATH/TO/DIDStore";
	public final static String storePass = "passwd";
	public final static String passphrase = "secret";

	public final static String walletDir = "/PATH/TO/WALLET";
	public final static String walletId = "test";
	public final static String walletPassword = "passwd";
	public final static String networkConfig = "/PATH/TO/privnet.json";
	public final static String resolver = "https://coreservices-didsidechain-privnet.elastos.org";
	*/

	public final static boolean verbose = true;

	public final static String tempDir = "/Users/jingyu/Temp";

	public final static String storeRoot = "/Users/jingyu/Temp/DIDStore";
	public final static String storePass = "passwd";
	public final static String passphrase = "secret";

	public final static String walletDir = "/Users/jingyu/.didwallet";
	public final static String walletId = "test";
	public final static String walletPassword = "helloworld";
	public final static String networkConfig = "/Users/jingyu/Projects/Elastos/DID/Native.SDK.jy/adapter/wallet/privnet.json";
	public final static String resolver = "https://coreservices-didsidechain-privnet.elastos.org";
}
