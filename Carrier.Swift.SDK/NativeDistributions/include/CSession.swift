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

import Foundation

/**
 * \~English
 * carrier stream types definition.
 * Reference:
 *      https://tools.ietf.org/html/rfc4566#section-5.14
 *      https://tools.ietf.org/html/rfc4566#section-8
 */
internal struct CStreamType : RawRepresentable, Equatable {

    init(_ rawValue: UInt32) {
        self.rawValue = rawValue
    }

    init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    var rawValue: UInt32
}

/**
 * \~English
 *  Audio stream.
 */
internal var CStreamType_audio: CStreamType { get { return CStreamType(0) } }
/**
 * \~English
 *  Video stream.
 */
internal var CStreamType_video: CStreamType { get { return CStreamType(1) } }
/**
 * \~English
 *  Text stream.
 */
internal var CStreamType_text: CStreamType { get { return CStreamType(2) } }
/**
 * \~English
 *  Application stream.
 */
internal var CStreamType_application: CStreamType { get { return CStreamType(3) } }
/**
 * \~English
 *  Message stream.
 */
internal var CStreamType_message: CStreamType { get { return CStreamType(4) } }

internal struct CCandidateType : RawRepresentable, Equatable {

    init(_ rawValue: UInt32) {
        self.rawValue = rawValue
    }

    init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    var rawValue: UInt32
}

internal var CCandidateType_Host: CCandidateType { get { return CCandidateType(0) } }
internal var CCandidateType_ServerReflexive: CCandidateType { get { return CCandidateType(1) } }
internal var CCandidateType_PeerReflexive: CCandidateType { get { return CCandidateType(2) } }
internal var CCandidateType_Relayed: CCandidateType { get { return CCandidateType(3) } }

internal struct CNetworkTopology : RawRepresentable, Equatable {

    init(_ rawValue: UInt32) {
        self.rawValue = rawValue
    }

    init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    var rawValue: UInt32
}
internal var CNetworkTopology_LAN: CNetworkTopology { get { return CNetworkTopology(0) } }
internal var CNetworkTopology_P2P: CNetworkTopology { get { return CNetworkTopology(1) } }
internal var CNetworkTopology_RELAYED: CNetworkTopology { get { return CNetworkTopology(2) } }

internal struct CAddressInfo {

    var type: CCandidateType = CCandidateType_Host

    var addr: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var port: Int32 = 0

    var related_addr: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var related_port: Int32 = 0

    init() {}
}

internal struct CTransportInfo {

    var topology: CNetworkTopology = CNetworkTopology_LAN

    var local: CAddressInfo = CAddressInfo()

    var remote: CAddressInfo = CAddressInfo()

    init() {}
}

/**
 * \~English
 * An application-defined function that handle session requests.
 *
 * ElaSessionRequestCallback is the callback function type.
 *
 * @param
 *      carrier     [in] A handle to the ElaCarrier node instance.
 * @param
 *      from        [in] The id(userid@nodeid) from who send the message.
 * @param
 *      bundle      [in] The bundle of this session.
 * @param
 *      sdp         [in] The remote users SDP. End the null terminal.
 *                       Reference: https://tools.ietf.org/html/rfc4566
 * @param
 *      len         [in] The length of the SDP.
 * @param
 *      context     [in] The application defined context data.
 */
internal typealias CSessionRequestCallback = @convention(c)
    (OpaquePointer?, UnsafePointer<Int8>?, UnsafePointer<Int8>?,
    UnsafePointer<Int8>?, Int, UnsafeMutableRawPointer?) -> Swift.Void

/**
 * \~English
 * Initialize carrier session extension.
 *
 * The application must initialize the session extension before calling
 * any session API.
 *
 * @param
 *      carrier     [in] A handle to the Carrier node instance.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_session_init")
internal func ela_session_init(_ carrier: OpaquePointer!) -> Int32

/**
 * \~English
 * Clean up Carrier session extension.
 *
 * The application should call ela_session_cleanup before quit,
 * to clean up the resources associated with the extension.
 *
 * If the extension is not initialized, this function has no effect.
 *
 * @param
 *      carrier [in] A handle to the carrier node instance.
 */
@_silgen_name("ela_session_cleanup")
internal func ela_session_cleanup(_ carrier: OpaquePointer!)

