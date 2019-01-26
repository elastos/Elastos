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
    A class representing the Carrier user information.

    In Elastos carrier SDK, self and all friends are carrier user, and have
    same user attributes.
 */
@objc(ELACarrierUserInfo)
public class CarrierUserInfo: NSObject {

    /// Elastos carrier User ID max length.
    public static let MAX_ID_LEN: Int = 45

    /// Elastos carrier user name max length.
    public static let MAX_USER_NAME_LEN: Int = 63

    /// Elastos carrier user description max length.
    public static let MAX_USER_DESCRIPTION_LEN: Int = 127

    /// Elastos carrier user gender max length.
    public static let MAX_GENDER_LEN: Int = 31

    /// Elastos carrier user phone number max length.
    public static let MAX_PHONE_LEN: Int = 31

    /// Elastos carrier user email address max length.
    public static let MAX_EMAIL_LEN: Int = 127

    /// Elastos carrier user region max length.
    public static let MAX_REGION_LEN: Int = 127

    internal override init() {
        self.hasAvatar = false
        super.init()
    }

    /**
        User ID.
     */
    public internal(set) var userId: String?

    /**
        Nickname, also as display name.
     */
    public var name: String?

    /**
        User's brief description, also as what's up.
     */
    public var briefDescription: String?

    /**
        If user has avatar.
     */
    public var hasAvatar: Bool

    /**
        User's gender.
     */
    public var gender: String?

    /**
        User's phone number.
     */
    public var phone: String?

    /**
        User's email address.
     */
    public var email: String?

    /**
        User's region information.
     */
    public var region: String?

    internal static func format(_ info: CarrierUserInfo) -> String {
        return String(format: "userId[%@], name[%@], briefDescription[%@]," +
                      "hasAvatar[%@], gender[%@], phone[%@]" +
                      "email[%@], region[%@]",
                      String.toHardString(info.userId),
                      String.toHardString(info.name),
                      String.toHardString(info.briefDescription),
                      String.toHardString(info.hasAvatar.description),
                      String.toHardString(info.gender),
                      String.toHardString(info.phone),
                      String.toHardString(info.email),
                      String.toHardString(info.region))
    }

    public override var description: String {
        return CarrierUserInfo.format(self)
    }
}

internal func convertCarrierUserInfoToCUserInfo(_ info: CarrierUserInfo) -> CUserInfo {
    var cInfo = CUserInfo()

    info.userId?.writeToCCharPointer(&cInfo.userid)
    info.name?.writeToCCharPointer(&cInfo.name)
    info.briefDescription?.writeToCCharPointer(&cInfo.description)
    info.gender?.writeToCCharPointer(&cInfo.gender)
    info.phone?.writeToCCharPointer(&cInfo.phone)
    info.email?.writeToCCharPointer(&cInfo.email)
    info.region?.writeToCCharPointer(&cInfo.region)

    cInfo.has_avatar = Int32(info.hasAvatar ? 1: 0)

    return cInfo
}

internal func convertCUserInfoToCarrierUserInfo(_ cInfo: CUserInfo) -> CarrierUserInfo {
    let info = CarrierUserInfo()
    var temp = cInfo

    info.userId = String(cCharPointer: &temp.userid)
    info.name = String(cCharPointer: &temp.name)
    info.briefDescription = String(cCharPointer: &temp.description)
    info.gender = String(cCharPointer: &temp.gender)
    info.phone = String(cCharPointer: &temp.phone)
    info.email = String(cCharPointer: &temp.email)
    info.region = String(cCharPointer: &temp.region)

    info.hasAvatar = (cInfo.has_avatar != 0)

    return info
}
