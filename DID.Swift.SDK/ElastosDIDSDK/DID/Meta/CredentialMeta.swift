import Foundation

public class CredentialMeta: Metadata {
    private static let ALIAS: String = "alias"

    var alias: String = ""
    
    public class func fromString(_ metadata: String) throws -> CredentialMeta {
        return try fromString(metadata, CredentialMeta.self)
    }
    
    override func fromJson(_ json: Dictionary<String, Any>) throws {
        let _alias = json[CredentialMeta.ALIAS] as? String ?? ""
        if !_alias.isEmpty {
            self.alias = _alias;
        }
    }
    
    override func toJson() -> String {
        var dict: OrderedDictionary<String, Any> = OrderedDictionary()
        if !alias.isEmpty {
            dict[CredentialMeta.ALIAS] = alias
        }
        return JsonHelper.creatJsonString(dic: dict)
    }
    
   public override func merge(_ meta: Metadata) throws {
        guard meta is CredentialMeta else {
            throw DIDError.failue("")
        }

        let _meta: CredentialMeta = meta as! CredentialMeta
        if !_meta.alias.isEmpty {
            alias = _meta.alias
        }

        try? super.merge(meta)
    }
    
    public override var description: String {
        return toJson()
    }
    
    public override func isEmpty() -> Bool {
        if alias != "" {
            return false
        }
        return super.isEmpty()
    }
}
