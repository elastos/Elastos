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

package org.elastos.did.examples;

import java.io.File;
import java.util.List;

import org.elastos.did.DID;
import org.elastos.did.DIDBackend;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDStore;
import org.elastos.did.Mnemonic;
import org.elastos.did.exception.DIDException;

public class InitializeDID {
	// Mnemonic passphrase and the store password should set by the end user.
	private final static String passphrase = "mypassphrase";
	private final static String storepass = "password";

	private DIDStore store;

	private void initDIDBackend() throws DIDException {
		// Get DID resolve cache dir.
		final String cacheDir = System.getProperty("user.home") + File.separator + ".cache"
				+ File.separator + "elastos.did";

		// Initializa the DID backend globally.
		DIDBackend.initialize("http://api.elastos.io:20606", cacheDir);
	}

	private void initPrivateIdentity() throws DIDException {
		final String storePath = System.getProperty("java.io.tmpdir")
				+ File.separator + "exampleStore";

		// Create a fake adapter, just print the tx payload to console.
		store = DIDStore.open("filesystem", storePath, (payload, memo) -> {
			System.out.println("Create ID transaction with:");
			System.out.println("  Payload = " + payload);
		});

		// Check the store whether contains the root private identity.
		if (store.containsPrivateIdentity())
			return; // Already exists

		// Create a mnemonic use default language(English).
		Mnemonic mg = Mnemonic.getInstance();
		String mnemonic = mg.generate();

		System.out.println("Please write down your mnemonic and passwords:");
		System.out.println("  Mnemonic: " + mnemonic);
		System.out.println("  Mnemonic passphrase: " + passphrase);
		System.out.println("  Store password: " + storepass);

		// Initialize the root identity.
		store.initPrivateIdentity(null, mnemonic, passphrase, storepass);
	}

	private void initDid() throws DIDException {
		// Check the DID store already contains owner's DID(with private key).
		List<DID> dids = store.listDids(DIDStore.DID_HAS_PRIVATEKEY);
		if (dids.size() > 0) {
			for (DID did : dids) {
				if (did.getMetadata().getAlias().equals("me")) {
					System.out.println("My DID: " + did);
					return; // Already create my DID.
				}
			}
		}

		DIDDocument doc = store.newDid("me", storepass);
		System.out.println("My new DID created: " + doc.getSubject());
		store.publishDid(doc.getSubject(), storepass);
	}

	public static void main(String args[]) {
		InitializeDID example = new InitializeDID();

		try {
			example.initDIDBackend();
			example.initPrivateIdentity();
			example.initDid();
		} catch (DIDException e) {
			e.printStackTrace();
		}
	}
}