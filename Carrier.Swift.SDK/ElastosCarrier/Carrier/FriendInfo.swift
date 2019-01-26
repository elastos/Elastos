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
 * A class representing the carrier friend information.
 *
 * Include the basic user information and the extra friend information.
 */
@objc(ELACarrierFriendInfo)
public class CarrierFriendInfo: CarrierUserInfo {

    /// carrier friend label max length.
    public static let MAX_LABEL_LEN = 63

    /// carrier node presence max length.
    public static let MAX_USER_PRESENCE_LEN = 31

    internal override init() {
        presence = CarrierPresenceStatus.None
        status   = CarrierConnectionStatus.Disconnected
        super.init()
    }

    /**
        Friend's user information.
     */
    internal var userInfo: CarrierUserInfo? {
        set {
            self.userId = newValue?.userId
            self.name = newValue?.name
            self.briefDescription = newValue?.briefDescription
            self.hasAvatar = newValue?.hasAvatar ?? false
            self.gender = newValue?.gender
            self.phone = newValue?.phone
            self.email = newValue?.email
            self.region = newValue?.region
        }
        get {
            return self as CarrierUserInfo
        }
    }

    /**
        Friend's presence status.
     */
    public var status: CarrierConnectionStatus

    /**
        Label name for the friend.
     */
    public var label: String?

    /**
        Friend's presence status.
     */
    public var presence: CarrierPresenceStatus

    internal static func format2(_ info: CarrierFriendInfo) -> String {
        return String(format: "FriendInfo: userInfo[%@], status[%@]," +
                      " label[%@], presence[%@]",
                      CarrierUserInfo.format(info),
                      String.toHardString(info.status.description),
                      String.toHardString(info.label),
                      String.toHardString(info.presence.description))
    }

    public override var description: String {
        return CarrierFriendInfo.format2(self)
    }
}

internal func convertCarrierFriendInfoToCFriendInfo(_ info: CarrierFriendInfo) -> CFriendInfo {
    var cInfo = CFriendInfo()

    cInfo.user_info = convertCarrierUserInfoToCUserInfo(info)
    cInfo.status = Int32(info.status.rawValue)
    cInfo.presence = Int32(info.presence.rawValue)

    info.label?.writeToCCharPointer(&cInfo.label)

    return cInfo
}

internal func convertCFriendInfoToCarrierFriendInfo(_ cInfo: CFriendInfo) -> CarrierFriendInfo {
    let info = CarrierFriendInfo()

    info.userInfo = convertCUserInfoToCarrierUserInfo(cInfo.user_info)
    info.status = convertCConnectionStatusToCarrierConnectionStatus(cInfo.status)
    info.presence = convertCPresenceStatusToCarrierPresenceStatus(cInfo.presence)
    info.presence = convertCPresenceStatusToCarrierPresenceStatus(cInfo.presence)

    var label = cInfo.label
    info.label = String(cCharPointer: &label)

    return info
}
