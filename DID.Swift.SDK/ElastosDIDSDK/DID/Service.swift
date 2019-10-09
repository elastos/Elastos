import Foundation

public class Service: DIDObject {
    private var endpoint: String!

    init(_ id: DIDURL, _ type: String, _ endpoint: String) {
        super.init(id, type)
        self.endpoint = endpoint
    }

    class func fromJson(_ node: Dictionary<String, Any>, _ ref: DID) throws -> Service {
        let id = try JsonHelper.getDidUrl(node, Constants.id, ref, "service' id")
        let type = try JsonHelper.getString(node,Constants.type, false, nil, "service' type")
        let endpoint = try JsonHelper.getString(node,Constants.serviceEndpoint, false, nil, "service' endpoint")
        return Service(id, type, endpoint)
    }

    public func toJson(_ ref: DID, _ compact: Bool) -> Dictionary<String, Any> {
        var dic: Dictionary<String, Any> = [: ]
        var value: String
        
        // id
        if compact && id.did.isEqual(ref) {
            value = "#" + id.fragment
        }
        else {
            value = id.toExternalForm()
        }
        dic[Constants.id] = value
        
        // type
        dic[Constants.type] = type
        
        // endpoint
        dic[Constants.serviceEndpoint] = endpoint
        return dic
    }
}
