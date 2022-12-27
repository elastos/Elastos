package org.elastos.carrier.exceptions;

public class UnknownException extends CarrierException {
	protected UnknownException(int errorCode) {
		super(errorCode);
	}

	protected UnknownException(int errorCode, String message) {
		super(errorCode, message);
	}

	protected UnknownException(int errorCode, String message, Throwable cause) {
		super(errorCode, message, cause);
	}

	// Only available when API_LEVEL >= 24
	/*
	public UnknownException(int errorCode, String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
		super(errorCode, message, cause, enableSuppression, writableStackTrace);
	}
	*/

	protected UnknownException(int errorCode, Throwable cause) {
		super(errorCode, cause);
	}

	@Override
	public int getFacility() {
		return 0;
	}

	@Override
	public int getCode() {
		return getErrorCode();
	}
}
