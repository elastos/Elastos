
import Foundation

public class CredentialMeta: Metadata {
    private let ALIAS: String = "alias"
    var alias: String = ""
    
    public class func fromString(_ metadata: String) throws -> CredentialMeta {
        return try fromString(metadata, CredentialMeta.self)
    }
    
    override func fromJson(_ json: OrderedDictionary<String, Any>) throws {
        let value = json[ALIAS] as? String ?? ""
        if value != "" {
            self.alias = value
        }
    }
    
    override func toJson() -> String {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        if alias != "" {
            dic[ALIAS] = alias
        }
        return JsonHelper.creatJsonString(dic: dic)
    }
    
   public override func merge(_ meta: Metadata) throws {
        guard meta is CredentialMeta else {
            throw DIDError.failue("")
        }
        
        let m: CredentialMeta = meta as! CredentialMeta
        if m.alias != "" {
            alias = m.alias
        }
        try super.merge(meta)
    }
    
    public override var description: String {
        return toJson()
    }
}

