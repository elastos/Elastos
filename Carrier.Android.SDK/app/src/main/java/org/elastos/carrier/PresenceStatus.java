package org.elastos.carrier;

/**
 * Carrier node presence status.
 */
public enum PresenceStatus {

    /**
     * Carrier node is online and available.
     */
    None,

    /**
     * Carrier node is being away.
     */
    Away,

    /**
     * Carrier node is being busy.
     */
    Busy;

    /**
     * Get PresenceStatus object from status value.
     *
     * @param
     *      status    The presence status value.
     *
     * @return
     *      The Carrier node status object.
     *
     * @throws
     *      IllegalArgumentException
     */
    public static PresenceStatus valueOf(int status)  {
        switch (status) {
            case 0:
                return None;
            case 1:
                return Away;
            case 2:
                return Busy;
            default:
                throw new IllegalArgumentException("Invalid presence status (expected: 0 ~ 2, Gieven:" + status);
        }
    }

    /**
     * Get carrier node presence status value.
     *
     * @return
     *      The carrier node presence status value.
     */
    public int value() {
        switch (this) {
            case None:
                return 0;
            case Away:
                return 1;
            case Busy:
            default:
                return 0;
        }
    }

    /**
     * Get debug description of carrier node presence.
     *
     * @param
     *      status        The carrier node presense status.
     *
     * @return
     *      The debug description of presence
     */
    public static String format(PresenceStatus status) {
        return String.format("%s[%d]", status.name(), status.value());
    }

    /**
     * Get debug description of current node presence.
     *
     * @return
     *      The debug description of current node presence.
     */
    @Override
    public String toString() {
        return format(this);
    }
}
