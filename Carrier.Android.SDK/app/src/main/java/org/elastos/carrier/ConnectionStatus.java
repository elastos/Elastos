package org.elastos.carrier;

/**
 * Carrier node connection status to the carrier network.
 */
public enum ConnectionStatus {
	/**
	 * Carrier node connected to the carrier network.
	 * Indicate the node is online.
	 */
	Connected,

	/**
	 * There is no connection to the carrier network.
	 * Indicate the node is offline.
	 */
	Disconnected;

	/**
	 * Get ConnectionStatus object from staus integter value.
	 *
	 * @param
	 * 		status			The status value
	 *
	 * @return
	 * 		The ConnectionStatus object
	 *
	 * @throws
	 * 		IllegalArgumentException
     */
	public static ConnectionStatus valueOf(int status) {
		switch (status) {
			case 0:
				return Connected;
			case 1:
				return Disconnected;
			default:
				throw new IllegalArgumentException("Invalid Connection Status (expected: 0 ~ 1, " +
						"Gieven:" + status);
		}
	}

	/**
	 * Get the status value of current ConnectionStatus object.
	 *
	 * @return
	 * 		The connection status value.
     */
	public int value() {
		switch (this) {
			case Connected:
				return 0;
			case Disconnected:
			default:
				return 1;
		}
	}

	/**
	 * Get the debug description of the ConnectionStatus object.
	 *
	 * @param
	 * 		status		The connect status
	 *
	 * @return
	 * 		The debug description of ConnectionStatus object
	 */
	static String format(ConnectionStatus status) {
		return String.format("%s[%d]", status.name(), status.value());
	}

	/**
	 * Get the debug description of current ConnectionStatus object.
	 *
	 * @return
	 * 		The debug description of current ConnectionStatus object.
	 */
	@Override
	public String toString() {
		return format(this);
	}
}