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
        _ data: String?) -> Void

    public typealias CarrierFriendMessageReceiptResponseHandler =
        (_ msgid: Int64, _ status: CarrierReceiptState) -> Void

    /// Carrier node App message max length.
    @objc public static let MAX_APP_MESSAGE_LEN: Int = 2048
    
    private static let TAG: String = "Carrier"
    private static let MAX_ADDRESS_LEN: Int = 52;
    private static let MAX_ID_LEN: Int = 45

    internal var ccarrier: OpaquePointer?
    private  var didKill : Bool
    private  let semaph  : DispatchSemaphore
    
    internal weak var delegate: CarrierDelegate?
    
    internal var friends: [CarrierFriendInfo]
    
    internal var groups: [String: CarrierGroup]
    
    /// Get current carrier node version.
    ///
    /// - Returns: The current carrier node version.
    @objc(getVersion)
    public static func getVersion() -> String {
        return String(cString: ela_get_version())
    }
    
    /// Check if the carrier address ID is valid.
    ///
    /// - Parameter address: The carrier address to be check
    ///
    /// - Returns: True if carrier address is valid, otherwise false
    @objc(isValidAddress:)
    public static func isValidAddress(_ address: String) -> Bool {
        return (Base58.decode(address)?.count == 38)
    }
    
    /// Check if the carrier public ID is valid.
    ///
    /// - Parameter id: The carrier id to be check
    ///
    /// - Returns: True if carrier id is valid, otherwise false
    @objc(isValidUserId:)
    public static func isValidUserId(_ id: String) -> Bool {
        return (Base58.decode(id)?.count == 32)
    }
    
    /// Extract ID from the carrier node address.
    ///
    /// - Parameter address: The carrier node address.
    ///
    /// - Returns: Valid Id if carrier node address is valid, otherwise nil
    @objc(getUserIdFromAddress:)
    public static func getUserIdFromAddress(_ address: String) -> String? {
        if let addr = Base58.decode(address), addr.count == 38 {
            return Base58.encode(Array(addr.prefix(32)))
        } else {
            return nil
        }
    }
    
    /// Create node  instance. After initialize the instance,
    /// it's ready to start and therefore connect to the carrier network.
    ///
    /// - Parameters:
    ///   - options: The options to set for carrier node
    ///   - delegate: The delegate for carrier node to comply with
    ///
    /// - Throws: CarrierError
    @objc(createInstance:delegate:error:)
    public static func createInstance(options: CarrierOptions, delegate: CarrierDelegate) throws -> Carrier {
        var copts = convertCarrierOptionsToCOptions(options)
        defer {
            cleanupCOptions(copts)
        }

        let carrier = Carrier(delegate)
        var chandler = getNativeHandlers()
        let cctx = Unmanaged.passUnretained(carrier).toOpaque()

        let ccarrier = ela_new(&copts, &chandler, cctx)
        guard let _ = ccarrier else {
            let errno = getErrorCode()
            Log.e(TAG, "Create native carrier instance error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }
        carrier.ccarrier = ccarrier

        let cb: CGroupsIterateCallback = { (cgroupId, ctxt) in
            if cgroupId != nil {
                let carrier = ctxt!.assumingMemoryBound(to: Carrier.self).pointee
                let groupId = String(cString: cgroupId!)
                let group = CarrierGroup(carrier.ccarrier, groupId, carrier.delegate!)

                carrier.groups[groupId] = group
            }
            return true
        }

        var _carrier = carrier
        let result = withUnsafeMutablePointer(to: &_carrier) { (ptr) -> Int32 in
            let cctxt = UnsafeMutableRawPointer(ptr)
            return ela_get_groups(ccarrier, cb, cctxt)
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Get current user's friends error: 0x%X", errno)
            ela_kill(ccarrier)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        carrier.didKill = false

        Log.i(TAG, "Native carrier node instance created.")
        return carrier
    }
    
    private init(_ delegate: CarrierDelegate) {
        self.delegate = delegate
        self.didKill = true
        self.semaph = DispatchSemaphore(value: 0)
        self.friends = [CarrierFriendInfo]()
        self.groups = [String: CarrierGroup]()
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
    @objc(start:error:)
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
    @objc(kill)
    public func kill() {
        objc_sync_enter(self)
        
        if (!didKill) {
            Log.d(Carrier.TAG, "Actively to kill native carrier node ...");
            
            ela_kill(ccarrier)
            ccarrier = nil
            semaph.wait()
            delegate = nil
            didKill = true
            Log.i(Carrier.TAG, "Native carrier node killed and exited")
        }
        
        objc_sync_exit(self)
    }
    
    /// Get node address associated with carrier node instance.
    ///
    /// Returns: The node address
    @objc(getAddress)
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
    @objc(getNodeId)
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
    @objc(getUserId)
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
    @objc(setSelfNospam:error:)
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
    
    @objc(getSelfNospam:)
    public func getSelfNospam(error: NSErrorPointer) -> UInt32 {
        var nospam: UInt32 = 0
        do {
            nospam = try getSelfNospam()
        } catch let aError as NSError {
            error?.pointee = aError
        }
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
    @objc(setSelfUserInfo:error:)
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
    @objc(setSelfPresence:error:)
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
    
    @objc(getSelfPresence:)
    public func getSelfPresence(error: NSErrorPointer) -> CarrierPresenceStatus {
        var presence : CarrierPresenceStatus = .None
        do {
            presence = try getSelfPresence()
        } catch let aError as NSError {
            error?.pointee = aError
        }
        return presence
    }
    
    /// Check if carrier node instance is being ready.
    ///
    /// - Returns: true if the carrier node instance is ready, or false if not
    @objc(isReady)
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
    @objc(addFriendWith:withGreeting:error:)
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
    @objc(acceptFriend:error:)
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
    @objc(removeFriend:error:)
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

    /// Send a message to target friend without intent to know whether the message
    /// was sent as online message or offline message.
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
    @objc(sendFriendMessage:withMessage:error:)
    public func sendFriendMessage(to target: String, _ withMessage: String) throws {

        let msgData = withMessage.data(using: .utf8)
        try sendFriendMessage(to: target, msgData!)
    }

    /// Send a message to target friend without intent to know whether the message
    /// was sent as online message or offline message.
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
    @objc(sendFriendMessage:withData:error:)
    public func sendFriendMessage(to target: String, _ withData: Data) throws {
        var isOffline: CBool = false
        let result = target.withCString { cto in
            return withData.withUnsafeBytes{ cdata -> Int32 in
                return ela_send_friend_message(ccarrier, cto, cdata, withData.count, &isOffline)
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Send message to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno) as NSError
        }

        Log.d(Carrier.TAG, "Sended message: \(withData) to \(target).")
    }

    /// Send a message to target friend with intent to konw whether the message
    /// was sent as online message or offline message.
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
    /// - returns: The value of true means the message was sent as online message.
    ///            Otherwise as offline message.
    public func sendFriendMessage(to target: String, withMessage msg: String) throws -> Bool {

        let msgData = msg.data(using: .utf8)
        return try sendFriendMessage(to: target, withData: msgData!)
    }

    /// Send a message to target friend with intent to konw whether the message
    /// was sent as online message or offline message.
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
    /// - returns: The value of true means the message was sent as online message.
    ///            Otherwise as offline message.
    public func sendFriendMessage(to target: String, withData data: Data) throws -> Bool {
        var isOffline: CBool = false
        let result = target.withCString { cto in
            return data.withUnsafeBytes{ cdata -> Int32 in
                return ela_send_friend_message(ccarrier, cto, cdata, data.count, &isOffline)
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Send message to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno) as NSError
        }

        Log.d(Carrier.TAG, "Sended message: \(data) to \(target).")
        return Bool(!isOffline)
    }

    /// Send a message to target friend with intent to konw whether the message
    /// was sent as online message or offline message.
    ///
    /// The message length may not exceed `MAX_APP_MESSAGE_LEN`, and message
    /// itself should be text-formatted. Larger messages must be splitted by
    /// application and sent as separate messages. Other nodes can reassemble 
    /// the fragments.
    ///
    /// - Parameters:
    ///   - target: The target id
    ///   - msg: The message content defined by application in string type.
    ///   - error: [out] CarrierError
    /// - returns: The value of true means the message was sent as online message.
    ///            Otherwise as offline message.
    @objc(sendFriendMessage:message:error:)
    public func sendFriendMessage(to target: String, withMessage msg: String, error: NSErrorPointer) -> Bool {
        
        let msgData = msg.data(using: .utf8)
      return sendFriendMessage(to: target, withData: msgData!, error: error)
    }
    
    /// Send a message to target friend with intent to konw whether the message
    /// was sent as online message or offline message.
    ///
    /// The message length may not exceed `MAX_APP_MESSAGE_LEN`, and message
    /// itself should be text-formatted. Larger messages must be splitted by
    /// application and sent as separate messages. Other nodes can reassemble
    /// the fragments.
    ///
    /// - Parameters:
    ///   - target: The target id
    ///   - msg: The message content defined by application in string type.
    ///   - error: [out] CarrierError
    /// - returns: The value of true means the message was sent as online message.
    ///            Otherwise as offline message.
    @objc(sendFriendMessage:data:error:)
    public func sendFriendMessage(to target: String, withData data: Data, error: NSErrorPointer) -> Bool {
        var isOffline: CBool = false
        let result = target.withCString { cto in
            return data.withUnsafeBytes { cdata -> Int32 in
                return ela_send_friend_message(ccarrier, cto, cdata, data.count, &isOffline)
            }
        }
        
        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Send message to \(target) error: 0x%X", errno)
            error?.pointee = CarrierError.FromErrorCode(errno: errno) as NSError
            return Bool(false)
        }
        
        Log.d(Carrier.TAG, "Sended message: \(data) to \(target).")
        return Bool(!isOffline)
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
    @objc(sendInviteFriendRequest:data:responseHandler:error:)
    public func sendInviteFriendRequest(to target: String,
                                        withData data: String,
                                        responseHandler: @escaping CarrierFriendInviteResponseHandler) throws {
        
        let cb: CFriendInviteResponseCallback = {
            
            (_, cfrom, _, cstatus, creason, cdata, clen, cctxt) in
            
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
                return ela_invite_friend(ccarrier, cto, nil, cdata, len, cb, cctxt)
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
    
    /// Send a message to a friend with receipt.
    ///
    /// The message length may not exceed ELA_MAX_BULK_MESSAGE_LEN. Larger messages
    /// must be split by application and sent as separate fragments. Other carrier
    /// nodes can reassemble the fragments.
    ///
    /// - Parameters:
    ///   - target: The target id
    ///   - msg: The message content defined by application.
    ///   - responseHandler: The user context in callback.
    /// - returns: Return the message identifier which would be used for handler to  invoke receipt notification.
    ///
    /// - Throws: CarrierError
    public func sendMessageWithReceipt(to target: String,
                                        withMessage msg: String,
                                        responseHandler: @escaping CarrierFriendMessageReceiptResponseHandler) throws -> Int64 {
        let cb: CFriendMessageReceiptCallback = { (cmsgid, cstatus, cctxt) in

            let ectxt = Unmanaged<AnyObject>.fromOpaque(cctxt!)
                .takeRetainedValue() as! [AnyObject?]

            let handler = ectxt[1] as! CarrierFriendMessageReceiptResponseHandler

            let msgid = Int64(cmsgid)

            var status = Int(cstatus)
            if status != 0 {
                status = getErrorCode()
            }
            let receipt = CarrierReceiptState(rawValue: status)!
            handler(msgid, receipt)
        }

        let econtext: [AnyObject?] = [self, responseHandler as AnyObject]
        let unmanaged = Unmanaged.passRetained(econtext as AnyObject)
        let cctxt = unmanaged.toOpaque()

        Log.d(Carrier.TAG, "\(target) send message with receipt with")

        let result = target.withCString { (cto) -> Int32 in
            return msg.withCString { (cdata) -> Int32 in
                let len = msg.utf8CString.count
                return ela_send_message_with_receipt(ccarrier, cto, cdata, len, cb, cctxt)
            }
        }

        guard result >= 0 else {
            unmanaged.release()
            let errno = getErrorCode()
            Log.e(Carrier.TAG, "send message with receipt to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "send message with receipt to \(target).")
        return Int64(result)
    }

    /// Send a message to a friend with receipt.
    ///
    /// The message length may not exceed ELA_MAX_BULK_MESSAGE_LEN. Larger messages
    /// must be split by application and sent as separate fragments. Other carrier
    /// nodes can reassemble the fragments.
    ///
    /// - Parameters:
    ///   - target: The target id
    ///   - msg: The message content defined by application.
    ///   - responseHandler: The user context in callback.
    /// - returns: Return the message identifier which would be used for handler to  invoke receipt notification.
    ///
    /// - Throws: CarrierError
    public func sendMessageWithReceipt(to target: String,
                                        withMessage msg: Data,
                                        responseHandler: @escaping CarrierFriendMessageReceiptResponseHandler) throws -> Int64 {
        let cb: CFriendMessageReceiptCallback = { (cmsgid, cstatus, cctxt) in

            let ectxt = Unmanaged<AnyObject>.fromOpaque(cctxt!)
                .takeRetainedValue() as! [AnyObject?]

            let handler = ectxt[1] as! CarrierFriendMessageReceiptResponseHandler

            let msgid = Int64(cmsgid)

            var status = Int(cstatus)
            if status != 0 {
                status = getErrorCode()
            }
            let receipt = CarrierReceiptState(rawValue: status)!
            handler(msgid, receipt)
        }

        let econtext: [AnyObject?] = [self, responseHandler as AnyObject]
        let unmanaged = Unmanaged.passRetained(econtext as AnyObject)
        let cctxt = unmanaged.toOpaque()

        Log.d(Carrier.TAG, "\(target) send message with receipt with")

        let result = target.withCString { (cto) -> Int32 in
            return msg.withUnsafeBytes { (cdata) -> Int32 in
                return ela_send_message_with_receipt(ccarrier, cto, cdata, msg.count, cb, cctxt)
            }
        }

        guard result >= 0 else {
            unmanaged.release()
            let errno = getErrorCode()
            Log.e(Carrier.TAG, "send message with receipt to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(Carrier.TAG, "send message with receipt to \(target).")
        return Int64(result)
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
    @objc(replyFriendInviteRequest:status:resson:data:error:)
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
            
            return ela_reply_friend_invite(ccarrier, cto, nil, CInt(status),
                                           creason, cdata, len)
        }
        
        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Reply friend invite to \(target) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }
        
        Log.d(Carrier.TAG, "Sended reply to friend invite request to \(target)")
    }
    
    /// New a group with specified delegate.
    ///
    /// - Parameters:
    ///     - delegate: The delegate attached to the new group.
    ///
    /// - Returns:
    ///     - The new group.
    ///
    /// - Throws:
    ///     CarrierError
    ///
    @objc(createGroup:)
    public func createGroup() throws -> CarrierGroup {
        let len  = Carrier.MAX_ID_LEN + 1
        var data = Data(count: len);
        
        let result = data.withUnsafeMutableBytes() {
            (ptr: UnsafeMutablePointer<Int8>) -> Int32 in
            return ela_new_group(ccarrier, ptr, len)
        }
        
        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "New group error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }
        
        let groupid = data.withUnsafeBytes() {
            (ptr: UnsafePointer<Int8>) -> String in
            return String(cString: ptr)
        }
        
        let group = CarrierGroup(ccarrier, groupid, self.delegate!)
        groups[groupid] = group
        return group
    }
    
    /// Join a group with specific cookie information.
    ///
    /// This function should be called only if application received a group
    /// invitation from remote friend.
    ///
    /// - Parameters:
    ///     - friendId: The friend who send a group invitation
    ///     - cookie:   The cookie information to join group
    ///     - delegate: The delegate to the new group
    ///
    /// - Returns:
    ///     The new group instance.
    ///
    /// - Throws:
    ///     CarrierError
    ///
    @objc(joinGroupCreatedBy:withCookie:error:)
    public func joinGroup(createdBy friendId: String, withCookie cookie: Data) throws -> CarrierGroup {
        
        let len  = Carrier.MAX_ID_LEN + 1
        var data = Data(count: len);
        
        let result = friendId.withCString { (cfriendid) -> Int32 in
            return cookie.withUnsafeBytes { (ccookie: UnsafePointer<Int8>) -> Int32 in
                return data.withUnsafeMutableBytes()  {
                    (cdata: UnsafeMutablePointer<Int8>) -> Int32 in
                    return ela_group_join(ccarrier, cfriendid, ccookie,
                                          cookie.count, cdata, len)
                }
            }
        }
        
        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "New group error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }
        
        let groupid = data.withUnsafeBytes() {
            (ptr: UnsafePointer<Int8>) -> String in
            return String(cString: ptr)
        }
        
        let group = CarrierGroup(ccarrier, groupid, self.delegate!)
        groups[groupid] = group
        return group
    }
    
    /// Leave from a specified group.
    ///
    /// - Parameters:
    ///     - group: The specified group to leave from.
    ///
    /// - Throws:
    ///     CarrierError
    ///
    @objc(leaveGroup:error:)
    public func leaveGroup(from group: CarrierGroup) throws {
        let groupid = group.getId()
        
        let result = groupid.withCString() { (ptr) in
            return ela_leave_group(ccarrier, ptr)
        }
        
        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(Carrier.TAG, "Leave group \(groupid) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }
        
        groups.removeValue(forKey: groupid)
        group.leave()
    }
    
    /// Get groups in the Carrier instance.
    ///
    /// - Returns:
    ///     The array of CarrierGroup instances.
    ///
    /// - Throws:
    ///     CarrierError
    ///
    @objc(getGroups:)
    public func getGroups() throws -> [CarrierGroup] {
        var tempGroups = [CarrierGroup]()
        tempGroups.append(contentsOf: self.groups.values)
        return tempGroups
    }
}
