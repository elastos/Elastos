import Foundation

public class Service: DIDObject {
    public var endpoint: String!
    
    init(_ id: DIDURL, _ type: String, _ endpoint: String) {
        super.init(id, type)
        self.endpoint = endpoint
    }
    
    class func fromJson(_ json: OrderedDictionary<String, Any>, _ ref: DID) throws -> Service {
        let id = try JsonHelper.getDidUrl(json, Constants.ID, ref, "service' id")
        let type = try JsonHelper.getString(json,Constants.TYPE, false, nil, "service' type")
        let endpoint = try JsonHelper.getString(json,Constants.SERVICE_ENDPOINT, false, nil, "service' endpoint")
        return Service(id!, type, endpoint)
    }
    
    public func toJson(_ ref: DID, _ normalized: Bool) -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        var value: String
        
        // id
        if normalized || id.did != ref {
            value = id.toExternalForm()
        }
        else {
            value = "#" + id.fragment!
        }
        dic[Constants.ID] = value
        
        // type
        dic[Constants.TYPE] = type
        
        // endpoint
        dic[Constants.SERVICE_ENDPOINT] = endpoint
        
        return dic
    }
}