/**
 * \~English
 * Set session request callback.
 *
 * @param
 *      carrier     [in] A handle to the carrier node instance.
 * @param
 *      bundle_prefix
 *                  [in] The prefix of bundle.
 * @param
 *      callback
 *                  [in] The callback function to process this request.
 * @param
 *      context
 *                  [in] The application defined context data.
 *
 * @return
 *      If no error occurs, return 0.
 *      Otherwise, return -1, and a specific error code can be
 *      retrieved by calling ela_get_error().
 */
 @_silgen_name("ela_session_set_callback")
 internal func ela_session_set_callback(_ carrier: OpaquePointer!,
                                        _ bundle_prefix: UnsafePointer<Int8>?,
                                        _ callback: CSessionRequestCallback!,
                                        _ context: UnsafeMutableRawPointer!) -> Int32

/**
 * \~English
 * Create a new session to a friend.
 *
 * The session object represent a conversation handle to a friend.
 *
 * @param
 *      carrier     [in] A handle to the carrier node instance.
 * @param
 *      address     [in] The target address.
 *
 * @return
 *      If no error occurs, return the pointer of ElaSession object.
 *      Otherwise, return NULL, and a specific error code can be
 *      retrieved by calling ela_get_error().
 */
@_silgen_name("ela_session_new")
internal func ela_session_new(_ carrier: OpaquePointer!,
                              _ address: UnsafePointer<Int8>!) -> OpaquePointer!

/**
 * \~English
 * Close a session to friend. All resources include streams, multiplexers
 * associated with current session will be destroyed.
 *
 * @param
 *      session     [in] A handle to the carrier session.
 */
@_silgen_name("ela_session_close")
internal func ela_session_close(_ session: OpaquePointer!)

/**
 * \~English
 * Get the remote peer id (userid or userid@nodeid) of the session.
 *
 * @param
 *      session     [in] A handle to the carrier session.
 * @param
 *      address     [out] The buffer that will receive the peer address.
 *                        The buffer size should at least
 *                        (2 * ELA_MAX_ID_LEN + 1) bytes.
 * @param
 *      len         [in] The buffer size of appid.
 *
 * @return
 *      The remote peer string address, or NULL if buffer is too small.
 */
@_silgen_name("ela_session_get_peer")
internal func ela_session_get_peer(_ session: OpaquePointer!,
                                   _ address: UnsafePointer<Int8>!,
                                   _ len: Int) -> UnsafePointer<Int8>!

/**
 * \~English
 * Set the arbitary user data to be associated with the session.
 *
 * @param
 *      session     [in] A handle to the carrier session.
 * @param
 *      userdata    [in] Arbitary user data to be associated with this session.
 */
@_silgen_name("ela_session_set_userdata")
internal func ela_session_set_userdata(_ session: OpaquePointer!,
                             _ userdata: UnsafeMutableRawPointer!) -> Swift.Void
/**
 * \~English
 * Get the user data associated with the session.
 *
 * @param
 *      session     [in] A handle to the carrier session.
 *
 * @return
 *      The user data associated with session.
 */
@_silgen_name("ela_session_get_userdata")
internal func ela_session_get_userdata(_ session: OpaquePointer!) -> UnsafeMutableRawPointer!

/**
 * \~English
 * An application-defined function that receive session request complete
 * event.
 *
 * ElaSessionRequestCompleteCallback is the callback function type.
 *
 * @param
 *      session     [in] A handle to the ElaSession.
 * @param
 *      bundle      [in] The bundle of this session.
 * @param
 *      status      [in] The status code of the response.
 *                       0 is success, otherwise is error.
 * @param
 *      reason      [in] The error message if status is error, or NULL
 * @param
 *      sdp         [in] The remote users SDP. End the null terminal.
 *                       Reference: https://tools.ietf.org/html/rfc4566
 * @param
 *      len         [in] The length of the SDP.
 * @param
 *      context     [in] The application defined context data.
 */
internal typealias CSessionRequestCompleteCallback = @convention(c)
    (OpaquePointer?, UnsafePointer<Int8>?, Int32, UnsafePointer<Int8>?,
    UnsafePointer<Int8>?, Int, UnsafeMutableRawPointer?) -> Swift.Void

