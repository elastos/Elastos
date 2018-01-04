package org.elastos.carrier.session;

/**
 * Port forwarding supported protocols.
 */
public enum PortForwardingProtocol {
    /**
     * UDP protocol.
     */
    UDP,

    /**
     * TCP protocol.
     */
    TCP;

    /**
     * Get port forwarding protocol from protocol value.
     *
     * @param
     *      protocol        The protocol value
     *
     * @return
     *      The port forwarding protocol
     *
     * @throws
     *      IllegalArgumentException
     */
    public static PortForwardingProtocol valueOf(int protocol) {
        switch (protocol) {
            case 0:
                return UDP;
            case 1:
                return TCP;
            default:
                throw new IllegalArgumentException("Invalid Protocol (expected: 0 ~ 1, Gieven:" + protocol);
        }
    }

    /**
     * Get value of port forwarding protocol.
     *
     * @return
     *      The value of port forwarding protocol
     */
    public int value() {
        switch (this) {
            case TCP:
                return 1;
            case UDP:
            default:
                return 0;
        }
    }

    /**
     * Get the debug description of port forwarding protocol
     *
     * @param
     *      protocol        The port forwarding protocol
     *
     * @return
     *      The deubg description of port forwarding protocol
     */
    public static String format(PortForwardingProtocol protocol) {
        return String.format("%s", protocol.name());
    }

    /**
     * Get the debug description
     *
     * @return
     *      The debug description of current port forwarding protocol
     */
    @Override
    public String toString() {
        return format(this);
    }
}
