package org.elastos.carrier.session;

/**
 * Carrier stream state
 *
 * The stream state will be changed according to the phase of the stream.
 */
public enum StreamState {
    /**
     * Initialized stream.
     */
    Initialized,

    /**
     * The underlying transport is ready for the stream to start.
     */
    TransportReady,

    /**
     * The stream is trying to connect the remote.
     */
    Connecting,

    /**
     * The stream connected with remove peer.
     */
    Connected,

	/**
	 * The stream is deactived.
     */
    Deactivated,

	/**
	 * The stream closed gracefully.
     */
    Closed,

    /**
     * The stream is on error, cannot to continue.
     */
    Error;

    /**
     * Get carrier stream state from state value.
     *
     * @param
     *      state       The state value.
     *
     * @return
     *      The carrier stream state.
     *
     * @throws
     *      IllegalArgumentException
     */
    public static StreamState valueOf(int state) {
        switch (state) {
            case 1:
                return Initialized;
            case 2:
                return TransportReady;
            case 3:
                return Connecting;
            case 4:
                return Connected;
            case 5:
                return Deactivated;
            case 6:
                return Closed;
            case 7:
                return Error;
            default:
                throw new IllegalArgumentException("Invalid Stream State (expected: 1 ~ 7, Gieven:" + state);
        }
    }

    /**
     * Get state value.
     *
     * @return
     *      The state value.
     */
    public int value() {
        switch (this) {
            case Initialized:
                return 1;
            case TransportReady:
                return 2;
            case Connecting:
                return 3;
            case Connected:
                return 4;
            case Deactivated:
                return 5;
            case Closed:
                return 6;
            case Error:
            default:
                return 7;
        }
    }

    /**
     * Get description of carrier stream state.
     *
     * @param
     *      state       The carrier stream state.
     *
     * @return
     *      The description of carrier stream state.
     */
    public static String format(StreamState state) {
        return String.format("%s", state.name());
    }

    /**
     * Get description of current carrier stream state.
     *
     * @return
     *      The human readable description.
     */
    @Override
    public String toString() {
        return format(this);
    }
}

