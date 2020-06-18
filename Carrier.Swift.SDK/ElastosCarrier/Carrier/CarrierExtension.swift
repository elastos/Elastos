
import Foundation

@objc(ELACarrierExtension)
open class CarrierExtension: NSObject {

    public typealias CarrierExtensionInviteReplyCallback =
        (_ carrier: Carrier, _ from: String, _ status: Int, _ reason: String?,
        _ data: String?) -> Void

    public typealias CarrierExtensionInviteCallback =
        (_ carrier: Carrier, _ from: String, _ data: String?) -> Void

    private let TAG = "CarrierExtension"
    private var carrier: Carrier?
    private var handler: CarrierExtensionInviteCallback?
    private var didCleanup: Bool

    deinit {
        cleanup()
    }

    public init(_ carrier: Carrier) {
        self.carrier = carrier
        Log.i(TAG, "CarrierExtension instance created")
        self.didCleanup = true
        super.init()
    }

    @objc (turnServerInfo:)
    public func turnServerInfo() throws -> TurnServerInfo {
        var cinfo = CTurnServer()
        let result = ela_get_turn_server(carrier!.ccarrier!, &cinfo)
        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(TAG, "Get turn server error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        return TurnServerInfo.convertCTurnServerToTurnServerInfo(cinfo)
    }

    @objc(sendInviteFriendRequest:data:responseHandler:error:)
    public func sendInviteFriendRequest(to target: String, withData data: String, _ responseHandler: @escaping CarrierExtensionInviteReplyCallback) throws {
        guard !target.isEmpty else {
            throw CarrierError.InvalidArgument
        }
        Log.d(TAG, "Inviting friend " + target + "with greet data " + data)
        let cb: CExtensionInviteReplyCallback = {

            (_, cfrom, cstatus, creason, cdata, clen, cctxt) in

            let ectxt = Unmanaged<AnyObject>.fromOpaque(cctxt)
                .takeRetainedValue() as! [AnyObject?]

            let carrier = ectxt[0] as! Carrier
            let handler = ectxt[1] as! CarrierExtensionInviteReplyCallback

            let from = String(cString: cfrom)
            let status = Int(cstatus)
            var reason: String? = nil
            var _data : String? = nil

            if status != 0 {
                reason = String(cString: creason!)
            } else {
                _data = String(cString: cdata!)
            }

            handler(carrier, from, status, reason, _data)
        }

        let econtext: [AnyObject?] = [carrier, responseHandler as AnyObject]
        let unmanaged = Unmanaged.passRetained(econtext as AnyObject)
        let cctxt = unmanaged.toOpaque()
        
        let result = target.withCString { (cto) -> Int32 in
            return data.withCString { (cdata) -> Int32 in
                let len = data.utf8CString.count
                return extension_invite_friend(carrier!.ccarrier!, cto, cdata, len, cb, cctxt)
            }
        }
        guard result >= 0 else {
            unmanaged.release()
            let errno = getErrorCode()
            Log.e(TAG, "Invite friend to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(TAG, "Sended friend invite request to \(target).")
    }

    @objc(replyFriendInviteRequest:status:resson:data:error:)
    public func replyFriendInviteRequest(to target: String, withStatus status: Int, _ reason: String?, _ data: String?) throws {
        guard !target.isEmpty else {
            throw CarrierError.InvalidArgument
        }
        if status == 0 {
            Log.d(TAG, "Attempt to confirm friend invite to " + target + "with data \(data)")
        }
        else {
            Log.d(TAG, "Attempt to confirm friend invite to " + target + "with status \(data)" + "and reason \(reason)")
        }

        var creason: UnsafeMutablePointer<Int8>?
        var cdata: UnsafeMutablePointer<Int8>?
        var len: Int = 0

        let result = target.withCString { (cto) -> Int32 in
            if status != 0 {
                creason = reason!.withCString() { (ptr) in
                    return strdup(ptr)
                }
            } else {
                cdata = data!.withCString() { (ptr) in
                    return strdup(ptr)
                }

                len = data!.utf8CString.count
            }

            defer {
                if creason != nil {
                    free(creason)
                }
                if cdata != nil {
                    free(cdata)
                }
            }
            return extension_reply_friend_invite(carrier!.ccarrier!, cto, CInt(status), creason, cdata, len)
        }
        guard result >= 0 else {
            let errno = getErrorCode()
            throw CarrierError.FromErrorCode(errno: errno)
        }

        if status == 0 {
            Log.d(TAG, "Confirmed friend invite to " + target + "with data \(data)")
        }
        else {
            Log.d(TAG, "Refused friend invite to " + target + "with status \(data)" + "and reason \(reason)")
        }
    }

    @objc (registerExtension:error:)
    public func registerExtension(_ handler: @escaping CarrierExtensionInviteCallback) throws {

        let carrierExtension = CarrierExtension(carrier!)
        carrierExtension.handler = handler
        let cb: CExtensionInviteCallback = {

            (_, cfrom, cdata, clen, cctxt) in

            let ccarrierExtension = Unmanaged<CarrierExtension>.fromOpaque(cctxt!)
                .takeUnretainedValue()

            let carrier = ccarrierExtension.carrier
            let handle = ccarrierExtension.handler

            let from = String(cString: cfrom!)
            let _data : String? = String(cString: cdata)

            handle!(carrier!, from, _data)
        }

        let cctxt = Unmanaged.passRetained(carrierExtension).toOpaque()
        let result = extension_init(carrier!.ccarrier!, cb, cctxt)
        guard result >= 0 else {
            let errno = getErrorCode()
            throw CarrierError.FromErrorCode(errno: errno)
        }

        carrierExtension.didCleanup = false
    }

    @objc (cleanup)
    public func cleanup() {

        objc_sync_enter(self)
        if !didCleanup {
            extension_cleanup(carrier!.ccarrier!)
            carrier = nil
            didCleanup = true
        }
        objc_sync_exit(self)
    }
}

