import Foundation

public class Proof {
    var type: String!
    var verificationMethod: DIDURL!
    var signature: String!
    var realm: String?
    var nonce: String?
    
    init(_ type: String, _ method: DIDURL, _ signature: String) {
        self.type = type
        self.verificationMethod = method
        self.signature = signature
    }
    
    init(_ type: String, _ method: DIDURL, _ realm: String, _ nonce: String, _ signature: String) {
        self.type = type
        self.verificationMethod = method
        self.realm = realm
        self.nonce = nonce
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
    
    func toJson_vp(_ ref: DID?, _ compact: Bool) -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        var value: String
        //type:
        if !compact || !(type == Constants.defaultPublicKeyType) {
            dic[Constants.type] = type
        }
        
        // method:
        if compact && ref != nil && verificationMethod.did.isEqual(ref!) {
            value = "#" + verificationMethod.fragment
        }
        else {
            value = verificationMethod.toExternalForm()
        }
        dic[Constants.verificationMethod] = value
        
        // realm
        dic[Constants.realm] = realm!
        
        // nonce
        dic[Constants.nonce] = nonce!
        
        // signature:
        dic[Constants.signature] = signature
        return dic
    }
    
    class func fromJson(_ json: OrderedDictionary<String, Any>, _ ref: DID?) throws -> Proof {
        let type: String = try JsonHelper.getString(json, Constants.type, true, Constants.defaultPublicKeyType, "crendential proof type")
        let method: DIDURL = try JsonHelper.getDidUrl(json, Constants.verificationMethod, ref, "crendential proof verificationMethod")
        let signature: String = try JsonHelper.getString(json, Constants.signature, false, nil, "crendential proof signature")
        return Proof(type, method, signature)
    }
    
    class func fromJson_vp(_ json: OrderedDictionary<String, Any>, _ ref: DID?) throws -> Proof {
        let type: String = try JsonHelper.getString(json, Constants.type, true, Constants.defaultPublicKeyType, "crendential proof type")
        let method: DIDURL = try JsonHelper.getDidUrl(json, Constants.verificationMethod, ref, "presentation proof verificationMethod")
         let realm: String = try JsonHelper.getString(json, Constants.realm, false, nil, "presentation proof realm")
         let nonce: String = try JsonHelper.getString(json, Constants.nonce, false, nil, "presentation proof nonce")
        let signature: String = try JsonHelper.getString(json, Constants.signature, false, nil, "presentation proof signature")
        return Proof(type, method, realm, nonce, signature)
    }
}
