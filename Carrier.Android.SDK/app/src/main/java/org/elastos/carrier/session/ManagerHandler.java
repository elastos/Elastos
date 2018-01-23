package org.elastos.carrier.session;

import org.elastos.carrier.Carrier;

/**
 * The interface to session Manager.
 */
public interface ManagerHandler {

    /**
     * An callback function that handle session request.
     *
     * @param
     *      carrier     A handle to the Carrier node instance
     * @param
     *      from        The id who send the message
     * @param
     *      sdp         The remote users SDP. Reference: https://tools.ietf.org/html/rfc4566
     *
     */
    void onSessionRequest(Carrier carrier, String from, String sdp);
}
