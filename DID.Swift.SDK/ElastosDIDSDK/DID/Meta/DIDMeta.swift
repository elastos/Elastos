

import Foundation

public class DIDMeta: Metadata {
    
    private let TXID: String = "txid"
    private let TIMESTAMP: String = "timestamp"
    private let ALIAS: String = "alias"
    private let DEACTIVATED: String = "deactivated"

    public var deactivated: Bool = false
    public var updated: Date?
    public var transactionId: String = ""
    public var alias: String = ""

    public func isDeactivated() -> Bool {
        return deactivated
    }
    
    public class func fromString(_ metadata: String) throws -> DIDMeta {
        return try fromString(metadata, DIDMeta.self)
    }

    override func fromJson(_ json: Dictionary<String, String>) throws {
        var value = json[ALIAS]
        if value != "" {
            self.alias = value!
        }
        
        value = json[DEACTIVATED]
        if value != "" {
            if value != "0" {
                self.deactivated = true
            }
            else {
                self.deactivated = false
            }
        }
        
        value = json[TXID]
        if value != "" {
            self.transactionId = value!
        }
        
        value = json[TIMESTAMP]
        if value != "" {
            let date = DateFormater.parseDate(value!)
            self.updated = date
        }
    }
    
    override func toJson(_ json: Dictionary<String, Any>) {
        var dic: Dictionary<String, Any> = [: ]
        if alias != "" {
            dic[ALIAS] = alias
        }
        
        if deactivated {
            dic[DEACTIVATED] = true
        }
        
        if transactionId != "" {
            dic[TXID] = transactionId
        }
        
        if updated != nil {
            dic[TIMESTAMP] = DateFormater.format(updated!)
        }
    }
    
    override public func merge(_ meta: Metadata) throws {
        guard meta is DIDMeta else {
            throw DIDError.failue("")
        }
        
        let m: DIDMeta = meta as! DIDMeta
        if m.alias != "" {
            alias = m.alias
        }
        
        deactivated = m.deactivated
        if m.transactionId != "" {
            transactionId = m.transactionId
        }
        
        if (m.updated != nil){
            updated = m.updated
        }
        try super.merge(meta)
    }
}
