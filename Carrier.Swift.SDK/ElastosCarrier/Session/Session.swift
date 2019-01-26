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

@inline(__always)
private func getCurrentStream(_ cctxt: UnsafeMutableRawPointer) -> CarrierStream {
    return Unmanaged<CarrierStream>.fromOpaque(cctxt).takeUnretainedValue()
}

func onStateChanged(_: OpaquePointer?, cstream: Int32, cstate: UInt32,
                    cctxt: UnsafeMutableRawPointer?) {

    let stream  = getCurrentStream(cctxt!)

    guard let handler = stream.delegate else {
        return
    }

    let state = CarrierStreamState(rawValue: Int(cstate))!

    handler.streamStateDidChange?(stream, state)
}

func onStreamData(_: OpaquePointer?, cstream: Int32,
                  cdata: UnsafeRawPointer?, clen: Int,
                  cctxt: UnsafeMutableRawPointer?) {

    let stream  = getCurrentStream(cctxt!)

    guard let handler = stream.delegate else {
        return
    }

    autoreleasepool {
        let data = Data(bytes: cdata!, count: clen)

        handler.didReceiveStreamData?(stream, data)
    }
}

func onChannelOpen(_: OpaquePointer?, cstream:Int32,
                   cchannel: Int32, ccookie: UnsafePointer<Int8>?,
                   cctxt: UnsafeMutableRawPointer?) -> Bool {

    let stream  = getCurrentStream(cctxt!)

    guard let handler = stream.delegate else {
        return false
    }

    let cookie = String(cString: ccookie!)

    return handler.shouldOpenNewChannel?(stream,
                                         Int(cchannel),
                                         cookie) ?? true
}

func onChannelOpened(_: OpaquePointer?, cstream:Int32,
                     cchannel: Int32, cctxt: UnsafeMutableRawPointer?) {

    let stream  = getCurrentStream(cctxt!)

    guard let handler = stream.delegate else {
        return
    }

    handler.didOpenNewChannel?(stream, Int(cchannel))
}

func onChannelClose(_: OpaquePointer?, cstream:Int32,
                    cchannel: Int32, creason: UInt32,
                    cctxt: UnsafeMutableRawPointer?)
{
    let stream  = getCurrentStream(cctxt!)

    guard let handler = stream.delegate else {
        return
    }

    let reason = CloseReason(rawValue: Int(creason))!

    handler.didCloseChannel?(stream, Int(cchannel), reason)
}

func onChannelData(_ : OpaquePointer?, cstream:Int32,
                   cchannel: Int32, cdata: UnsafeRawPointer?, clen: Int,
                   cctxt: UnsafeMutableRawPointer?) -> Bool
{
    let stream  = getCurrentStream(cctxt!)

    guard let handler = stream.delegate else {
        return false
    }

    var result = true

    autoreleasepool {
        let data = Data(bytes: cdata!, count: clen)

        result = handler.didReceiveChannelData?(stream,
                                                Int(cchannel),
                                                data) ?? true
    }

    return result
}

func onChannelPending(_ : OpaquePointer?, cstream:Int32,
                      cchannel: Int32, cctxt: UnsafeMutableRawPointer?)
{
    let stream  = getCurrentStream(cctxt!)

    guard let handler = stream.delegate else {
        return
    }

    handler.channelPending?(stream, Int(cchannel))
}

func onChannelResume(_ : OpaquePointer?, cstream:Int32,
                     cchannel: Int32, cctxt: UnsafeMutableRawPointer?)
{
    let stream  = getCurrentStream(cctxt!)

    guard let handler = stream.delegate else {
        return
    }

    handler.channelResumed?(stream, Int(cchannel))
}

@inline(__always) private func TAG() -> String { return "CarrierSession" }

public typealias CarrierSessionRequestCompleteHandler = (_ session: CarrierSession,
                _ status: Int, _ reason: String?, _ sdp: String?) -> Void

/// The class representing the carrier session conversation.
@objc(ELACarrierSession)
public class CarrierSession: NSObject {

    internal var csession: OpaquePointer
    private  var streams : Dictionary<Int, CarrierStream>
    private  var to: String
    private  var didClose: Bool

    internal init(_ csession: OpaquePointer, _ to: String) {
        self.csession = csession
        self.streams  = [Int: CarrierStream]()
        self.to = to
        self.didClose = false
        super.init()
    }

