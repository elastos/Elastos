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
func getCarrier(_ cctxt: UnsafeMutableRawPointer) -> Carrier {
    return Unmanaged<Carrier>.fromOpaque(cctxt).takeUnretainedValue()
}

private func onIdle(_: OpaquePointer?, cctxt: UnsafeMutableRawPointer?) {
    
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    handler.willBecomeIdle?(carrier)
}

private func onConnection(_: OpaquePointer?, cstatus: UInt32,
                          cctxt: UnsafeMutableRawPointer?) {
    
    let carrier = getCarrier(cctxt!)
    let status  = CarrierConnectionStatus(rawValue: Int(cstatus))!
    let handler = carrier.delegate!
    
    handler.connectionStatusDidChange?(carrier, status)
}

private func onReady(_: OpaquePointer?, cctxt: UnsafeMutableRawPointer?) {
    
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    handler.didBecomeReady?(carrier)
}

private func onSelfInfoChanged(_: OpaquePointer?,
                               cinfo: UnsafeRawPointer?,
                               cctxt: UnsafeMutableRawPointer?) {
    
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    let cUserInfo = cinfo!.assumingMemoryBound(to: CUserInfo.self).pointee
    let info = convertCUserInfoToCarrierUserInfo(cUserInfo)
    
    handler.selfUserInfoDidChange?(carrier, info)
}

private func onFriendIterated(_: OpaquePointer?,
                              cinfo: UnsafeRawPointer?,
                              cctxt: UnsafeMutableRawPointer?) -> CBool {
    
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    if (cinfo != nil) {
        let cFriendInfo = cinfo!.assumingMemoryBound(to: CFriendInfo.self).pointee
        let info = convertCFriendInfoToCarrierFriendInfo(cFriendInfo)
        carrier.friends.append(info)
    } else {
        handler.didReceiveFriendsList?(carrier, carrier.friends)
        carrier.friends.removeAll()
    }
    
    return true
}

private func onFriendConnectionChanged(_: OpaquePointer?,
                                       cfriendId: UnsafePointer<Int8>?,
                                       cstatus: UInt32,
                                       cctxt: UnsafeMutableRawPointer?) {
    
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    let friendId = String(cString: cfriendId!)
    let status = CarrierConnectionStatus(rawValue: Int(cstatus))!
    
    handler.friendConnectionDidChange?(carrier, friendId, status)
}

private func onFriendInfoChanged(_: OpaquePointer?,
                                 cfriendId: UnsafePointer<Int8>?,
                                 cinfo: UnsafeRawPointer?,
                                 cctxt: UnsafeMutableRawPointer?) {
    
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    let friendId = String(cString: cfriendId!)
    let cFriendInfo = cinfo!.assumingMemoryBound(to: CFriendInfo.self).pointee
    let info = convertCFriendInfoToCarrierFriendInfo(cFriendInfo)
    
    handler.friendInfoDidChange?(carrier,friendId, info)
}

private func onFriendPresence(_: OpaquePointer?,
                              cfriendId: UnsafePointer<Int8>?,
                              cpresence: UInt32,
                              cctxt: UnsafeMutableRawPointer?) {
    
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    let friendId = String(cString: cfriendId!)
    let presence = CarrierPresenceStatus(rawValue: Int(cpresence))!
    
    handler.friendPresenceDidChange?(carrier, friendId, presence)
}

private func onFriendRequest(_: OpaquePointer?,
                             cuserId: UnsafePointer<Int8>?,
                             cinfo: UnsafeRawPointer?,
                             chello: UnsafePointer<Int8>?,
                             cctxt: UnsafeMutableRawPointer?) {
    
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    let userId = String(cString: cuserId!)
    let cUserInfo = cinfo!.assumingMemoryBound(to: CUserInfo.self).pointee
    let info   = convertCUserInfoToCarrierUserInfo(cUserInfo)
    let hello  = String(cString: chello!)
    
    handler.didReceiveFriendRequest?(carrier, userId, info, hello)
}

private func onFriendAdded(_: OpaquePointer?,
                           cinfo: UnsafeRawPointer?,
                           cctxt: UnsafeMutableRawPointer?) {
    
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    let cFriendInfo = cinfo!.assumingMemoryBound(to: CFriendInfo.self).pointee
    let info = convertCFriendInfoToCarrierFriendInfo(cFriendInfo)
    
    handler.newFriendAdded?(carrier, info)
}

private func onFriendRemoved(_: OpaquePointer?,
                             cfriendId: UnsafePointer<Int8>?,
                             cctxt: UnsafeMutableRawPointer?) {
    
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    let friendId = String(cString: cfriendId!)
    handler.friendRemoved?(carrier, friendId)
}

private func onFriendMessage(_: OpaquePointer?,
                             cfrom: UnsafePointer<Int8>?,
                             cmessage: UnsafePointer<Int8>?,
                             len : Int,
                             timestamp: Int64,
                             is_offline: Bool,
                             cctxt: UnsafeMutableRawPointer?)
{
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    let from = String(cString: cfrom!)
    let msg  = Data(bytes: cmessage!, count: len)
    let interval:TimeInterval = TimeInterval.init(timestamp)
    let date = Date(timeIntervalSince1970: interval)

    handler.didReceiveFriendMessage?(carrier, from, msg, date, is_offline)
}

