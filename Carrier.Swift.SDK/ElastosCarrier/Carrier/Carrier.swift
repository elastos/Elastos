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

@inline(__always) internal func getErrorCode() -> Int {
    let error = ela_get_error()
    return Int(error & 0x7FFFFFFF)
}

/// The class representing carrier node.
@objc(ELACarrier)
public class Carrier: NSObject {

    public typealias CarrierFriendInviteResponseHandler =
        (_ carrier: Carrier, _ from: String, _ status: Int, _ reason: String?,
         _ data: String?) ->Void

    /// Carrier node App message max length.
    public static let MAX_APP_MESSAGE_LEN: Int = 2048

    private static let TAG: String = "Carrier"
    private static let MAX_ADDRESS_LEN: Int = 52;
    private static let MAX_ID_LEN: Int = 45

    private static var carrierInst: Carrier?

    internal var ccarrier: OpaquePointer?
    private  var didKill : Bool
    private  let semaph  : DispatchSemaphore

    internal var delegate: CarrierDelegate?

    internal var friends: [CarrierFriendInfo]

    /// Get current carrier node version.
    ///
    /// - Returns: The current carrier node version.
    public static func getVersion() -> String {
        return String(cString: ela_get_version())
    }

    /// Check if the carrier address ID is valid.
    ///
    /// - Parameter address: The carrier address to be check
    ///
    /// - Returns: True if carrier address is valid, otherwise false
    public static func isValidAddress(_ address: String) -> Bool {
        return (Base58.decode(address)?.count == 38)
    }

    /// Check if the carrier public ID is valid.
    ///
    /// - Parameter id: The carrier id to be check
    ///
    /// - Returns: True if carrier id is valid, otherwise false
    public static func isValidId(_ id: String) -> Bool {
        return (Base58.decode(id)?.count == 32)
    }

    /// Extract ID from the carrier node address.
    ///
    /// - Parameter address: The carrier node address.
    ///
    /// - Returns: Valid Id if carrier node address is valid, otherwise nil
    public static func getIdFromAddress(_ address: String) -> String? {
        let addr = Base58.decode(address);

        if addr?.count == 38 {
            return Base58.encode(Array(addr!.prefix(32)))
        } else {
            return nil
        }
    }

    /// Set log level for carrier node.
    /// Default level to control log output is `CarrierLogLevel.Info`
    ///
    /// - Parameter level: The log level
    public static func setLogLevel(_ level: CarrierLogLevel) {
        Log.setLevel(level)
        ela_log_init(convertCarrierLogLevelToCLogLevel(level), nil, nil)
    }

    /// Create node singleton instance. After initialize the instance,
    /// it's ready to start and therefore connect to the carrier network.
    ///
    /// - Parameters:
    ///   - options: The options to set for carrier node
    ///   - delegate: The delegate for carrier node to comply with
    ///
    /// - Throws: CarrierError
    public static func initializeInstance(options: CarrierOptions,
                                   delegate: CarrierDelegate) throws {
        if (carrierInst == nil) {
            Log.d(TAG, "Attempt to create native carrier instance ...")

            var copts = convertCarrierOptionsToCOptions(options);
            defer {
                cleanupCOptions(copts)
            }

            let carrier = Carrier(delegate)
            var chandler = getNativeHandlers()
            let cctxt = Unmanaged.passUnretained(carrier).toOpaque()
            let ccarrier = ela_new(&copts, &chandler, cctxt)

            guard ccarrier != nil else {
                let errno = getErrorCode()
                Log.e(TAG, "Create native carrier instance error: 0x%X", errno)
                throw CarrierError.FromErrorCode(errno: errno)
            }

            carrier.ccarrier = ccarrier
            carrier.didKill = false

            Log.i(TAG, "Native carrier node instance created.")
            carrierInst = carrier
        }
    }

    /// Get a carrier node singleton instance.
    ///
    /// - Returns: The carrier node instance or ni
    public static func getInstance() -> Carrier? {
        return carrierInst
    }

