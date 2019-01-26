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

/// The class representing carrier group.
@objc(ELACarrierGroup)
public class CarrierGroup: NSObject {
    /// Carrier group message max length.
    public static let MAX_APP_MESSAGE_LEN: Int = 2048

    private static let TAG: String = "CarrierGroup"
    private static let MAX_GROUP_TITLE_LEN: Int = 127
    private static let MAX_ADDRESS_LEN: Int = 52
    private static let MAX_ID_LEN: Int = 45

    internal var ccarrier: OpaquePointer?
    internal var groupid : String
    private  var didLeave : Bool
    weak var delegate: CarrierGroupDelegate?

    init(_ ccarrier: OpaquePointer!, _ groupid: String,
                 _ delegate: CarrierGroupDelegate) {

        self.ccarrier = ccarrier
        self.groupid  = groupid
        self.delegate = delegate
        self.didLeave = false

        super.init()
    }

    deinit {
        leave()
    }

    func leave() {
        objc_sync_enter(self)

        if (!didLeave) {
            Log.d(CarrierGroup.TAG, "Destroying group \(groupid) ...");

            self.ccarrier = nil
            self.delegate = nil
            self.didLeave = true
            Log.i(CarrierGroup.TAG, "Group \(groupid) destroyed")
        }

        objc_sync_exit(self)
    }

    /// Get groupid in string way.
    ///
    /// - Returns:
    ///     The groupid in string.
    ///
    public func getId() -> String {
        return self.groupid
    }

    /// Invite a specified friend into group.
    ///
    /// - Parameters:
    ///     - friendId: The friend wll be invited into group.
    ///
    /// - Throws:
    ///     CarrierError.
    ///
    @objc(inviteFriend:error:)
    public func inviteFriend(_ friendId: String) throws {
        let result = groupid.withCString{ (cgroupid) -> Int32 in
            return friendId.withCString { (cfriendid) -> Int32 in
                return ela_group_invite(ccarrier, cgroupid, cfriendid)
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(CarrierGroup.TAG, "Invite friend \(friendId) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(CarrierGroup.TAG, "Invite friend \(friendId) to group \(groupid) success.")
    }

    /// Send a message to a group.
    ///
    /// The message length may not exceed MAX_APP_MESSAGE_LEN. Larger messages
    /// must be split by application and sent as separate fragments. Other carrier
    /// nodes can reassemble the fragments.
    ///
    /// - Parameters:
    ///   - data: The data content to send.
    ///
    /// - Throws:
    ///     CarrierError
    ///
    @objc(sendMessage:error:)
    public func sendMessage(_ data: Data) throws {
        let result = groupid.withCString{ (cgroupid) -> Int32 in
            return data.withUnsafeBytes{ (cdata) -> Int32 in
                return ela_group_send_message(ccarrier, cgroupid, cdata, data.count)
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(CarrierGroup.TAG, "Send group message to \(groupid) error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(CarrierGroup.TAG, "Sended group \(groupid) message: \(data).")
    }

    /// Get group title.
    ///
    /// - Returns:
    ///     The new group title.
    ///
    /// - Throws:
    ///     CarrierError
    ///
    @objc(getTitle:)
    public func getTitle() throws -> String {
        let len = CarrierGroup.MAX_GROUP_TITLE_LEN + 1
        var data = Data(count: len)

        let title = groupid.withCString{ (cgroupid) -> String? in
            return data.withUnsafeMutableBytes() {
                (ptr: UnsafeMutablePointer<Int8>) -> String? in
                let result = ela_group_get_title(ccarrier, cgroupid, ptr, len)
                return result >= 0 ? String(cString: ptr) : nil
            }
        }

        guard title != nil else {
            let errno: Int = getErrorCode();
            Log.e(CarrierGroup.TAG, "Get group \(groupid) title error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(CarrierGroup.TAG, "Current group \(groupid) title: \(title!)")
        return title!
    }

    /// Set new group title.
    ///
    /// - Parameters:
    ///     - newTitle: new group title
    ///
    /// - Throws:
    ///     CarrierError
    ///
    @objc(setTitle:error:)
    public func setTitle(_ newTitle: String) throws {
        let result = groupid.withCString{ (cgroupid) -> Int32 in
            return newTitle.withCString{ (ctitle) -> Int32 in
                return ela_group_set_title(ccarrier, cgroupid, ctitle)
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(CarrierGroup.TAG, "Set group \(groupid) title error: 0x%x", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }
    }

    /// Get group peer list
    ///
    /// - Returns:
    ///     The array of CarrierGroupPeer instance.
    ///
    /// - Throws:
    ///     CarrierError
    ///
    @objc(getPeers:)
    public func getPeers() throws -> [CarrierGroupPeer] {
        let cb: CGroupPeersIterateCallback = { (cpeer, ctxt) in
            if cpeer != nil {
                let cGroupPeer = cpeer!.assumingMemoryBound(to: CGroupPeer.self).pointee
                let peer = convertCGroupPeerToCarrierGroupPeer(cGroupPeer)
                ctxt!.assumingMemoryBound(to: [CarrierGroupPeer].self)
                    .pointee.append(peer)
            }
            return true
        }

        var peers = [CarrierGroupPeer]()
        let result = withUnsafeMutablePointer(to: &peers) { (ptr) -> Int32 in
            return groupid.withCString{ (cgroupid) -> Int32 in
                let cctxt = UnsafeMutablePointer(ptr)
                return ela_group_get_peers(ccarrier, cgroupid, cb, cctxt)
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(CarrierGroup.TAG, "Get group \(groupid) peers error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        Log.d(CarrierGroup.TAG, "Current group peers listed below: +++>>")
        for peer in peers {
            Log.d(CarrierGroup.TAG, "\(peer)");
        }
        Log.d(CarrierGroup.TAG, "<<+++")

        return peers
    }

    /// Get group peer
    ///
    /// - Returns:
    ///     The CarrierGroupPeer instance.
    ///
    /// - Throws:
    ///     CarrierError
    ///
    @objc(getPeerByPeerId:error:)
    public func getPeer(byPeerid peerId: String) throws -> CarrierGroupPeer {
        var cpeer = CGroupPeer()
        
        let result = peerId.withCString { (cpeerid) -> Int32 in
            return groupid.withCString{ (cgroupid) -> Int32 in
                return ela_group_get_peer(ccarrier, cgroupid, cpeerid, &cpeer)
            }
        }

        guard result >= 0 else {
            let errno: Int = getErrorCode()
            Log.e(CarrierGroup.TAG, "Get peer \(peerId) info error: 0x%X", errno)
            throw CarrierError.FromErrorCode(errno: errno)
        }

        let peer = convertCGroupPeerToCarrierGroupPeer(cpeer)
        Log.d(CarrierGroup.TAG, "Current peer infos: \(peer)")
        return peer
    }
}
