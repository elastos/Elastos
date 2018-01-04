package org.elastos.carrier.session;

/**
 * Carrier stream type.
 *
 * Reference:
 *      https://tools.ietf.org/html/rfc4566#section-5.14
 *      https://tools.ietf.org/html/rfc4566#section-8
 */
public enum StreamType {

    /**
     * Audio stream.
     */
    Audio,

    /**
     * Video stream.
     */
    Video,

    /**
     * Text stream.
     */
    Text,

    /**
     * Application stream.
     */
    Application,

    /**
     * Message stream.
     */
    Message;

    /**
     * Get carrier stream type from type value.
     *
     * @param
     *      type        The type value
     *
     * @return
     *      The carrier stream type
     *
     * @throws
     *      IllegalArgumentException
     */
    public static StreamType valueOf(int type) {
        switch (type) {
            case 0:
                return Audio;
            case 1:
                return Video;
            case 2:
                return Text;
            case 3:
                return Application;
            case 4:
                return Message;
            default:
                throw new IllegalArgumentException("Invalid Stream type (expected: 0 ~ 4, Gieven:" + type);
        }
    }

    /**
     * Get type value of current stream type.
     *
     * @return
     *      The stream type value
     */
    public int value() {
        switch (this) {
            case Audio:
                return 0;
            case Video:
                return 1;
            case Text:
                return 2;
            case Application:
                return 3;
            case Message:
                return 4;
            default:
                return 5;
        }
    }

    /**
     * Get description of carrier stream type.
     *
     * @param
     *      type        The carrier stream type
     *
     * @return
     *      The description string of stream type.
     */
    public static String format(StreamType type) {
        return String.format("%s", type.name());
    }

    /**
     * Get description of current carrier stream type.
     *
     * @return
     *      The description string.
     */
    @Override
    public String toString() {
        return format(this);
    }
}
