
import UIKit

public class CredentialProof: Proof {
        public var verificationMethod: DIDURL!
    
    init(_ type: String, _ method: DIDURL, _ signature: String) {
        super.init()
        self.type = type
        self.verificationMethod = method
        self.signature = signature
    }
    
    class func fromJson(_ json: OrderedDictionary<String, Any>, _ ref: DID?) throws -> CredentialProof {
        let type: String = try JsonHelper.getString(json, TYPE, true, DEFAULT_PUBLICKEY_TYPE, "crendential proof type")
        
        let method: DIDURL = try JsonHelper.getDidUrl(json, VERIFICATION_METHOD, ref, "crendential proof verificationMethod")!
        
        let signature: String = try JsonHelper.getString(json, SIGNATURE, false, nil, "crendential proof signature")
        
        return CredentialProof(type, method, signature)
    }
    
    func toJson(_ ref: DID, _ normalized: Bool) -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        var value: String
        //type:
        if normalized || type != DEFAULT_PUBLICKEY_TYPE {
            dic[TYPE] = type
        }
        
        // method:
        if normalized || verificationMethod.did != ref {
             value = verificationMethod.toExternalForm()
        }
        else {
            value = "#" + verificationMethod.fragment
        }
        dic[VERIFICATION_METHOD] = value
        
        // signature:
        dic[SIGNATURE] = signature
        return dic
    }
}
