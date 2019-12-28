
import Foundation

public class CredentialMeta: Metadata {
    private let ALIAS: String = "alias"
    var alias: String = ""
    
    
    
    public class func fromString(_ metadata: String) throws -> CredentialMeta {
        return try fromString(metadata, CredentialMeta.self)
    }
    
    override func fromJson(_ json: Dictionary<String, String>) throws {
    var value = json[ALIAS]
        if value != "" {
            self.alias = value!
        }
    }
    
    override func toJson(_ json: Dictionary<String, Any>) {
        var dic: Dictionary<String, String> = [: ]
        if alias != "" {
            dic[ALIAS] = alias
        }
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
}

