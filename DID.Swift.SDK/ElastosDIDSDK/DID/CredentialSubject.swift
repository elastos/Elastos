import Foundation

public class CredentialSubject {
    
    public var id: DID!
    public var properties: OrderedDictionary<String, String>!
    
    public init(_ id: DID) {
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
    
    func toJson(_ ref: DID?, _ normalized: Bool) -> OrderedDictionary< String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        // id
        if normalized || ref == nil || id != ref {
            dic[Constants.ID] = id.description
        }
        
        // Properties
        properties.forEach { (key, value) in
            dic[key] = value
        }
        return dic
    }

    class func fromJson(_ json: OrderedDictionary<String, Any>, _ ref: DID?) throws -> CredentialSubject {
        // id
        let op: Bool = ref != nil
        let id: DID = try JsonHelper.getDid(json, Constants.ID, op, ref, "crendentialSubject id")!
        let cs: CredentialSubject = CredentialSubject(id)
        json.forEach { key, value in
            if key != Constants.ID {
                if key != "" {
                    cs.addProperty(key, value as! String)
                }
            }
        }
        return cs
    }
}

