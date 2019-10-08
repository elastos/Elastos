import Foundation

public class CredentialSubject {
    
    public var id: DID!
    public var properties: Dictionary<String, String>!
    
    init(_ id: DID) {
        self.id = id
        properties = [: ]
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
    
    public func addProperties(_ dic: Dictionary<String, String> ) {
        properties.merge(dict: dic)
    }
    
    func toJson(_ ref: DID, _ compact: Bool) -> Dictionary< String, Any> {
        var dic: Dictionary<String, Any> = [: ]
        
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
}
