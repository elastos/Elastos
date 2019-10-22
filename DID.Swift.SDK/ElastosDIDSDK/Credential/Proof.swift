import Foundation

public class Proof {
    var type: String!
    var verificationMethod: DIDURL!
    var signature: String!
    
    init(_ type: String, _ method: DIDURL, _ signature: String) {
        self.type = type
        self.verificationMethod = method
        self.signature = signature
    }
    
    func toJson(_ ref: DID, _ compact: Bool) -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        var value: String
        //type:
        if !compact || !(type == Constants.defaultPublicKeyType) {
            dic[Constants.type] = type
        }
        
        // method:
        if compact && verificationMethod.did.isEqual(ref) {
            value = "#" + verificationMethod.fragment
        }
        else {
            value = verificationMethod.toExternalForm()
        }
        dic[Constants.verificationMethod] = value
        
        // signature:
        dic[Constants.signature] = signature
        return dic
    }
    
    class func fromJson(_ json: OrderedDictionary<String, Any>, _ ref: DID) throws -> Proof {
        let type: String = try JsonHelper.getString(json, Constants.type, true, Constants.defaultPublicKeyType, "crendential proof type")
        let method: DIDURL = try JsonHelper.getDidUrl(json, Constants.verificationMethod, ref, "crendential proof verificationMethod")
        let signature: String = try JsonHelper.getString(json, Constants.signature, false, nil, "crendential proof signature")
        return Proof(type, method, signature)
    }
}