private func onFriendInvite(_: OpaquePointer?, cfrom: UnsafePointer<Int8>?,
                            _: UnsafePointer<Int8>?,
                            cdata: UnsafePointer<Int8>?, _: Int,
                            cctxt: UnsafeMutableRawPointer?) {
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    let from = String(cString: cfrom!)
    let data = String(cString: cdata!)
    
    handler.didReceiveFriendInviteRequest?(carrier, from, data)
}

private func onGroupInvite(_: OpaquePointer?, cfrom: UnsafePointer<Int8>?,
                           _ ccookie: UnsafePointer<Int8>?, _ length: Int,
                           cctxt: UnsafeMutableRawPointer?) {
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!
    
    let from = String(cString: cfrom!)
    let cookie = Data(bytes: ccookie!, count: length)
    
    handler.didReceiveGroupInvite?(carrier, from, cookie)
}

private func onGroupConnected(_: OpaquePointer?, cgroupid: UnsafePointer<Int8>?,
                              cctxt: UnsafeMutableRawPointer?) {
    let carrier = getCarrier(cctxt!)
    let groupid = String(cString: cgroupid!)
    let group   = carrier.groups[groupid]
    
    if (group != nil)  {
        let handler = group!.delegate!
        
        handler.groupDidConnect?(group!)
    }
}

private func onGroupMessage(_: OpaquePointer?, cgroupid: UnsafePointer<Int8>?,
                            cfrom: UnsafePointer<Int8>?,
                            cmsg: UnsafeRawPointer?, length: Int,
                            cctxt: UnsafeMutableRawPointer?) {
    let carrier = getCarrier(cctxt!)
    let groupid = String(cString: cgroupid!)
    let group   = carrier.groups[groupid]
    
    if (group != nil) {
        let handler = group!.delegate!
        let from    = String(cString: cfrom!)
        let message = Data(bytes: cmsg!, count: length)
        
        handler.didReceiveGroupMessage?(group!, from, message)
    }
}

private func onGroupTitle(_: OpaquePointer?, cgroupid: UnsafePointer<Int8>?,
                          _ cfrom: UnsafePointer<Int8>?,
                          _ ctitle: UnsafePointer<Int8>?,
                          cctxt: UnsafeMutableRawPointer?)
{
    let carrier = getCarrier(cctxt!)
    let groupid = String(cString: cgroupid!)
    let group   = carrier.groups[groupid]
    
    if (group != nil) {
        let handler = group!.delegate!
        
        let from  = String(cString: cfrom!)
        let title = String(cString: ctitle!)
        
        handler.groupTitleDidChange?(group!, from, title)
    }
}

private func onPeerName(_: OpaquePointer?, cgroupid: UnsafePointer<Int8>?,
                        _ cfrom: UnsafePointer<Int8>?,
                        _ cname: UnsafePointer<Int8>?,
                        cctxt: UnsafeMutableRawPointer?)
{
    let carrier = getCarrier(cctxt!)
    let groupid = String(cString: cgroupid!)
    let group   = carrier.groups[groupid]
    
    if (group != nil)  {
        let handler = group!.delegate!
        
        let from = String(cString: cfrom!)
        let name = String(cString: cname!)
        
        handler.groupPeerNameDidChange?(group!, from, name)
    }
}

private func onPeerListChanged(_: OpaquePointer?, cgroupid: UnsafePointer<Int8>?,
                               cctxt: UnsafeMutableRawPointer?)
{
    let carrier = getCarrier(cctxt!)
    let groupid = String(cString: cgroupid!)
    let group   = carrier.groups[groupid]
    
    if (group != nil)  {
        let handler = group!.delegate!
        
        handler.groupPeerListDidChange?(group!)
    }
}

internal func getNativeHandlers() -> CCallbacks {
    
    var callbacks = CCallbacks()
    
    callbacks.idle = onIdle
    callbacks.connection_status = onConnection
    callbacks.ready = onReady
    callbacks.self_info = onSelfInfoChanged
    callbacks.friend_list = onFriendIterated
    callbacks.friend_info = onFriendInfoChanged
    callbacks.friend_connection = onFriendConnectionChanged
    callbacks.friend_presence = onFriendPresence
    callbacks.friend_request = onFriendRequest
    callbacks.friend_added = onFriendAdded
    callbacks.friend_removed = onFriendRemoved
    callbacks.friend_message = onFriendMessage
    callbacks.friend_invite = onFriendInvite
    callbacks.group_invite = onGroupInvite
    callbacks.group_callbacks = CGroupCallbacks()
    callbacks.group_callbacks.group_connected = onGroupConnected
    callbacks.group_callbacks.group_message = onGroupMessage
    callbacks.group_callbacks.group_title = onGroupTitle
    callbacks.group_callbacks.peer_name = onPeerName
    callbacks.group_callbacks.peer_list_changed = onPeerListChanged
    
    return callbacks
}

