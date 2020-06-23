import Foundation

public class DIDMeta: Metadata {
    private var _deactivated: Bool = false
    private var _transactionId: String?
    private var _aliasName: String?
    private var _prevSignature: String?
    private var _signature: String?
    private var _published: Int?
    private let TXID = RESERVED_PREFIX + "txid"
    private let PREV_SIGNATURE = RESERVED_PREFIX + "prevSignature"
    private let SIGNATURE = RESERVED_PREFIX + "signature"
    private let PUBLISHED = RESERVED_PREFIX + "published"
    private let ALIAS = RESERVED_PREFIX + "alias"
    private let DEACTIVATED = RESERVED_PREFIX + "deactivated"

    public required init() {
        super.init()
    }

    public var aliasName: String? {
        return self.get(key: ALIAS) as? String
    }

    public func setAlias(_ alias: String?) {
        put(key: ALIAS, value: alias as Any)
    }

    public var transactionId: String? {
        return self.get(key: TXID) as? String
    }

    public func setTransactionId(_ newValue: String?) {
        put(key: TXID, value: newValue as Any)
    }
    public var previousSignature: String? {
       return self.get(key: PREV_SIGNATURE) as? String
    }

    public func setPreviousSignature(_ newValue: String?) {
         put(key: PREV_SIGNATURE, value: newValue as Any)
    }

    public var signature: String? {
        return self.get(key: SIGNATURE) as? String
    }

    public func setSignature(_ newValue: String?) {
        put(key: SIGNATURE, value: newValue as Any)
    }

    public func getPublished() -> Date? {
        let time = self.get(key: PUBLISHED) as? Int
        return DateHelper.getDateFromTimeStamp(time)
    }

    public func setPublished(_ timestamp: Date) {
        let timestampDate = DateHelper.getTimeStamp(timestamp)
        put(key: PUBLISHED, value: timestampDate as Any)
    }

    public var isDeactivated: Bool {
        let v =  self.get(key: DEACTIVATED)
        if case Optional<Any>.none = v {
            return false
        }
        else {
            return v as! Bool
        }
    }

    public func setDeactivated(_ newValue: Bool) {
        put(key: DEACTIVATED, value: newValue as Any)
    }
}
