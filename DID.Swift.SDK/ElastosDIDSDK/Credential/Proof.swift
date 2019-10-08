import Foundation

public class Proof {
    var type: String!
    var verificationMethod: DIDURL!
    var signature: String!
    
    func toJson(_ ref: DID, _ compact: Bool) -> Dictionary<String, Any> {
        var dic: Dictionary<String, Any> = [: ]
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
            value = verificationMethod.fragment
        }
        dic[Constants.verificationMethod] = value
        
        // signature:
        dic[Constants.signature] = signature
        return dic
    }
    
    
    
}
