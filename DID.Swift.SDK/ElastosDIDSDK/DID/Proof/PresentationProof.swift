
import UIKit

public class PresentationProof: Proof {
        public var verificationMethod: DIDURL!
        public var realm: String?
        public var nonce: String?
    
    init(_ type: String, _ method: DIDURL, _ realm: String, _ nonce: String, _ signature: String) {
        super.init()
        self.type = type
        self.verificationMethod = method
        self.realm = realm
        self.nonce = nonce
        self.signature = signature
    }
    
    init(_ method: DIDURL, _ realm: String, _ nonce: String, _ signature: String) {
        super.init()
        self.type = DEFAULT_PUBLICKEY_TYPE
        self.verificationMethod = method
        self.realm = realm
        self.nonce = nonce
        self.signature = signature
    }
    
    class func fromJson(_ json: Dictionary<String, Any>, _ ref: DID?) throws -> PresentationProof {
        let type: String = try JsonHelper.getString(json, TYPE, true, ref: DEFAULT_PUBLICKEY_TYPE, "crendential proof type")
        
        let method: DIDURL = try JsonHelper.getDidUrl(json, VERIFICATION_METHOD, ref: ref, "presentation proof verificationMethod")!
        
         let realm: String = try JsonHelper.getString(json, REALM, false, "presentation proof realm")
        
         let nonce: String = try JsonHelper.getString(json, NONCE, false, "presentation proof nonce")
        
        let signature: String = try JsonHelper.getString(json, SIGNATURE, false, "presentation proof signature")
        
        return PresentationProof(type, method, realm, nonce, signature)
    }
    
    func toJson() -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        var value: String
        //type:
        dic[TYPE] = type
        
        // method:
        value = verificationMethod.toExternalForm()
        dic[VERIFICATION_METHOD] = value
        
        // realm
        dic[REALM] = realm!
        
        // nonce
        dic[NONCE] = nonce!
        
        // signature:
        dic[SIGNATURE] = signature

        return dic
    }
}
