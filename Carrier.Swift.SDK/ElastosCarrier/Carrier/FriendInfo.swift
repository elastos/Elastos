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

    private var _label    : String?
    private var _presence : CarrierPresenceStatus
    private var _status   : CarrierConnectionStatus

    internal override init() {
        _presence = CarrierPresenceStatus.None
        _status   = CarrierConnectionStatus.Disconnected
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
    public var status: CarrierConnectionStatus {
        set {
            _status = newValue
        }
        get {
            return _status
        }
    }

    /**
        Label name for the friend.
     */
    public var label: String? {
        set {
            _label = newValue
        }
        get {
            return _label
        }
    }

    /**
        Friend's presence status.
     */
    public var presence: CarrierPresenceStatus {
        set {
            _presence = newValue
        }
        get {
            return _presence
        }
    } 

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