    private init(_ delegate: CarrierDelegate) {
        self.delegate = delegate
        self.didKill = true
        self.semaph = DispatchSemaphore(value: 0)
        self.friends = [CarrierFriendInfo]()
        super.init()
    }

    deinit {
        kill()
    }

    /// Start carrier node asynchronously to connect to carrier network.
    /// If the connection to network is successful, carrier node starts
    /// working.
    ///
    /// - Parameter iterateInterval: Internal loop interval, in milliseconds
    ///
    /// - Throws: CarrierError
    public func start(iterateInterval: Int = 0) throws {
        guard iterateInterval >= 0 else {
            throw CarrierError.InvalidArgument
        }

        let backgroundQueue = DispatchQueue(label: "org.elastos.queue",
                                            qos: .background, target: nil)
        weak var weakSelf = self

        backgroundQueue.async {
            Log.i(Carrier.TAG, "Native carrier node started.")
            _ = ela_run(weakSelf?.ccarrier, Int32(iterateInterval))
            Log.i(Carrier.TAG, "Native carrier node stopped.")

            DispatchQueue.global(qos: .background).async {
                weakSelf?.semaph.signal()
            }
        }
    }

    /// Disconnect carrier node from the server, and destroy all associated
    /// resources to carrier node instance.
    ///
    /// After calling the method, the carrier node instance becomes invalid,
    /// and can not be refered any more.
    public func kill() {
        objc_sync_enter(self)

        if (!didKill) {
            Log.d(Carrier.TAG, "Actively to kill native carrier node ...");

            ela_kill(ccarrier)
            ccarrier = nil
            semaph.wait()
            Carrier.carrierInst = nil
            delegate = nil
            didKill = true
            Log.i(Carrier.TAG, "Native carrier node killed and exited")
        }

        objc_sync_exit(self)
    }

    /// Get node address associated with carrier node instance.
    ///
    /// Returns: The node address
    public func getAddress() -> String {
        let len = Carrier.MAX_ADDRESS_LEN + 1
        var data = Data(count: len);

        let address = data.withUnsafeMutableBytes() {
            (ptr: UnsafeMutablePointer<Int8>) -> String in
            return String(cString: ela_get_address(ccarrier, ptr, len))
        }

        Log.d(Carrier.TAG, "Current carrier address: \(address)")
        return address;
    }

    /// Get node identifier associated with the carrier node instance.
    ///
    /// - Returns: The node identifier
    public func getNodeId() -> String {
        let len  = Carrier.MAX_ID_LEN + 1
        var data = Data(count: len)

        let nodeId = data.withUnsafeMutableBytes() {
            (ptr: UnsafeMutablePointer<Int8>) -> String in
            return String(cString: ela_get_nodeid(ccarrier, ptr, len))
        }

        Log.d(Carrier.TAG, "Current carrier NodeId: \(nodeId)")
        return nodeId
    }

    /// Get user identifier associated with the carrier node instance.
    ///
    /// - Returns: The user identifier
    public func getUserId() -> String {
        let len  = Carrier.MAX_ID_LEN + 1
        var data = Data(count: len)

        let userId = data.withUnsafeMutableBytes() {
            (ptr: UnsafeMutablePointer<Int8>) -> String in
                return String(cString: ela_get_userid(ccarrier, ptr, len))
        }

        Log.d(Carrier.TAG, "Current carrier UserId: \(userId)")
        return userId
    }

    /// Update the nospam for carrier node address
    ///
    /// Update the 4-byte nospam part of the Carrier address with host byte order
    /// expected. Nospam for Carrier address is used to eliminate spam friend
    /// request.
    ///
    /// - Parameter newNospam: The new nospam to address.
    ///
    /// - Throws: CarrierError
    public func setSelfNospam(_ newNospam: UInt32) throws {
        let result = ela_set_self_nospam(ccarrier, newNospam)

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Set self nospam error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.i(Carrier.TAG, "Current nospam updated.")
    }

