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

@inline(__always) private func TAG() -> String { return "CarrierStream" }

/// The class representing carrier stream.
@objc(ELACarrierStream)
public class CarrierStream: NSObject {

    private var  csession: OpaquePointer
    private var      type: CarrierStreamType;

    internal var streamId: Int
    
    internal weak var delegate: CarrierStreamDelegate?

    internal init(_ csession: OpaquePointer, _ type: CarrierStreamType) {
        self.csession = csession
        self.streamId = -1
        self.type = type
        super.init()
    }

    /// Get the carrier stream type.
    ///
    /// - Returns: The stream type defined in CarrierStreamType
    ///
    public func getType() -> CarrierStreamType {
        return type
    }

    /// TODO: add getState
    
    @objc(getTransportInfo:)
    public func getTransportInfo() throws -> CarrierTransportInfo {
        var cinfo = CTransportInfo()
        let result = ela_stream_get_transport_info(csession,
                                                   Int32(streamId),
                                                   &cinfo)

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(TAG(), "Get transport info error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        let info = convertCTransportInfoToCarrierTransportInfo(cinfo)
        Log.d(TAG(), "Current transport info: \(info)")
        return info
    }

    /// Send outgoing data to remote peer.
    ///
    /// If the stream is in multiplexing mode, application can not call this
    /// function to send data. If this function is called on multiplexing mode
    /// stream, it will throw Error.
    ///
    /// - Parameters:
    ///   - data: The ougoing data
    ///
    /// - Returns: Bytes of data sent on success
    ///
    /// - Throws: CarrierError
    ///
    @objc(writeData:error:)
    public func writeData(_ data: Data) throws -> NSNumber {

        let bytes = data.withUnsafeBytes() { (ptr) -> Int in
            return ela_stream_write(csession, Int32(streamId), ptr, data.count)
        }

        guard bytes > 0 else {
            let errno = getErrorCode()
            Log.e(TAG(), "Write data to stream \(streamId) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        return NSNumber(value: bytes)
    }

    /// Open a new channel on multiplexing stream.
    ///
    /// If the stream is not multiplexing this function will throw Error.
    ///
    /// - Parameters:
    ///   - cookie: The application defined data passed to remote peer.
    ///
    /// - Returns: New channel ID
    ///
    /// - Throws: CarrierError
    ///
    @objc(openChannelWithCookie:error:)
    public func openChannel(cookie: String)
        throws -> NSNumber {

        let channel = cookie.withCString() { (ptr) -> Int32 in
            return ela_stream_open_channel(csession, Int32(streamId), ptr)
        }

        guard channel >= 0 else {
            throw CarrierError.FromErrorCode(errno: getErrorCode())
        }

        Log.d(TAG(), "Channel \(channel) opened locally")

        return NSNumber(value: channel)
    }

    /// Close a new channel on multiplexing stream.
    ///
    /// If the stream is not multiplexing this function will throw Error.
    ///
    /// - Parameter:
    ///   - channel: The channel ID to close
    ///
    /// - Throws: CarrierError
    ///
    @objc(closeChannel:error:)
    public func closeChannel(_ channel: Int) throws {
        if channel <= 0 {
            throw CarrierError.InvalidArgument
        }

        let result = ela_stream_close_channel(csession, Int32(streamId),
                                              Int32(channel))

        guard result >= 0 else {
            throw CarrierError.FromErrorCode(errno: getErrorCode())
        }

        Log.d(TAG(), "Channel \(channel) closed.")
    }

    /// Send outgoing data to remote peer.
    ///
    /// If the stream is not multiplexing this function will throw Error.
    ///
    /// - Parameters:
    ///   - channel: The channel ID
    ///   - data: The outgoing data
    ///
    /// - Returns: Bytes of data sent on sucess
    ///
    /// - Throws: CarrierError
    ///
    @objc(writeChannel:data:error:)
    public func writeChannel(_ channel: Int, data: Data) throws -> NSNumber {
        if channel <= 0 {
            throw CarrierError.InvalidArgument
        }

        let bytes = data.withUnsafeBytes() { (cdata) -> Int in
            return ela_stream_write_channel(csession, Int32(streamId),
                                            Int32(channel),
                                            cdata, data.count)
        }

        guard bytes > 0 else {
            throw CarrierError.FromErrorCode(errno: getErrorCode())
        }

        return NSNumber(value: bytes)
    }

    /// Request remote peer to pend channel data sending.
    ///
    /// If the stream is not multiplexing this function will throw Error.
    ///
    /// - Parameters:
    ///   - channel: The channel ID
    ///
    /// - Throws: CarrierError
    ///
    @objc(pendChannel:error:)
    public func pendChannel(_ channel: Int) throws {
        if channel <= 0 {
            throw CarrierError.InvalidArgument
        }

        let result = ela_stream_pend_channel(csession, Int32(streamId),
                                             Int32(channel))

        guard result >= 0 else {
            throw CarrierError.FromErrorCode(errno: getErrorCode())
        }
    }

    /// Request remote peer to resume channel data sending.
    ///
    /// If the stream is not multiplexing this function will throw Error.
    ///
    /// - Parameters:
    ///   - channel: The channel ID
    ///
    /// - Throws: CarrierError
    ///
    @objc(resumeChannel:error:)
    public func resumeChannel(_ channel: Int) throws {
        if channel <= 0 {
            throw CarrierError.InvalidArgument
        }

        let result = ela_stream_resume_channel(csession, Int32(streamId),
                                               Int32(channel))

        guard result >= 0 else {
            throw CarrierError.FromErrorCode(errno: getErrorCode())
        }
    }

    /// Open a port forwarding to remote service over multiplexing.
    ///
    /// If the stream is not multiplexing this function will throw Error.
    ///
    /// - Parameters:
    ///   - service: The remote service name
    ///   - proto: Port forwarding protocol
    ///   - host: Local host or ip to bind. Defaultlly port forwarding
    ///           will bind to localhost
    ///   - port: Local port to bind
    ///
    /// - Returns: Port forwarding ID
    ///
    /// - Throws: CarrierError
    ///
    @objc(openPortForwardingForService:withProtocol:host:port:error:)
    public func openPortForwarding(service: String,
                                   ptotocol proto: PortForwardingProtocol,
                                   host: String = "localhost",
                                   port: String) throws -> NSNumber {

        let cproto = convertPortForwardingProtocolToCPortForwardingProtocol(proto)
        let pfId = service.withCString() { (cservice) -> Int32 in
            return host.withCString() { (chost) in
                return port.withCString() { (cport) in
                    return ela_stream_open_port_forwarding(csession,
                               Int32(streamId), cservice, cproto, chost, cport)
                }
            }
        }

        guard pfId > 0 else {
            throw CarrierError.FromErrorCode(errno: getErrorCode())
        }

        Log.d(TAG(), "Port forwarding \(pfId) to service \(service) created",
            ", and currently listning on \(proto)://\(host):\(port)")

        return NSNumber(value: pfId)
    }

    /// Close a port forwarding.
    ///
    /// If the stream is not multiplexing this function will throw Error.
    ///
    /// - Parameters:
    ///   - portForwarding: The port forwarding ID
    ///
    /// - Throws: CarrierError
    ///
    @objc(closePortForwarding:error:)
    public func closePortForwarding(_ portForwarding: Int) throws {

        if portForwarding <= 0 {
            throw CarrierError.InvalidArgument
        }

        let result = ela_stream_close_port_forwarding(csession,
                         Int32(streamId), Int32(portForwarding))

        guard result >= 0 else {
            throw CarrierError.FromErrorCode(errno: getErrorCode())
        }

        Log.d(TAG(), "Port forwarding \(portForwarding) closed")
    }
}
