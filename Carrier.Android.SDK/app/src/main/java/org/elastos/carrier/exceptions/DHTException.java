package org.elastos.carrier.exceptions;

public class DHTException extends CarrierException {
	protected DHTException(int errorCode) {
		super(errorCode);
	}

	protected DHTException(int errorCode, String message) {
		super(errorCode, message);
	}

	protected DHTException(int errorCode, String message, Throwable cause) {
		super(errorCode, message, cause);
	}

	// Only available when API_LEVEL >= 24
	/*
	protected DHTException(int errorCode, String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
		super(errorCode, message, cause, enableSuppression, writableStackTrace);
	}
	*/

	protected DHTException(int errorCode, Throwable cause) {
		super(errorCode, cause);
	}

	@Override
	public int getFacility() {
		return FACILITY_DHT;
	}
}
