package org.elastos.credential;

import org.elastos.did.DIDException;

public class MalformedPresentationException extends DIDException {
	private static final long serialVersionUID = 8453912647945382207L;

	public MalformedPresentationException() {
        super();
    }

    public MalformedPresentationException(String message) {
        super(message);
    }

    public MalformedPresentationException(String message, Throwable cause) {
        super(message, cause);
    }

    public MalformedPresentationException(Throwable cause) {
        super(cause);
    }
}
