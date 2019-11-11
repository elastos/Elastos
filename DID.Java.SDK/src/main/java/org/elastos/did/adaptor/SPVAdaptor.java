package org.elastos.did.adaptor;

import org.elastos.did.DIDAdaptor;
import org.elastos.did.DIDException;

public class SPVAdaptor implements DIDAdaptor {
	static {
		System.loadLibrary("spvadaptorjni");
	}

	private String walletDir;
	private String walletId;
	private String network;

	private long handle;
	private PasswordCallback passwordCallback;

	public interface PasswordCallback {
		public String getPassword(String walletDir, String walletId);
	}

	public SPVAdaptor(String walletDir, String walletId, String network,
			String resolver, PasswordCallback passwordCallback) {
		handle = create(walletDir, walletId, network, resolver);
		this.walletDir = walletDir;
		this.walletId = walletId;
		this.network = network;
		this.passwordCallback = passwordCallback;
	}

	public void destroy() {
		destroy(handle);
		handle = 0;
	}

	private final static native long create(String walletDir, String walletId,
			String network, String resolver);

	private final static native void destroy(long handle);

	private final static native int createIdTransaction(long handle,
			String payload, String memo, String password);

	private final static native String resolve(long handle, String did);

	@Override
	public boolean createIdTransaction(String payload, String memo)
			throws DIDException {
		String password = passwordCallback.getPassword(walletDir, walletId);
		if (password == null)
			return false;

		int rc = createIdTransaction(handle, payload, memo, password);
		return rc == 0;
	}

	@Override
	public String resolve(String did) {
		return resolve(handle, did);
	}
}
