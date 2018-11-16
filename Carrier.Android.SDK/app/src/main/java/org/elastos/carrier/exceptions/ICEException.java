package org.elastos.carrier.exceptions;

public class ICEException extends CarrierException {
	protected ICEException(int errorCode) {
		super(errorCode);
	}

	protected ICEException(int errorCode, String message) {
		super(errorCode, message);
	}

	protected ICEException(int errorCode, String message, Throwable cause) {
		super(errorCode, message, cause);
	}

	// Only available when API_LEVEL >= 24
	/*
	protected ICEException(int errorCode, String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
		super(errorCode, message, cause, enableSuppression, writableStackTrace);
	}
	*/

	protected ICEException(int errorCode, Throwable cause) {
		super(errorCode, cause);
	}

	@Override
	public int getFacility() {
		return FACILITY_ICE;
	}
}