    /// Get the nospam for Carrier address.
    ///
    /// Get the 4-byte nospam part of the Carrier address with host byte order
    /// expected. Nospam for Carrier address is used to eliminate spam friend
    /// request.
    ///
    /// - Returns: The current nospam.
    ///
    /// - Throws: CarrierError
    public func getSelfNospam() throws -> UInt32 {
        var nospam: UInt32 = 0
        let result = ela_get_self_nospam(ccarrier, &nospam)

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Get self nospam error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.i(Carrier.TAG, "Current node spam: \(nospam)")
        return nospam
    }

    /// Update self user information.
    ///
    /// After self user information changed, carrier node will update this
    /// information to network, and thereupon network broadcasts the change to
    //  all friends.

    /// - Parameter newUserInfo: The new user information to set
    ///
    /// - Throws: CarrierError
    public func setSelfUserInfo(_ newUserInfo: CarrierUserInfo) throws {
        var cinfo = convertCarrierUserInfoToCUserInfo(newUserInfo)
        let result = ela_set_self_info(ccarrier, &cinfo)

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Update self user infos error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.i(Carrier.TAG, "Current user information updated.")
    }

    /// Get self user information.
    ///
    /// - Returns: The current user information
    ///
    /// - Throws: CarrierError
    @objc(getSelfUserInfo:)
    public func getSelfUserInfo() throws -> CarrierUserInfo {
        var cinfo = CUserInfo()
        let result = ela_get_self_info(ccarrier, &cinfo)

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Get current user infos error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        let info = convertCUserInfoToCarrierUserInfo(cinfo)
        Log.d(Carrier.TAG, "Current user infos: \(info)")
        return info
    }

