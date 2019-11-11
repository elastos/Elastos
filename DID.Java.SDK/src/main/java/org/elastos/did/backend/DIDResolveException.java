package org.elastos.did.backend;

import org.elastos.did.DIDException;

public class DIDResolveException extends DIDException {
	private static final long serialVersionUID = 8679582737929676981L;

	public DIDResolveException() {
        super();
    }

    public DIDResolveException(String message) {
        super(message);
    }

    public DIDResolveException(String message, Throwable cause) {
        super(message, cause);
    }

    public DIDResolveException(Throwable cause) {
        super(cause);
    }
}
