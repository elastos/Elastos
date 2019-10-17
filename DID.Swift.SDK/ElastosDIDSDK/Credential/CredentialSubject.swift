import Foundation

public class CredentialSubject {
    
    public var id: DID!
    public var properties: OrderedDictionary<String, String>!
    
    init(_ id: DID) {
        self.id = id
        properties = OrderedDictionary()
    }
    
    public func getPropertyCount() -> Int {
        return properties.count
    }
    
    public func getProperty(_ name: String) -> String {
        return properties[name]!
    }
    
    public func addProperty(_ name: String, _ value: String) {
        properties[name] = value
    }
    
    public func addProperties(_ dic: OrderedDictionary<String, String> ) {
        dic.forEach { (key, value) in
            properties[key] = value
        }
    }
    
    func toJson(_ ref: DID, _ compact: Bool) -> OrderedDictionary< String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        
        // id
        if !compact && !id.isEqual(ref) {
            dic[Constants.id] = id.toExternalForm()
        }
        
        // Properties
        properties.forEach { (key, value) in
            dic[key] = value
        }
        return dic
    }
    
    class func fromJson(_ json: Dictionary<String, Any>, _ ref: DID?) throws -> CredentialSubject {
        // id
        let op: Bool = ref != nil
        let id: DID = try JsonHelper.getDid(json, Constants.id, op, ref, "crendentialSubject id")
        let cs: CredentialSubject = CredentialSubject(id)
        guard json.count > 1 else {
            print("Empty credentialSubject.")
            return cs
        }
        json.forEach { key, value in
            if key == Constants.id {
                cs.addProperty(key, value as! String)
            }
        }
        return cs
    }
}

