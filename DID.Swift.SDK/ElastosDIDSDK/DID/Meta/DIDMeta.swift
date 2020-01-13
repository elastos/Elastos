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

    override func fromJson(_ json: OrderedDictionary<String, Any>) throws {
        var value = json[DIDMeta.ALIAS] as? String ?? ""
        if value != "" {
            self.alias = value
        }
        
        value = json[DIDMeta.DEACTIVATED] as? String ?? ""
        if value != "" {
            if value != "0" {
                self.deactivated = true
            }
            else {
                self.deactivated = false
            }
        }
        
        value = json[DIDMeta.TXID] as? String ?? ""
        if value != "" {
            self.transactionId = value
        }
        
        value = json[DIDMeta.TIMESTAMP] as? String ?? ""
        if value != "" {
            let date = DateFormater.parseDate(value)
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
}
