package org.elastos.carrier.exceptions;

public class SystemException extends CarrierException {
	protected SystemException(int errorCode) {
		super(errorCode);
	}

	protected SystemException(int errorCode, String message) {
		super(errorCode, message);
	}

	protected SystemException(int errorCode, String message, Throwable cause) {
		super(errorCode, message, cause);
	}

	// Only available when API_LEVEL >= 24
	/*
	protected SystemException(int errorCode, String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
		super(errorCode, message, cause, enableSuppression, writableStackTrace);
	}
	*/

	protected SystemException(int errorCode, Throwable cause) {
		super(errorCode, cause);
	}

	@Override
	public int getFacility() {
		return FACILITY_SYSTEM;
	}
}
