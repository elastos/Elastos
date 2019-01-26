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

@inline(__always) private func TAG() -> String { return "CarrierSessionManager" }

public typealias CarrierSessionRequestHandler = (_ carrier: Carrier,
                                       _ from: String, _ sdp: String) -> Void

/// The class representing carrier session manager.
@objc(ELACarrierSessionManager)
public class CarrierSessionManager: NSObject {

    private static var sessionMgr: CarrierSessionManager?

    private var carrier: Carrier?
    private var handler: CarrierSessionRequestHandler?
    private var didCleanup: Bool

    /// Get a carrier session manager instance.
    ///
    /// This function is convinience way to get instance without interest to
    /// session request from friends.
    ///
    /// - Parameters:
    ///   - carrier: Carrier node instance
    ///   - options: The options to set for carrier session manager
    ///
    /// - Throws: CarrierError
    @objc(initializeSharedInstance:error:)
    public static func initializeSharedInstance(carrier: Carrier) throws {
        if (sessionMgr != nil && sessionMgr!.carrier != carrier) {
            sessionMgr!.cleanup()
        }

        if (sessionMgr == nil) {
            Log.d(TAG(), "Begin to initialize native carrier session manager...")

            var result = ela_session_init(carrier.ccarrier)
            guard result >= 0 else {
                let errno = getErrorCode()
                Log.e(TAG(), "Initialize native session manager error:0x%X", errno)
                throw CarrierError.FromErrorCode(errno: errno)
            }

            Log.d(TAG(), "The native carrier session manager initialized.")

            result = ela_session_set_callback(carrier.ccarrier, nil, nil, nil)
            guard result >= 0 else {
                let errno = getErrorCode()
                Log.e(TAG(), "Set session callback error: 0x%x", errno)
                ela_session_cleanup(carrier.ccarrier)
                throw CarrierError.FromErrorCode(errno: errno)
            }

            sessionMgr = CarrierSessionManager(carrier)
            sessionMgr!.didCleanup = false

            Log.i(TAG(), "Native carrier session manager instance created.");
        }
    }

    /// Get a carrier session manager instance.
    ///
    /// - Parameters:
    ///   - carrier: Carrier node instance
    ///   - options: The options to set for carrier session manager.
    ///   - handler: The handler for carrier session manager to process session
    ///              request from friends.
    ///
    /// - Throws: CarrierError
    @objc(initializeSharedInstance:sessionRequestHandler:error:)
    public static func initializeSharedInstance(carrier: Carrier,
                       sessionRequestHandler handler: @escaping CarrierSessionRequestHandler) throws {
        if (sessionMgr != nil && sessionMgr!.carrier != carrier) {
            sessionMgr!.cleanup()
        }

        if (sessionMgr == nil) {

            Log.d(TAG(), "Begin to initialize native carrier session manager...")

            let sessionManager = CarrierSessionManager(carrier)
            sessionManager.handler = handler

            var result = ela_session_init(carrier.ccarrier)

            guard result >= 0 else {
                let errno = getErrorCode()
                Log.e(TAG(), "Initialize native session manager error: 0x%X", errno)
                throw CarrierError.FromErrorCode(errno: errno)
            }

            let cb: CSessionRequestCallback = { (_, _, cfrom, csdp, _, cctxt) in
                let manager = Unmanaged<CarrierSessionManager>
                        .fromOpaque(cctxt!).takeUnretainedValue()

                let carrier = manager.carrier
                let handler = manager.handler

                let from = String(cString: cfrom!)
                let  sdp = String(cString: csdp!)

                handler!(carrier!, from, sdp)

            }
            let cctxt = Unmanaged.passUnretained(sessionManager).toOpaque()

            result = ela_session_set_callback(carrier.ccarrier, nil, cb, cctxt);
            guard result >= 0 else {
                let errno = getErrorCode()
                Log.e(TAG(), "Set session callback error: 0x%X", errno)
                ela_session_cleanup(carrier.ccarrier)
                throw CarrierError.FromErrorCode(errno: errno)
            }

            Log.d(TAG(), "The native carrier session manager initialized.")

            sessionManager.didCleanup = false
            sessionMgr = sessionManager

            Log.i(TAG(), "Native carrier session manager instance created.");
        }
    }

    /// Get a carrier session manager instance.
    ///
    /// - Returns: The carrier session manager or nil
    public static func sharedInstance() -> CarrierSessionManager? {
        return sessionMgr;
    }

    private init(_ carrier: Carrier) {
        self.carrier = carrier
        self.didCleanup = true
        super.init()
    }

    deinit {
        cleanup()
    }

    ///  Clean up carrier session manager.
    public func cleanup() {

        objc_sync_enter(self)
        if !didCleanup {
            Log.d(TAG(), "Begin clean up native carrier session manager ...")

            ela_session_cleanup(carrier!.ccarrier)
            carrier = nil
            CarrierSessionManager.sessionMgr = nil
            didCleanup = true

            Log.i(TAG(), "Native carrier session managed cleanuped.")
        }
        objc_sync_exit(self)
    }

    /// Create a new session converstation to the specified friend.
    ///
    /// The session object represent a conversation handle to a friend.
    ///
    /// - Parameters:
    ///   - target:    The target id.
    ///
    /// - Returns: The new CarrierSession
    ///
    /// - Throws: CarrierError
    public func createSession(to target: String)
        throws -> CarrierSession {

        let ctmp = target.withCString { (ptr) -> OpaquePointer? in
            return ela_session_new(carrier!.ccarrier, ptr)
        }

        guard ctmp != nil else {
            let errno = getErrorCode()
            Log.e(TAG(), "Open session conversation to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.i(TAG(), "An new session to \(target) created locally.")

        return CarrierSession(ctmp!, target)
    }
}
