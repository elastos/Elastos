import Foundation

public class VerifiableCredential: DIDObject {
    public var types: Array<String>!
    public var issuer: DID!
    public var issuanceDate: Date!
    public var expiationDate: Date!
    public var subject: CredentialSubject!
    public var proof: Proof!
    
    override init() {
        super.init()
    }
    
    init(_ vc: VerifiableCredential) {
        super.init(vc.id, vc.type)
        self.id = vc.id
        self.types = vc.types
        self.issuer = vc.issuer
        self.issuanceDate = vc.issuanceDate
        self.expiationDate = vc.expiationDate
        self.subject = vc.subject
        self.proof = vc.proof
    }
    
    public func toJson(_ ref: DID, _ compact: Bool, _ forSign: Bool) -> Dictionary<String, Any> {
        var dic: Dictionary<String, Any> = [: ]
        var value: String
        
        // id
        if compact && id.isEqual(ref) {
            value = "#" + id.fragment
        }
        else {
            value = id.toExternalForm()
        }
        dic[Constants.id] = value
        
        // type
        var strs: Array<String> = []
        types.forEach{ str in
            strs.append(str)
        }
        dic[Constants.type] = strs
        
        // issuer
        if !compact && !(issuer.isEqual(subject.id)) {
            dic[Constants.issuer] = issuer.toExternalForm()
        }
        
        // issuanceDate
        if (expiationDate != nil) {
            dic[Constants.expirationDate] = "TODO: change to time string"
        }
        
        // credentialSubject
        dic[Constants.credentialSubject] = subject.toJson(ref, compact)
        
        // proof
        if !forSign {
            dic[Constants.proof] = proof.toJson(ref, compact)
        }
        return dic
    }
    
//    class func fromJson(_ json: String) -> VerifiableCredential {
//        let vc: VerifiableCredential = VerifiableCredential()
//    }
//
//    class func fromJson(_ md: Any, _ ref: DID) -> VerifiableCredential {
//
//    }
//
//    class func fromJson(_ md: Any) -> VerifiableCredential {
//
//    }
    
}
