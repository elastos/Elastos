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

    handler.didBecomeReady(carrier)
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

private func onFriendMessage(_: OpaquePointer?, cfrom: UnsafePointer<Int8>?,
                             cmessage: UnsafePointer<Int8>?, _: Int,
                             cctxt: UnsafeMutableRawPointer?) {

    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!

    let from = String(cString: cfrom!)
    let msg  = String(cString: cmessage!)

    handler.didReceiveFriendMessage?(carrier, from, msg)
}

private func onFriendInvite(_: OpaquePointer?, cfrom: UnsafePointer<Int8>?,
                            cdata: UnsafePointer<Int8>?, _: Int,
                            cctxt: UnsafeMutableRawPointer?) {
    let carrier = getCarrier(cctxt!)
    let handler = carrier.delegate!

    let from = String(cString: cfrom!)
    let data = String(cString: cdata!)

    handler.didReceiveFriendInviteRequest?(carrier, from, data)
}

internal func getNativeHandlers() -> CCallbacks {

    var callbacks = CCallbacks()

    callbacks.idle = onIdle
    callbacks.connection_status = onConnection
    callbacks.ready = onReady
    callbacks.self_info = onSelfInfoChanged
    callbacks.friend_list = onFriendIterated
    callbacks.friend_info = onFriendInfoChanged
    callbacks.friend_presence = onFriendPresence
    callbacks.friend_request = onFriendRequest
    callbacks.friend_added = onFriendAdded
    callbacks.friend_removed = onFriendRemoved
    callbacks.friend_message = onFriendMessage
    callbacks.friend_invite = onFriendInvite

    return callbacks
}
