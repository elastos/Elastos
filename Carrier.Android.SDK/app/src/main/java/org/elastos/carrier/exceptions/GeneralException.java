package org.elastos.carrier.exceptions;

public class GeneralException extends CarrierException {
	protected GeneralException(int errorCode) {
		super(errorCode);
	}

	protected GeneralException(int errorCode, String message) {
		super(errorCode, message);
	}

	protected GeneralException(int errorCode, String message, Throwable cause) {
		super(errorCode, message, cause);
	}

	// Only available when API_LEVEL >= 24
	/*
	protected GeneralException(int errorCode, String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
		super(errorCode, message, cause, enableSuppression, writableStackTrace);
	}
	*/

	protected GeneralException(int errorCode, Throwable cause) {
		super(errorCode, cause);
	}

	@Override
	public int getFacility() {
		return FACILITY_GENERAL;
	}
}
