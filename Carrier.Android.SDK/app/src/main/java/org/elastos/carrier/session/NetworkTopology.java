package org.elastos.carrier.session;

/**
 * Carrier network topology for session peers related to each other.
 */
public enum NetworkTopology {
	/**
	 * LAN network topology.
	 */
	LAN,
	/**
	 * P2P network topology,
	 */
	P2P,
	/**
	 * Relayed netowrk topology.
	 */
	Relayed;

	/**
	 * Get network topolgoy.
	 *
	 * @param
	 *      type      The value of topolgoy.
	 *
	 * @return
	 *      The network topology instance.
	 *
	 * @throws
	 *      IllegalArgumentException
	 */
	public static NetworkTopology valueOf(int type) {
		switch (type) {
			case 0:
				return LAN;
			case 1:
				return P2P;
			case 2:
				return Relayed;
			default:
				throw new IllegalArgumentException("Invalid network topology (expected: 0 ~ 3, Gieven:" + type);
		}
	}

	/**
	 * Get netowrk topology type.
	 *
	 * @return
	 *      The topology value.
	 */
	public int value() {
		switch (this) {
			case LAN:
				return 0;
			case P2P:
				return 1;
			case Relayed:
			default:
				return 2;
		}
	}

	/**
	 * Get the fully formatized string of network topology type.
	 *
	 * @param
	 *      type      The network topology instance.
	 *
	 * @return
	 *      The formatized string of network topology.
	 */
	public static String format(NetworkTopology type) {
		return String.format("%s[%d]", type.name(), type.value());
	}

	/**
	 * Get fully formatized string.
	 *
	 * @return
	 *      The fully formatized string.
	 */
	@Override
	public String toString() {
		return format(this);
	}
}
