package org.elastos.carrier.session;

/**
 * Multiplexing channel close reason mode.
 */
public enum CloseReason {
    /**
     * Channel closed normaly.
     */
    Normal,

    /**
     * Channel closed because of timeout.
     */
    Timeout,

    /**
     * Channel closed because error occured.
     */
    Error;

    /**
     * Get CloseReason instance from reason value.
     *
     * @param
     *      reason      The value of reason mode.
     *
     * @return
     *      The close reason instance.
     *
     * @throws
     *      IllegalArgumentException
     */
    public static CloseReason valueOf(int reason) {
        switch (reason) {
            case 0:
                return Normal;
            case 1:
                return Timeout;
            case 2:
                return Error;
            default:
                throw new IllegalArgumentException("Invalid close reason (expected: 0 ~ 2, Gieven:" + reason);
        }
    }

    /**
     * Get reason value.
     *
     * @return
     *      The reason value.
     */
    public int value() {
        switch (this) {
            case Normal:
                return 0;
            case Timeout:
                return 1;
            case Error:
            default:
                return 2;
        }
    }

    /**
     * Get the fully formatized string of close reason instance.
     *
     * @param
     *      reason      The close reason instance.
     *
     * @return
     *      The formatized string of close reason.
     */
    public static String format(CloseReason reason) {
        return String.format("%s[%d]", reason.name(), reason.value());
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