/**
 * \~English
 * Send session request to the friend.
 *
 * @param
 *      session     [in] A handle to the ElaSession.
 * @param
 *      bundle      [in] The bundle of this session.
 * @param
 *      callback    [in] A pointer to ElaSessionRequestCompleteCallback
 *                       function to receive the session response.
 * @param
 *      context      [in] The application defined context data.
 *
 * @return
 *      0 if the session request successfully send to the friend.
 *      Otherwise, return -1, and a specific error code can be
 *      retrieved by calling ela_get_error().
 */
@_silgen_name("ela_session_request")
internal func ela_session_request(_ session: OpaquePointer!,
                                  _ bundle: UnsafePointer<Int8>?,
                                  _ callback: CSessionRequestCompleteCallback!,
                                  _ context: UnsafeMutableRawPointer!) -> Int32

/**
 * \~English
 * Reply the session request from friend.
 *
 * This function will send a session response to friend.
 *
 * @param
 *      session     [in] A handle to the ElaSession.
 * @param
 *      bundle      [in] The bundle of this session.
 * @param
 *      status      [in] The status code of the response.
 *                       0 is success, otherwise is error.
 * @param
 *      reason      [in] The error message if status is error, or NULL
 *                       if success.
 *
 * @return
 *      0 if the session response successfully send to the friend.
 *      Otherwise, return -1, and a specific error code can be
 *      retrieved by calling ela_get_error().
 */
@_silgen_name("ela_session_reply_request")
internal func ela_session_reply_request(_ session: OpaquePointer!,
                                        _ bundle: UnsafePointer<Int8>?,
                                        _ status: Int32,
                                        _ reason: UnsafePointer<Int8>!) -> Int32

/**
 * \~English
 * Begin to start a session.
 *
 * All streams in current session will try to connect with remote friend,
 * the stream status will update to application by stream's callbacks.
 *
 * @param
 *      session     [in] A handle to the ElaSession.
 * @param
 *      sdp         [in] The remote users SDP. End the null terminal.
 *                       Reference: https://tools.ietf.org/html/rfc4566
 * @param
 *      len         [in] The length of the SDP.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
@_silgen_name("ela_session_start")
internal func ela_session_start(_ session: OpaquePointer!,
                                _ sdp: UnsafePointer<Int8>!,
                                _ len: Int) -> Int32
/**
 * \~English
 * Carrier stream state.
 * The stream status will be changed according to the phase of the stream.
 */
internal struct CStreamState : RawRepresentable, Equatable {

    init(_ rawValue: UInt32) {
        self.rawValue = rawValue
    }

    init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    var rawValue: UInt32
}

/** Initialized stream */
internal var CStreamState_initialized: CStreamState { get { return CStreamState(1) } }
/** The underlying transport is ready for the stream. */
internal var CStreamState_transport_ready: CStreamState { get { return CStreamState(2) } }
/** The stream is trying to connecting the remote. */
internal var CStreamState_connecting: CStreamState { get { return CStreamState(3) } }
/** The stream connected with remote */
internal var CStreamState_connected: CStreamState { get { return CStreamState(4) } }
/** The stream is deactivated */
internal var CStreamState_deactivated: CStreamState { get { return CStreamState(5) } }
/** The stream closed normally */
internal var CStreamState_closed: CStreamState { get { return CStreamState(6) } }
/** The stream is failed, cannot to continue. */
internal var CStreamState_failed: CStreamState { get { return CStreamState(7) } }

/**
 * \~English
 * Portforwarding supported protocols.
 */
internal struct CPortForwardingProtocol : RawRepresentable, Equatable {

    init(_ rawValue: UInt32) {
        self.rawValue = rawValue
    }

    init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    var rawValue: UInt32
}

/** TCP protocol. */
internal var CPortForwardingProtocol_TCP: CPortForwardingProtocol { get { return CPortForwardingProtocol(1) } }

/**
 * \~English
 * Multiplexing channel close reason code.
 */
internal struct CCloseReason : RawRepresentable, Equatable {

    init(_ rawValue: UInt32) {
        self.rawValue = rawValue
    }

    init(rawValue: UInt32) {
        self.rawValue = rawValue
    }

    var rawValue: UInt32
}

/* Channel closed normally. */
internal var CCloseReason_Normal: CCloseReason { get { return CCloseReason(0) } }
/* Channel closed because timeout. */
internal var CCloseReason_Timeout: CCloseReason { get { return CCloseReason(1) } }
/* Channel closed because error ocurred. */
internal var CCloseReason_Error: CCloseReason { get { return CCloseReason(2) } }

