import Foundation

public class DIDMeta: Metadata {
    private static let TXID: String = "txid"
    private static let TIMESTAMP: String = "timestamp"
    private static let ALIAS: String = "alias"
    private static let DEACTIVATED: String = "deactivated"

    public var deactivated: Bool = false
    public var updated: Date?
    public var transactionId: String?
    public var alias: String = ""

    public var isDeactivated: Bool {
        return deactivated
    }
    
    public class func fromString(_ metadata: String) throws -> DIDMeta {
        return try fromString(metadata, DIDMeta.self)
    }

    override func fromJson(_ json: Dictionary<String, Any>) throws {
        let _alias = json[DIDMeta.ALIAS] as? String ?? ""
        if !_alias.isEmpty {
            self.alias = _alias
        }

        let _deactived = json[DIDMeta.DEACTIVATED] as? String
        if _deactived != nil && _deactived!.count > 0 {
            self.deactivated = true
        }
        
        let _transactionId = json[DIDMeta.TXID] as? String
        if _transactionId != nil && _transactionId!.count > 0 {
            self.transactionId = _transactionId
        }

        let _timestamp = json[DIDMeta.TIMESTAMP] as? String
        if _timestamp != nil && _timestamp!.count > 0 {
            let date = DateFormater.parseDate(_timestamp!)
            self.updated = date
        }
    }
    
    override func toJson() -> String {
        var dict: OrderedDictionary<String, Any> = OrderedDictionary()

        if !alias.isEmpty {
            dict[DIDMeta.ALIAS] = alias
        }

        if deactivated {
            dict[DIDMeta.DEACTIVATED] = true
        }

        if let _ = transactionId {
            dict[DIDMeta.TXID] = transactionId
        }

        if let _ = updated {
            dict[DIDMeta.TIMESTAMP] = DateFormater.format(updated!)
        }
        
        return JsonHelper.creatJsonString(dic: dict)
    }
    
    override public func merge(_ meta: Metadata) throws {
        guard meta is DIDMeta else {
            throw DIDError.failue("")
        }

        let _meta = meta as! DIDMeta
        if !_meta.alias.isEmpty {
            alias = _meta.alias
        }
        
        if !deactivated {
            deactivated = _meta.deactivated
        }

        if let _ = _meta.transactionId {
            transactionId = _meta.transactionId
        }

        if let _ = _meta.updated {
            updated = _meta.updated
        }

        try super.merge(meta)
    }

    public override var description: String {
        return toJson()
    }
    
    public override func isEmpty() -> Bool {
        if alias != "" || deactivated || transactionId != nil || transactionId != "" || updated != nil {
            return false
        }
        return super.isEmpty()
    }
}
