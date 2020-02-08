import Foundation

public class CredentialSubject {
    
    public var id: DID
    public var properties: Dictionary<String, Any> = [: ]
    
    public init(_ id: DID) {
        self.id = id
    }
    
    public func getPropertyCount() -> Int {
        return properties.count
    }
    
    public func getProperty(_ name: String) -> Any? {
        return properties[name]
    }
    
    public func addProperty(_ name: String, _ value: Any) {
        properties[name] = value
    }

    public func addProperties(_ dic: Dictionary<String, Any> ) {
        dic.forEach { (key, value) in
            properties[key] = value
        }
    }
    
    func toJson(_ ref: DID?, _ normalized: Bool) -> OrderedDictionary< String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        // id
        if normalized || ref == nil || id != ref {
            dic[ID] = id.description
        }
        
        // Properties
        let d = DIDURLComparator.DIDOrderedDictionaryComparatorByKey(source: properties)
        d.forEach { (key, value) in
            dic[key] = value
        }
        return dic
    }

    class func fromJson(_ json: Dictionary<String, Any>, _ ref: DID?) throws -> CredentialSubject {
        // id
        let op: Bool = ref != nil
        let id: DID = try JsonHelper.getDid(json, ID, op, ref: ref, "crendentialSubject id")!
        let cs: CredentialSubject = CredentialSubject(id)
        json.forEach { key, value in
            if key != ID {
                if key != "" {
                    cs.addProperty(key, value)
                }
            }
        }
        return cs
    }
}

