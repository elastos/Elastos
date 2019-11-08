package org.elastos.did.adaptor;

public class SPVAdaptor {
	static {
		System.loadLibrary("spvadaptorjni");
	}

	private long handle;

	public SPVAdaptor(String walletDir, String walletId, String network) {
		handle = create(walletDir, walletId, network);
	}

	public void destroy() {
		destroy(handle);
		handle = 0;
	}

	public int createIdTransaction(String payload, String memo,
			String password) {
		return createIdTransaction(handle, payload, memo, password);
	}

	public String resolve(String did) {
		return resolve(handle, did);
	}

	private final static native long create(String walletDir, String walletId,
			String network);

	private final static native void destroy(long handle);

	private final static native int createIdTransaction(long handle,
			String payload, String memo, String password);

	private final static native String resolve(long handle, String did);
}