/**
 * \~English
 * Carrier stream callbacks.
 *
 * Include stream status callback, stream data callback, and multiplexing
 * callbacks.
 */
internal struct CStreamCallbacks {

    /* Common callbacks */
    /**
     * \~English
     * Callback to report status of various stream operations.
     *
     * @param
     *      session     [in] The handle to the ElaSession.
     * @param
     *      stream      [in] The stream ID.
     * @param
     *      state       [in] Stream state defined in ElaStreamState.
     * @param
     *      context     [in] The application defined context data.
     */
    var state_changed: (@convention(c) (OpaquePointer?, Int32, UInt32, UnsafeMutableRawPointer?) -> Swift.Void)!


    /* Stream callbacks */
    /**
     * \~English
     * Callback will be called when the stream receives
     * incoming packet.
     *
     * If the stream enabled multiplexing mode, application will not
     * receive stream_data callback any more. All data will reported
     * as multiplexing channel data.
     *
     * @param
     *      session     [in] The handle to the ElaSession.
     * @param
     *      stream      [in] The stream ID.
     * @param
     *      data        [in] The received packet data.
     * @param
     *      len         [in] The received data length.
     * @param
     *      context     [in] The application defined context data.
     */
    var stream_data: (@convention(c) (OpaquePointer?, Int32, UnsafeRawPointer?, Int, UnsafeMutableRawPointer?) -> Swift.Void)!

    /* Multiplexer callbacks */
    /**
     * \~English
     * Callback will be called when new multiplexing channel request to open.
     *
     * @param
     *      session     [in] The handle to the ElaSession.
     * @param
     *      stream      [in] The stream ID.
     * @param
     *      channel     [in] The current channel ID.
     * @param
     *      cookie      [in] Application defined string data receive from peer.
     * @param
     *      context     [in] The application defined context data.
     *
     * @return
     *      True on success, or false if an error occurred.
     *      The channel will continue to open only this callback return true,
     *      otherwise the channel will be closed.
     */
    var channel_open: (@convention(c) (OpaquePointer?, Int32, Int32, UnsafePointer<Int8>?, UnsafeMutableRawPointer?) -> Bool)!

    /**
     * \~English
     * Callback will be called when new multiplexing channel opened.
     *
     * @param
     *      session     [in] The handle to the ElaSession.
     * @param
     *      stream      [in] The stream ID.
     * @param
     *      channel     [in] The current channel ID.
     * @param
     *      context     [in] The application defined context data.
     */
    var channel_opened: (@convention(c) (OpaquePointer?, Int32, Int32, UnsafeMutableRawPointer?) -> Swift.Void)!

    /**
     * \~English
     * Callback will be called when channel close.
     *
     * @param
     *      session     [in] The handle to the ElaSession.
     * @param
     *      stream      [in] The stream ID.
     * @param
     *      channel     [in] The current channel ID.
     * @param
     *      reason      [in] Channel close reason code, defined in CloseReason.
     * @param
     *      context     [in] The application defined context data.
     */
    var channel_close: (@convention(c) (OpaquePointer?, Int32, Int32, UInt32, UnsafeMutableRawPointer?) -> Swift.Void)!

    /**
     * \~English
     * Callback will be called when channel received incoming data.
     *
     * @param
     *      session     [in] The handle to the ElaSession.
     * @param
     *      stream      [in] The stream ID.
     * @param
     *      channel     [in] The current channel ID.
     * @param
     *      data        [in] The received data.
     * @param
     *      len         [in] The received data length.
     * @param
     *      context     [in] The application defined context data.
     *
     * @return
     *      True on success, or false if an error occurred.
     *      If this callback return false, the channel will be closed
     *      with CloseReason_Error.
     */
    var channel_data: (@convention(c) (OpaquePointer?, Int32, Int32, UnsafeRawPointer?, Int, UnsafeMutableRawPointer?) -> Bool)!

    /**
     * \~English
     * Callback will be called when remote peer ask to pend data sending.
     *
     * @param
     *      session     [in] The handle to the ElaSession.
     * @param
     *      stream      [in] The stream ID.
     * @param
     *      channel     [in] The current channel ID.
     * @param
     *      context     [in] The application defined context data.
     */
    var channel_pending: (@convention(c) (OpaquePointer?, Int32, Int32, UnsafeMutableRawPointer?) -> Swift.Void)!

