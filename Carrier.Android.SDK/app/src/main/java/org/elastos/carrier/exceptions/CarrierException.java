/*
 * Copyright (c) 2018 Elastos Foundation
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

package org.elastos.carrier.exceptions;

public abstract class CarrierException extends Exception {
	public static final int FACILITY_GENERAL	= 1;
	public static final int FACILITY_SYSTEM		= 2;
	public static final int FACILITY_RESERVED1	= 3;
	public static final int FACILITY_RESERVED2	= 4;
	public static final int FACILITY_ICE		= 5;
	public static final int FACILITY_DHT		= 6;

	private static final long serialVersionUID = -1729415961509977814L;
	private int errorCode;

	protected CarrierException(int errorCode) {
		super();
		this.errorCode = errorCode;
	}

	protected CarrierException(int errorCode, String message) {
		super(message);
		this.errorCode = errorCode;
	}

	protected CarrierException(int errorCode, String message, Throwable cause) {
		super(message, cause);
		this.errorCode = errorCode;
	}

	// Only available when API_LEVEL >= 24
	/*
	protected CarrierException(int errorCode, String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
		super(message, cause, enableSuppression, writableStackTrace);
		this.errorCode = errorCode;
	}
	*/

	protected CarrierException(int errorCode, Throwable cause) {
		super(cause);
		this.errorCode = errorCode;
	}

	@Override
	public String getMessage() {
		String message = super.getMessage();
		if (message != null)
			return message;

		// TODO: call Carrier native function convert errorCode to message,
		//       or null if no message associated with the errorCode.
		// message = Carrier.getErrorMessage(errorCode);
		return message;
	}

	public abstract int getFacility();

	public int getCode() {
		return errorCode & 0x00FFFFFF;
	}

	public int getErrorCode() {
		return errorCode;
	}

	private static int getFacility(int errorCode) {
		return (errorCode & 0x7FFFFFFF) >> 24;
	}

	public static CarrierException fromErrorCode(int errorCode, String message, Throwable cause) {
		CarrierException e;

		int facility = getFacility(errorCode);
		switch (facility) {
			case FACILITY_GENERAL:
				e = new GeneralException(errorCode, message, cause);
				break;

			case FACILITY_SYSTEM:
				e = new SystemException(errorCode, message, cause);
				break;

			case FACILITY_ICE:
				e = new ICEException(errorCode, message, cause);
				break;

			case FACILITY_DHT:
				e = new DHTException(errorCode, message, cause);
				break;

			default:
				e = new UnknownException(errorCode, message, cause);
		}

		return e;
	}

	public static CarrierException fromErrorCode(int errorCode, String message) {
		return fromErrorCode(errorCode, message, null);

	}

	public static CarrierException fromErrorCode(int errorCode, Throwable cause) {
		return fromErrorCode(errorCode, null, cause);
	}

	public static CarrierException fromErrorCode(int errorCode) {
		return fromErrorCode(errorCode, null, null);
	}
}