    deinit {
        close()
    }

    /// Close a session to friend. All resources include streams, channels,
    /// portforwardings associated with current session will be destroyed.
    public func close() {
        objc_sync_enter(self)
        if !didClose {
            Log.d(TAG(), "Begin to close native session instance ...")

            ela_session_close(csession)
            didClose = true

            Log.d(TAG(), "Native session instance closed nicely")
        }
        objc_sync_exit(self)
    }

    /// Get remote peer id.
    ///
    /// - Returns: The remote peer userid or userid@nodeid
    public func getPeer() -> String {
        return to;
    }

    /// TODO: Add setUserdata & getUserdata

    /// Send session request to the friend.
    ///
    /// - Parameters:
    ///   - handler: A handler to receive the session response
    ///
    /// - Throws: CarrierError
    @objc(sendInviteRequestWithResponseHandler:error:)
    public func sendInviteRequest(handler: @escaping CarrierSessionRequestCompleteHandler) throws {

        let cb: CSessionRequestCompleteCallback = {
                (_, _, cstatus, creason, csdp, clen, cctxt) in

                let wctxt = Unmanaged<AnyObject>.fromOpaque(cctxt!)
                    .takeRetainedValue() as! [AnyObject?]

                let session = wctxt[0] as! CarrierSession
                let handler = wctxt[1] as! CarrierSessionRequestCompleteHandler

                let status = Int(cstatus)
                var reason: String?
                var sdp: String?

                if status != 0 {
                    reason = String(cString: creason!)
                } else {
                    sdp = String(cString: csdp!)
                }

                handler(session, status, reason, sdp)
        }

        Log.d(TAG(), "Begin to request to invite session to \(to) ...")

        let wcontext : [AnyObject?] = [self, handler as AnyObject]
        let manager = Unmanaged.passRetained(wcontext as AnyObject);
        let cctxt = manager.toOpaque()
        let result = ela_session_request(csession, nil, cb, cctxt)

        guard result >= 0 else {
            manager.release()
            let errno = getErrorCode()
            Log.e(TAG(), "Request to invite session error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Sended session invite request to \(to)")
    }

    /// Reply the session request from friend.
    ///
    /// This function will send a session response to friend.
    ///
    /// - Parameters:
    ///   - status: The status code of the response. 0 is success, 
    ///             otherwise is error
    ///   - reason: The error message if status is error, or nil if success
    ///
    /// - Throws: CarrierError
    @objc(replyInviteRequestWithStatus:reason:error:)
    public func replyInviteRequest(with status: Int, reason: String?) throws {
        if status != 0 && reason == nil {
            throw CarrierError.InvalidArgument
        }

        if status == 0 {
            Log.d(TAG(), "Attempt to confirm session invite request to \(to).")
        } else {
            Log.d(TAG(), "Attempt to refuse sesion invite request to \(to)" +
                " with status \(status) and reason \(reason!)")
        }

        let creason: UnsafeMutablePointer<Int8>?
        if (reason != nil) {
            creason = reason!.withCString() { (ptr) in
                return strdup(ptr)
            }
        } else {
            creason = nil
        }

        defer {
            if creason != nil {
                free(creason)
            }
        }

        let result = ela_session_reply_request(csession, nil, Int32(status), creason)
        guard result >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Reply session invite request error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        if status == 0 {
            Log.d(TAG(), "Confirmed the session invite requst to \(to)")
        } else {
            Log.d(TAG(), "Refused the sesion invite to \(to) with reason \(reason!)")
        }
    }

    /// Begin to start a session.
    ///
    /// All streams in current session will try to connect with remote friend,
    /// The stream status will update to application by stream's 
    /// `CarrierStreamDelegate`.
    ///
    /// - Parameter sdp: The remote user's SDP. 
    ///                  Reference: https://tools.ietf.org/html/rfc4566
    ///
    /// - Throws: CarrierError
    public func start(remoteSdp sdp: String) throws {

        Log.d(TAG(), "Begin to start session with remote sdp: \(sdp) ...")

        let result = sdp.withCString() { (ptr) -> Int32 in
            return ela_session_start(csession, ptr, sdp.utf8CString.count)
        }

        guard result >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Start to establish session error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Session to \(to) started")
    }

    /// Add a new stream to session.
    ///
    /// Carrier stream supports several underlying transport mechanisms:
    ///
    ///   - Plain/encrypted UDP data gram protocol
    ///   - Plain/encrypted TCP like reliable stream protocol
    ///   - Multiplexing over UDP
    ///   - Multiplexing over TCP like reliable protocol
    ///
    ///  Application can use options to specify the new stream mode.
    ///  Multiplexing over UDP can not provide reliable transport.
    ///
    /// - Parameters:
    ///   - type: The stream type defined in CarrierStreamType
    ///   - options: The stream mode options
    ///   - delegate: The Application defined protocol defined in
    ///              `CarrierStreamDelegate`
    ///
    /// - Returns: The new added carrier stream
    ///
    /// - Throws: CarrierError
    public func addStream(type: CarrierStreamType,
                          options: CarrierStreamOptions,
                          delegate: CarrierStreamDelegate) throws -> CarrierStream {

        let ctype  = convertCarrierStreamTypeToCStreamType(type)

        var callbacks = CStreamCallbacks()
        callbacks.state_changed   = onStateChanged
        callbacks.stream_data     = onStreamData
        callbacks.channel_open    = onChannelOpen
        callbacks.channel_opened  = onChannelOpened
        callbacks.channel_close   = onChannelClose
        callbacks.channel_data    = onChannelData
        callbacks.channel_pending = onChannelPending
        callbacks.channel_resume  = onChannelResume

        let stream = CarrierStream(self.csession, type)
        stream.delegate = delegate

        Log.d(TAG(), "Begin to add a new stream with type \(type)")

        let cctxt = Unmanaged.passUnretained(stream).toOpaque()
        let streamId = ela_session_add_stream(csession, ctype, options.rawValue,
                                              &callbacks, cctxt)

        guard streamId >= 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Add a stream to session error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Added a stream to session in success\n")

        stream.streamId = Int(streamId)
        self.streams[stream.streamId] = stream

        return stream
    }

    /// Remove a stream from session
    ///
    /// - Parameter stream: The carrier stream to be removed
    ///
    /// - Throws: CarrierError
    @objc(removeStream:error:)
    public func removeStream(stream: CarrierStream) throws {

        let streamId = stream.streamId

        Log.d(TAG(), "Begin to remove stream \(streamId) from session.")

        let result = ela_session_remove_stream(csession, Int32(streamId))

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(TAG(), "Remove stream \(streamId) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        self.streams.removeValue(forKey: streamId)
        Log.d(TAG(), "Stream \(streamId) was removed from session.")
    }

    /// Add a new portforwarding service to session.
    ///
    /// The registered services can be used by remote peer in portforwarding
    /// request.
    ///
    /// - Parameters:
    ///   - serviceName: The new service name, should be unique in session scope
    ///   - proto: The protocol of the service
    ///   - host: The host name or ip of the service
    ///   - port: The port of the service
    ///
    /// - Throws: CarrierError
    @objc(addServiceWithName:protocol:host:port:error:)
    public func addService(serviceName: String, protocol proto: PortForwardingProtocol,
                           host: String = "localhost", port: String) throws {

        Log.d(TAG(), "Begin too add service \(serviceName) listening on: ",
            "\(proto):\\\(host):\(port)")

        let cproto = convertPortForwardingProtocolToCPortForwardingProtocol(proto)
        let result = serviceName.withCString() { (cservice) -> Int32 in
            return host.withCString() { (chost) -> Int32 in
                return port.withCString() { (cport) -> Int32 in
                    return ela_session_add_service(csession, cservice,
                                                   cproto, chost, cport)
                }
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(TAG(), "Add service \(serviceName) to session error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG(), "Service \(serviceName) was added to this session, and" +
            " now listening on \(proto):\\\(host):\(port)")
    }

    /// Remove a portforwarding server to session.
    ///
    /// This function has not effect on existing portforwarings.
    ///
    /// - Parameter serviceName: The service name
    @objc(removeServiceWithName:)
    public func removeService(serviceName: String) {

        serviceName.withCString() { (ptr) in
            ela_session_remove_service(csession, ptr)
        }

        Log.d(TAG(), "Service \(serviceName) was removed from session.")
    }
}