    /**
     * \~English
     * Callback will be called when remote peer ask to resume data sending.
     *
     * @param
     *      session     [in] The handle to the ElaSession.
     * @param
     *      stream      [in] The stream ID.
     * @param
     *      channel     [in] The current channel ID.
     * @param
     *      context     [in] The application defined context data.
     */
    var channel_resume: (@convention(c) (OpaquePointer?, Int32, Int32, UnsafeMutableRawPointer?) -> Swift.Void)!

    init() {}
}

/**
 * \~English
 * Add a new stream to session.
 *
 * Carrier stream supports several underlying transport mechanisms:
 *
 *   - Plain/encrypted UDP data gram protocol
 *   - Plain/encrypted TCP like reliable stream protocol
 *   - Multiplexing over UDP
 *   - Multiplexing over TCP like reliable protocol
 *
 *  Application can use options to specify the new stream mode. Data
 *  transferred on stream is defaultly encrypted.  Multiplexing over UDP can
 *  not provide reliable transport.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      type        [in] The stream type defined in ElaStreamType.
 * @param
 *      options     [in] The stream mode options. options are constructed
 *                       by a bitwise-inclusive OR of flags from the
 *                       following list:
 *
 *                       - ELA_STREAM_PLAIN
 *                         Plain mode.
 *                       - ELA_STREAM_RELIABLE
 *                         Reliable mode.
 *                       - ELA_STREAM_MULTIPLEXING
 *                         Multiplexing mode.
 *                       - ELA_STREAM_PORT_FORWARDING
 *                         Support portforwarding over multiplexing.
 *
 * @param
 *      callbacks   [in] The Application defined callback functions in
 *                       ElaStreamCallbacks.
 * @param
 *      context     [in] The application defined context data.
 *
 * @return
 *      Return stream id on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_session_add_stream")
internal func ela_session_add_stream(_ session: OpaquePointer!,
                                     _ type: CStreamType,
                                     _ options: Int32,
                                     _ callbacks: UnsafeMutablePointer<CStreamCallbacks>!,
                                     _ context: UnsafeMutableRawPointer!) -> Int32

/**
 * \~English
 * Remove a stream from session.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream id to be removed.
 *
 * @return
 *      0 on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_session_remove_stream")
internal func ela_session_remove_stream(_ session: OpaquePointer!,
                                        _ stream: Int32) -> Int32

/**
 * \~English
 * Add a new portforwarding service to session.
 *
 * The registered services can be used by remote peer in portforwarding
 * request.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      service     [in] The new service name, should be unique
 *                       in session scope.
 * @param
 *      protocol    [in] The protocol of the service.
 * @param
 *      host        [in] The host name or ip of the service.
 * @param
 *      port        [in] The port of the service.
 *
 * @return
 *      0 on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_session_add_service")
internal func ela_session_add_service(_ session: OpaquePointer!,
                                      _ service: UnsafePointer<Int8>!,
                                      _ protocol: CPortForwardingProtocol,
                                      _ host: UnsafePointer<Int8>!,
                                      _ port: UnsafePointer<Int8>!) -> Int32

/**
 * \~English
 * Remove a portforwarding service to session.
 *
 * This function has not effect on existing portforwarings.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      service     [in] The service name.
 */
@_silgen_name("ela_session_remove_service")
internal func ela_session_remove_service(_ session: OpaquePointer!,
                                         _ service: UnsafePointer<Int8>!)

