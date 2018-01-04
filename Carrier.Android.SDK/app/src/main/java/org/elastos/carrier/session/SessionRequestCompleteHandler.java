package org.elastos.carrier.session;

/**
 * The interface to carrier session.
 */
public interface SessionRequestCompleteHandler {

    /**
     * The callback function to receive session request complete event.
     *
     * @param
     *      session     The carrier session instance.
     * @param
     *      status      The status code of the response.
     *                  0 is success, otherwise is error.
     * @param
     *      reason      The error message if status is error, or nil if session request
     *                  error happened
     * @param
     *      sdp         The remote users SDP.
     *                  Reference: https://tools.ietf.org/html/rfc4566
     */
    void onCompletion(Session session, int status, String reason, String sdp);
}