    /// Set self presence status
    ///
    /// - Parameter newPresence: The new presence status to friends.
    ///
    /// - Throws: CarrierError
    public func setSelfPresence(_ newPresence: CarrierPresenceStatus) throws {
        let presence = convertCarrierPresenceStatusToCPresenceStatus(newPresence)
        let result = ela_set_self_presence(ccarrier, presence)

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Set self presence error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "Self presence updated to be: \(newPresence)")
    }

    /// Get self presence status
    ///
    /// - Returns: The current presence status
    ///
    /// - Throws: CarrierError
    //@objc(getSelfPresence:)
    public func getSelfPresence() throws -> CarrierPresenceStatus {
        var cpresence = CPresenceStatus_None
        let result = ela_get_self_presence(ccarrier, &cpresence)

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Get self presence error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        let presence = convertCPresenceStatusToCarrierPresenceStatus(cpresence.rawValue)
        Log.d(Carrier.TAG, "Current self presence: \(presence)")
        return presence
    }

    /// Check if carrier node instance is being ready.
    ///
    /// - Returns: true if the carrier node instance is ready, or false if not
    public func isReady() -> Bool {
        return ela_is_ready(ccarrier)
    }

    /// Get current user's friend list
    ///
    /// - Returns: The list of friend information
    //
    /// - Throws: CarrierError
    @objc(getFriends:)
    public func getFriends() throws -> [CarrierFriendInfo] {

        let cb: CFriendsIterateCallback = { (cinfo, ctxt) in
            if cinfo != nil {
                let cFriendInfo = cinfo!.assumingMemoryBound(to: CFriendInfo.self).pointee
                let info = convertCFriendInfoToCarrierFriendInfo(cFriendInfo)
                ctxt!.assumingMemoryBound(to: [CarrierFriendInfo].self)
                    .pointee.append(info)
            }
            return true
        }

        var friends = [CarrierFriendInfo]()
        let result = withUnsafeMutablePointer(to: &friends) { (ptr) -> Int32 in
            let cctxt = UnsafeMutableRawPointer(ptr)
            return ela_get_friends(ccarrier, cb, cctxt)
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Get current user's friends error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "Current user's friends listed below: +++>>")
        for friend in friends {
            Log.d(Carrier.TAG, "\(friend)");
        }
        Log.d(Carrier.TAG, "<<+++")

        return friends
    }

    /// Get specified friend information.
    ///
    /// - Parameter friendId: The user identifier of friend
    ///
    /// - Returns: The friend information to user `friendId`
    ///
    /// - Throws: CarrierError
    @objc(getFriendInfoForFriend:error:)
    public func getFriendInfo(_ friendId: String) throws ->CarrierFriendInfo {
        var cinfo = CFriendInfo()
        let result = friendId.withCString { (cfriendId) -> Int32 in
            return ela_get_friend_info(ccarrier, cfriendId, &cinfo)
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Get infos of friend \(friendId) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        let info = convertCFriendInfoToCarrierFriendInfo(cinfo)
        Log.d(Carrier.TAG, "The infos of friend \(friendId): \(info)")
        return info
    }

    /// Set the label of the specified friend.
    ///
    /// The label of a friend is a private alias name for current user. 
    /// It can be seen by current user only, and has no impact to the target 
    /// friend itself.
    ///
    /// - Parameters:
    ///   - friendId: the friend's user identifier
    ///   - newLabel: the new label of specified friend
    ///
    /// - Throws: CarrierError
    @objc(setLabelForFriend:withLabel:error:)
    public func setFriendLabel(forFriend friendId: String, newLabel: String) throws {

        let result = friendId.withCString { (cfriendId) in
            return newLabel.withCString { (clabel) in
                return ela_set_friend_label(ccarrier, cfriendId, clabel);
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Set friend \(friendId)'s label error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "Friend \(friendId)'s label changed -> \(newLabel)")
    }

    /// Check if the user ID is friend.
    ///
    /// - Parameter userId: The userId to check
    ///
    /// - Returns: True if the user is friend, otherwise false
    @objc(isFriendWithUser:)
    public func isFriend(with userId: String) -> Bool {
        return userId.withCString { (ptr) -> Bool in
            return Bool(ela_is_friend(ccarrier, ptr))
        }
    }

    /// Attempt to add friend by sending a new friend request.
    ///
    /// This function will add a new friend with specific address, and then
    /// send a friend request to the target node.
    ///
    /// - Parameters:
    ///   - userId: The target user id
    ///   - hello: PIN for target user, or any application defined
    ///            content.
    ///
    /// - Throws: CarrierError
    public func addFriend(with userId: String,
                          withGreeting hello: String) throws {

        Log.d(Carrier.TAG, "Begin to add be friend with user \(userId)" +
            "by sending friend request with hello: \(hello) to \(userId)")

        let result = userId.withCString { (cuserId) in
            return hello.withCString { (chello) in
                return ela_add_friend(ccarrier, cuserId, chello)
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Send friend request to user \(userId)" +
                " error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "Sended a friend request to user \(userId).")
    }

    /// Accept the friend request.
    ///
    /// This function is used to add a friend in response to a friend request.
    ///
    /// - Parameters:
    ///   - userId: The user id who want be friend with current user
    ///
    /// - Throws: CarrierError
    public func acceptFriend(with userId: String) throws {

        let result = userId.withCString { (cuserId) -> Int32 in
            return ela_accept_friend(ccarrier, cuserId)
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Accept user \(userId) as be friend " +
                "error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "Accepted user \(userId) as friend.")
    }

    /// Remove friendship with the specified friend.
    ///
    /// If all correct, Carrier network will clean the friend relationship,
    /// and send friend removed message to both.
    ///
    /// - Parameter friendId: The target user id to remove friendship.
    ///
    /// - Throws: CarrierError
    public func removeFriend(_ friendId: String) throws {
        let result = friendId.withCString { (cfriendId) -> Int32 in
            return ela_remove_friend(ccarrier, cfriendId)
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Remove friend \(friendId) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "Friend \(friendId) was removed")
    }

    /// Send a message to the specified friend.
    ///
    /// The message length may not exceed `MAX_APP_MESSAGE_LEN`, and message
    /// itself should be text-formatted. Larger messages must be splitted by
    /// application and sent as separate messages. Other nodes can reassemble 
    /// the fragments.
    ///
    /// - Parameters:
    ///   - target: The target id
    ///   - msg: The message content defined by application in string type.
    ///
    /// - Throws: CarrierError
    public func sendFriendMessage(to target: String, withMessage msg: String) throws {
        let result = target.withCString {(cto) in
            return msg.withCString { (cmsg) -> Int32 in
                let len: Int = msg.utf8CString.count
                return ela_send_friend_message(ccarrier, cto, cmsg, len)
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Send message to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "Sended message: \(msg) to \(target).")
    }

    /// Send a message to the specified friend.
    ///
    /// The message length may not exceed `MAX_APP_MESSAGE_LEN`, and message
    /// itself should be text-formatted. Larger messages must be splitted by
    /// application and sent as separate messages. Other nodes can reassemble
    /// the fragments.
    ///
    /// - Parameters:
    ///   - target: The target id
    ///   - msg: The message content defined by application in Data type.
    ///
    /// - Throws: CarrierError
    public func sendFriendMessage(to target: String, withData data: Data) throws {
        let result = target.withCString { (cto) in
            return data.withUnsafeBytes{ (cdata) -> Int32 in
                return ela_send_friend_message(ccarrier, cto, cdata, data.count)
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Send message to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "Sended message: \(data) to \(target).")
    }

    /// Send invite request to the specified friend
    ///
    /// Application can attach the application defined data with in the invite
    /// request, and the data will send to target friend.
    ///
    /// - Parameters:
    ///   - target: The target id
    ///   - data: The application defined data send to target user
    ///   - responseHandler: The callback to receive invite reponse
    ///
    /// - Throws: CarrierError
    public func sendInviteFriendRequest(to target: String,
                                        withData data: String,
                                        responseHandler: @escaping CarrierFriendInviteResponseHandler) throws {

        let cb: CFriendInviteResponseCallback = {

           (_, cfrom, cstatus, creason, cdata, clen, cctxt) in

                let ectxt = Unmanaged<AnyObject>.fromOpaque(cctxt!)
                    .takeRetainedValue() as! [AnyObject?]

                let carrier = ectxt[0] as! Carrier
                let handler = ectxt[1] as! CarrierFriendInviteResponseHandler

                let from = String(cString: cfrom!)
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

        let econtext: [AnyObject?] = [self, responseHandler as AnyObject]
        let unmanaged = Unmanaged.passRetained(econtext as AnyObject)
        let cctxt = unmanaged.toOpaque()

        Log.d(Carrier.TAG, "Begin to invite friend to \(target) with greet data" +
            " \(data)")

        let result = target.withCString { (cto) -> Int32 in
            return data.withCString { (cdata) -> Int32 in
                let len = data.utf8CString.count
                return ela_invite_friend(ccarrier, cto, cdata, len, cb, cctxt)
            }
        }

        guard result >= 0 else {
            unmanaged.release()
            let errno = getErrorCode()
            Log.e(Carrier.TAG, "Invite friend to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "Sended friend invite request to \(target).")
    }

    /// Reply the friend invite request.
    ///
    /// This function will send a invite response to friend.
    ///
    /// - Parameters:
    ///   - target: The id(userid@nodeid) who send invite request
    ///   - status: The status code of the response
    ///             0 is on success, otherse is error
    ///   - reason: The error message if status is error, or nil if success
    ///   - data: The application defined data send to target user.
    ///           If the status is error, this will be ignored.
    ///
    /// - Throws:   CarrierError
    public func replyFriendInviteRequest(to target: String,
                                         withStatus status: Int,
                                         reason: String?,
                                         data: String?) throws {

        if status == 0 && data == nil {
            throw CarrierError.InvalidArgument
        }

        if status != 0 && reason == nil {
            throw CarrierError.InvalidArgument
        }

        if status == 0 {
            Log.d(Carrier.TAG, "Begin to confirm friend invite request to" +
            "\(target) with data: \(data!)")
        } else {
            Log.d(Carrier.TAG, "Begin to refuse friend invite request to" +
            "\(target) with status: \(status) and reason: \(reason!)")
        }

        let result = target.withCString { (cto) -> Int32 in
            var creason: UnsafeMutablePointer<Int8>?
            var cdata: UnsafeMutablePointer<Int8>?
            var len: Int = 0

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

            return ela_reply_friend_invite(ccarrier, cto, CInt(status),
                                           creason, cdata, len)
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Reply friend invite to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "Sended reply to friend invite request to \(target)")
    }
}