/**
 * \~English
 * Get the carrier stream type.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      type        [out] The stream type defined in ElaStreamType.
 *
 * @return
 *      0 on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_get_type")
internal func ela_stream_get_type(_ session: OpaquePointer!,
                                  _ stream: Int32,
                                  _ type: UnsafeMutablePointer<CStreamType>!) -> Int32

/**
 * \~English
 * Get the carrier stream current state.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      state       [out] The stream state defined in ElaStreamState.
 *
 * @return
 *      0 on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_get_state")
internal func ela_stream_get_state(_ session: OpaquePointer!,
                                   _ stream: Int32,
                                   _ state: UnsafeMutablePointer<CStreamState>) -> Int32

/**
 * \~English
 * Get the carrier stream transport information.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      info        [out] The stream transport information defined in
 *                        ElaTransportInfo.
 *
 * @return
 *      0 on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_get_transport_info")
internal func ela_stream_get_transport_info(_ session: OpaquePointer!,
                                            _ stream: Int32,
                                            _ info: UnsafeMutablePointer<CTransportInfo>!) -> Int32

/**
 * \~English
 * Send outgoing data to remote peer.
 *
 * If the stream is in multiplexing mode, application can not
 * call this function to send data. If this function is called
 * on multiplexing mode stream, it will return error.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      data        [in] The outgoing data.
 * @param
 *      len         [in] The outgoing data length.
 *
 * @return
 *      Sent bytes on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_write")
internal func ela_stream_write(_ session: OpaquePointer!,
                               _ stream: Int32,
                               _ data: UnsafeRawPointer!,
                               _ len: Int) -> Int

/**
 * \~English
 * Open a new channel on multiplexing stream.
 *
 * If the stream is not multiplexing this function will fail.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      cookie      [in] Application defined data pass to remote peer.
 *
 * @return
 *      New channel ID on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_open_channel")
internal func ela_stream_open_channel(_ session: OpaquePointer!,
                                      _ stream: Int32,
                                      _ cookie: UnsafePointer<Int8>!) -> Int32


/**
 * \~English
 * Close a new channel on multiplexing stream.
 *
 * If the stream is not multiplexing this function will fail.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      channel     [in] The channel ID.
 *
 * @return
 *      0 on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_close_channel")
internal func ela_stream_close_channel(_ session: OpaquePointer!,
                                       _ stream: Int32,
                                       _ channel: Int32) -> Int32

/**
 * \~English
 * Send outgoing data to remote peer.
 *
 * If the stream is not multiplexing this function will fail.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      channel     [in] The channel ID.
 * @param
 *      data        [in] The outgoing data.
 * @param
 *      len         [in] The outgoing data length.
 *
 * @return
 *      Sent bytes on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_write_channel")
internal func ela_stream_write_channel(_ session: OpaquePointer!,
                                       _ stream: Int32,
                                       _ channel: Int32,
                                       _ data: UnsafeRawPointer!,
                                       _ len: Int) -> Int

/**
 * \~English
 * Request remote peer to pend channel data sending.
 *
 * If the stream is not multiplexing this function will fail.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      channel     [in] The channel ID.
 *
 * @return
 *      0 on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_pend_channel")
internal func ela_stream_pend_channel(_ session: OpaquePointer!,
                                      _ stream: Int32,
                                      _ channel: Int32) -> Int32
/**
 * \~English
 * Request remote peer to resume channel data sending.
 *
 * If the stream is not multiplexing this function will fail.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      channel     [in] The channel ID.
 *
 * @return
 *      0 on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_resume_channel")
internal func ela_stream_resume_channel(_ session: OpaquePointer!,
                                        _ stream: Int32,
                                        _ channel: Int32) -> Int32

/**
 * \~English
 * Open a portforwarding to remote service over multiplexing.
 *
 * If the stream is not multiplexing this function will fail.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      service     [in] The remote service name.
 * @param
 *      protocol    [in] Portforwarding protocol.
 * @param
 *      host        [in] Local host or ip to binding.
 *                       If host is NULL, portforwarding will bind to 127.0.0.1.
 * @param
 *      port        [in] Local port to binding, can not be NULL.
 *
 * @return
 *      Portforwarding ID on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_open_port_forwarding")
internal func ela_stream_open_port_forwarding(_ session: OpaquePointer!,
                                              _ stream: Int32,
                                              _ service: UnsafePointer<Int8>!,
                                              _ protocol: CPortForwardingProtocol,
                                              _ host: UnsafePointer<Int8>!,
                                              _ port: UnsafePointer<Int8>!) -> Int32

/**
 * \~English
 * Close a portforwarding to remote service over multiplexing.
 *
 * If the stream is not multiplexing this function will fail.
 *
 * @param
 *      session     [in] The handle to the ElaSession.
 * @param
 *      stream      [in] The stream ID.
 * @param
 *      portforwarding  [in] The portforwarding ID.
 *
 * @return
 *      0 on success, or -1 if an error occurred.
 *      The specific error code can be retrieved by calling
 *      ela_get_error().
 */
@_silgen_name("ela_stream_close_port_forwarding")
internal func ela_stream_close_port_forwarding(_ session: OpaquePointer!,
                                               _ stream: Int32,
                                               _ portforwarding: Int32) -> Int32
